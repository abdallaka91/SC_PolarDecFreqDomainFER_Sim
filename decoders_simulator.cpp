// FPGA-UART decoded-readback FER version.
// The host generates random u, polar-encodes it, sends N*GF probabilities to the FPGA,
// reads back the decoded u word, and counts FER/SER in software.
// IMPORTANT: this file assumes the FPGA bitstream uses uart_batch_decoder_top at 3 Mbaud
// with CMD_BULK, CMD_START, CMD_STATUS, and CMD_DECODED enabled.

// #define find_llr_rang // disable it

#include "init.h"
#include "struct.h"
#include "tools.h"
#include <algorithm>
#include <atomic>
#include <cerrno>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fcntl.h>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <memory>
#include <omp.h>
#include <random>
#include <signal.h>
#include <sstream>
#include <stdexcept>
#include <string>
#include <sys/select.h>
#include <termios.h>
#include <unistd.h>
#include <vector>
#include <inttypes.h>

#include "./ccsk_simulator/ccsk_simulator.hpp"
#include "./ccsk_simulator/simul_parameters.hpp"

#include "./include/custom_types.hpp"
#include "./include/encoder_1.hpp"

using namespace PoAwN::structures;
using namespace PoAwN::tools;
using namespace PoAwN::init;
using std::array;
using std::cout;
using std::endl;
using std::stod;
using std::stoi;
using std::string;
using std::vector;

namespace fs = std::filesystem;

// ============================================================================
// UART helpers for FPGA decoded-word readback
// ============================================================================

#ifndef B3000000
#error "B3000000 is not defined by your system headers. Use B2000000/B1000000 and change FPGA UART_BAUD too."
#endif

static constexpr int FPGA_UART_BAUD_PRINT = 3000000;
static constexpr speed_t FPGA_UART_BAUD_CONST = B3000000;

static int uart_open_3mbaud(const std::string &dev)
{
    int fd = open(dev.c_str(), O_RDWR | O_NOCTTY | O_SYNC);
    if (fd < 0)
    {
        throw std::runtime_error("open(" + dev + ") failed: " + std::strerror(errno));
    }

    termios tty{};
    if (tcgetattr(fd, &tty) != 0)
    {
        close(fd);
        throw std::runtime_error("tcgetattr failed: " + std::string(std::strerror(errno)));
    }

    cfmakeraw(&tty);
    cfsetospeed(&tty, FPGA_UART_BAUD_CONST);
    cfsetispeed(&tty, FPGA_UART_BAUD_CONST);

    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;
    tty.c_cflag |= (CLOCAL | CREAD);
    tty.c_cflag &= ~(PARENB | PARODD);
    tty.c_cflag &= ~CSTOPB;
#ifdef CRTSCTS
    tty.c_cflag &= ~CRTSCTS;
#endif

    tty.c_iflag = 0;
    tty.c_oflag = 0;
    tty.c_lflag = 0;
    tty.c_cc[VMIN]  = 0;
    tty.c_cc[VTIME] = 1;

    if (tcsetattr(fd, TCSANOW, &tty) != 0)
    {
        close(fd);
        throw std::runtime_error("tcsetattr failed: " + std::string(std::strerror(errno)));
    }

    tcflush(fd, TCIOFLUSH);
    return fd;
}

static void uart_write_all(int fd, const uint8_t *buf, size_t len)
{
    size_t off = 0;
    while (off < len)
    {
        ssize_t n = write(fd, buf + off, len - off);
        if (n < 0)
        {
            if (errno == EINTR) continue;
            throw std::runtime_error("UART write failed: " + std::string(std::strerror(errno)));
        }
        if (n == 0)
        {
            throw std::runtime_error("UART write returned 0");
        }
        off += static_cast<size_t>(n);
    }
}

static bool uart_read_exact(int fd, uint8_t *buf, size_t len, int timeout_ms)
{
    size_t off = 0;
    auto t0 = std::chrono::steady_clock::now();

    while (off < len)
    {
        auto now = std::chrono::steady_clock::now();
        int elapsed_ms = (int)std::chrono::duration_cast<std::chrono::milliseconds>(now - t0).count();
        int remaining_ms = timeout_ms - elapsed_ms;
        if (remaining_ms <= 0)
            return false;

        fd_set rfds;
        FD_ZERO(&rfds);
        FD_SET(fd, &rfds);

        timeval tv{};
        tv.tv_sec  = remaining_ms / 1000;
        tv.tv_usec = (remaining_ms % 1000) * 1000;

        int r = select(fd + 1, &rfds, nullptr, nullptr, &tv);
        if (r < 0)
        {
            if (errno == EINTR) continue;
            throw std::runtime_error("UART select failed: " + std::string(std::strerror(errno)));
        }
        if (r == 0)
            return false;

        ssize_t n = read(fd, buf + off, len - off);
        if (n < 0)
        {
            if (errno == EINTR) continue;
            throw std::runtime_error("UART read failed: " + std::string(std::strerror(errno)));
        }
        if (n > 0)
            off += static_cast<size_t>(n);
    }

    return true;
}

static void fpga_clear_counters(int fd)
{
    uint8_t cmd = 0x72;
    uint8_t ack = 0;
    uart_write_all(fd, &cmd, 1);
    if (!uart_read_exact(fd, &ack, 1, 2000) || ack != 0x55)
    {
        throw std::runtime_error("FPGA clear-counters command failed");
    }
}

static void fpga_read_counters(int fd, uint32_t &total, uint32_t &errors)
{
    uint8_t cmd = 0x70;
    uint8_t b[8]{};
    uart_write_all(fd, &cmd, 1);
    if (!uart_read_exact(fd, b, 8, 2000))
    {
        throw std::runtime_error("Timeout while reading FPGA counters");
    }

    total  = (uint32_t)b[0] | ((uint32_t)b[1] << 8) | ((uint32_t)b[2] << 16) | ((uint32_t)b[3] << 24);
    errors = (uint32_t)b[4] | ((uint32_t)b[5] << 8) | ((uint32_t)b[6] << 16) | ((uint32_t)b[7] << 24);
}

// Send one N x q probability frame to FPGA memory without starting the decoder.
static void fpga_send_bulk_frame(int fd, const uint16_t *prob_intgr, int N, int q)
{
    std::vector<uint8_t> packet;
    packet.reserve(1 + (size_t)N * (size_t)q * 2);
    packet.push_back(0x60);

    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j < q; j++)
        {
            // FPGA data_t is signed W=10. For probabilities use 0..511.
            uint16_t v = prob_intgr[i * q + j] & 0x03FFu;
            packet.push_back((uint8_t)(v & 0xFFu));
            packet.push_back((uint8_t)((v >> 8) & 0x03u));
        }
    }

    uart_write_all(fd, packet.data(), packet.size());

    uint8_t resp = 0;
    if (!uart_read_exact(fd, &resp, 1, 10000) || resp != 0x55)
    {
        char msg[128];
        std::snprintf(msg, sizeof(msg), "FPGA bulk-load failed, response=0x%02X", resp);
        throw std::runtime_error(msg);
    }
}

// Start one decode after a bulk frame has been loaded.
static void fpga_start_decode(int fd)
{
    uint8_t cmd = 0x30;
    uint8_t ack = 0;
    uart_write_all(fd, &cmd, 1);
    if (!uart_read_exact(fd, &ack, 1, 2000) || ack != 0x55)
    {
        throw std::runtime_error("FPGA start-decode command failed");
    }
}

// Read wrapper status: bit0 busy, bit1 valid_seen, bit2 done_seen, bit3 load_ready.
static uint8_t fpga_read_status(int fd)
{
    uint8_t cmd = 0x50;
    uint8_t st = 0;
    uart_write_all(fd, &cmd, 1);
    if (!uart_read_exact(fd, &st, 1, 2000))
    {
        throw std::runtime_error("Timeout while reading FPGA status");
    }
    return st;
}

// Wait until the decoder has latched valid/done for the current frame.
static void fpga_wait_decode_done(int fd)
{
    auto t0 = std::chrono::steady_clock::now();
    while (true)
    {
        uint8_t st = fpga_read_status(fd);
        if ((st & 0x06u) != 0)
            return;

        auto now = std::chrono::steady_clock::now();
        int elapsed_ms = (int)std::chrono::duration_cast<std::chrono::milliseconds>(now - t0).count();
        if (elapsed_ms > 10000)
            throw std::runtime_error("Timeout waiting for FPGA decode done");

        usleep(1000);
    }
}

// Read the decoded u word returned by fixed_decoder over CMD_DECODED.
static std::vector<uint16_t> fpga_read_decoded_word(int fd, int N, int q)
{
    uint8_t cmd = 0x71;
    std::vector<uint8_t> bytes(N);
    uart_write_all(fd, &cmd, 1);
    if (!uart_read_exact(fd, bytes.data(), bytes.size(), 5000))
    {
        throw std::runtime_error("Timeout while reading FPGA decoded word");
    }

    std::vector<uint16_t> decoded(N);
    uint16_t mask = (q >= 256) ? 0x00FFu : (uint16_t)(q - 1u);
    for (int i = 0; i < N; i++)
        decoded[i] = (uint16_t)bytes[i] & mask;
    return decoded;
}

static void lzc_normalize_runtime(std::vector<int32_t> &s, int nbits)
{
    const int target_bit = nbits - 2;

    auto abs_val = [](int32_t x) -> uint32_t
    {
        if (x == INT32_MIN)
            return (uint32_t)INT32_MAX + 1u;
        return (x < 0) ? (uint32_t)(-x) : (uint32_t)x;
    };

    uint32_t or_all = 0;
    for (int32_t v : s)
        or_all |= abs_val(v);

    if (or_all == 0)
        return;

    int msb_pos = 31 - __builtin_clz(or_all);
    int shift = msb_pos - target_bit;

    for (int32_t &v : s)
    {
        if (shift > 0)
            v >>= shift;
        else if (shift < 0)
            v <<= -shift;
    }
}

static void quantize_like_fixed_decoder(const float *prob, uint16_t *dst, int q, int nbits)
{
    float max_p = *std::max_element(prob, prob + q);
    if (max_p <= 0.0f)
        max_p = 1.0f;

    std::vector<int32_t> tmp(q);
    for (int j = 0; j < q; j++)
    {
        double normalized = static_cast<double>(prob[j]) / static_cast<double>(max_p);
        double scaled = normalized * (0.999 * static_cast<double>(1u << 31));
        tmp[j] = static_cast<int32_t>(std::floor(scaled));
    }

    lzc_normalize_runtime(tmp, nbits);

    const int32_t max_val = (1 << (nbits - 1)) - 1;
    const int32_t min_val = -(1 << (nbits - 1));
    for (int j = 0; j < q; j++)
    {
        int32_t v = std::clamp(tmp[j], min_val, max_val);
        dst[j] = static_cast<uint16_t>(v) & ((1u << nbits) - 1u);
    }
}

// ============================================================================
// Original result helpers
// ============================================================================

std::string format_FER(double FER_value, int width = 10)
{
    std::ostringstream oss;
    if (FER_value < 0.0001)
        oss << std::scientific << std::setprecision(3) << FER_value;
    else
        oss << std::fixed << std::setprecision(10) << FER_value;
    std::string s = oss.str();
    if ((int)s.length() < width)
        s = std::string(width - s.length(), ' ') + s;

    if ((int)s.length() > width)
        s = s.substr(0, width);

    return s;
}

void append_results_to_file1(const std::string &dec, int GFx, int Nx, int Kx, double SNR, unsigned long nb_err, uint64_t nb_gen_frame, float llr_sigma, int seconds, int nbits = -1)
{
    fs::path dir = "results";

    std::error_code ec;
    if (!fs::exists(dir))
    {
        if (!fs::create_directories(dir, ec))
        {
            std::cerr << "Error creating directory " << dir << ": " << ec.message() << "\n";
            return;
        }
    }

    fs::path filename = dir / ("GF" + std::to_string(GFx) + "_N" + std::to_string(Nx) + "_K" + std::to_string(Kx) + "_" + dec.c_str() + ".txt");

    FILE *fou = fopen(filename.c_str(), "a");

    if (fou == nullptr)
    {
        std::cerr << "Error opening file " << filename << " for appending.\n";
        return;
    }

    double FER_value = (nb_gen_frame == 0) ? 0.0 : static_cast<double>(nb_err) / nb_gen_frame;
    std::string FER_str = format_FER(FER_value, 11);

    fprintf(fou, "%+7.3f   %s   %6lu   %12" PRIu64, SNR, FER_str.c_str(), nb_err, nb_gen_frame);
    fprintf(fou, " %6d ", seconds);
    if ((nbits > 0))
    {
        fprintf(fou, "    %5d", nbits);
        fprintf(fou, "    %5.3f", llr_sigma);
    }

    if (dec.rfind("dec4", 0) == 0)
    {
#ifdef REP_DISABLED
        fprintf(fou, "   REP_DISABLED");
#endif
#ifdef SPC_DISABLED
        fprintf(fou, "   SPC_DISABLED");
#endif
    }

    fprintf(fou, "\n");
    fclose(fou);
}

#define STR(S) #S
#define EVAL(x) STR(x)

static bool force_quit = false;
void intHandler(int)
{
    printf("\n#(DD) CTRL+C was detected\n");
    force_quit = true;
}

int main(int argc, char *argv[])
{
#ifdef __AVX512BW__
    printf("#(II) Non-binary FFT Successive Cancellation decoder evaluation program (AVX512 version)\n");
#elif __AVX2__
    printf("#(II) Non-binary FFT Successive Cancellation decoder evaluation program (AVX2 version)\n");
#else
    printf("#(II) Non-binary FFT Successive Cancellation decoder evaluation program (ARM NEON/generic version)\n");
#endif
    printf("#(II) + FPGA-UART decoded-readback FER mode, random codewords\n");
    printf("#(II) Binary generated : %s - %s\n", __DATE__, __TIME__);

    signal(SIGINT, intHandler);

    // Not used for decoding in FPGA mode, but kept if your program expects the shared library to be present.
    int num_threads = 1; // one UART link => keep simulation/FPGA access single-threaded

    std::string dec_type = "fpga_uart_readback";
    std::string uart_dev = "/dev/ttyUSB1";
    uint64_t NbMonteCarlo = 10000000000ULL;
    float EbN0 = -1000.f;
    uint16_t q = 0;
    uint16_t p = 0;
    uint16_t N = 0;
    uint16_t n = 0;
    uint16_t K = 0;
    int FER_STOP = 100;
    uint16_t frozen_val = 0;
    int nbits = 10;
    float llr_sigma = -1.f;
    uint32_t rng_seed = 1;

    for (int i = 1; i < argc; i++)
    {
        const std::string arg = argv[i];
        if (arg == "-snr")
        {
            EbN0 = stod(std::string(argv[++i]));
        }
        else if (arg == "-q")
        {
            q = stoi(std::string(argv[++i]));
            p = log2(q);
        }
        else if (arg == "-N")
        {
            N = stoi(std::string(argv[++i]));
            n = log2(N);
        }
        else if (arg == "-K")
        {
            K = stoi(std::string(argv[++i]));
        }
        else if (arg == "-cw")
        {
            NbMonteCarlo = std::stoull(argv[++i]);
        }
        else if (arg == "-errors")
        {
            FER_STOP = std::atoi(argv[++i]);
        }
        else if (arg == "-nbits")
        {
            nbits = std::atoi(argv[++i]);
        }
        else if (arg == "-llr_sigma")
        {
            llr_sigma = std::stof(argv[++i]);
        }
        else if (arg == "-dec")
        {
            dec_type = argv[++i];
        }
        else if (arg == "-uart")
        {
            uart_dev = argv[++i];
        }
        else if (arg == "-seed")
        {
            rng_seed = static_cast<uint32_t>(std::stoul(argv[++i]));
        }
        else
        {
            printf("(EE) Error during CLI parsing\n");
            printf("(EE) argument = [%s]\n", argv[i]);
            exit(EXIT_FAILURE);
        }
    }

    if (EbN0 == -1000.f)
    {
        printf("(EE) missing [-snr] option\n");
        exit(EXIT_FAILURE);
    }
    if (q == 0)
    {
        printf("(EE) missing [-q] option\n");
        exit(EXIT_FAILURE);
    }
    if (N == 0)
    {
        printf("(EE) missing [-N] option\n");
        exit(EXIT_FAILURE);
    }
    if (K == 0)
    {
        printf("(EE) missing [-K] option\n");
        exit(EXIT_FAILURE);
    }
    if (nbits <= 0 || nbits > 10)
    {
        printf("(EE) This FPGA UART wrapper expects 10-bit signed probabilities. Use -nbits 10.\n");
        exit(EXIT_FAILURE);
    }



    const float MAX_2pn1 = (float)((1u << (nbits - 1)) - 1u); // for nbits=10 => 511

    std::cout << "(DD) NbMonteCarlo : " << NbMonteCarlo << std::endl;
    std::cout << "(DD) EbN0         : " << EbN0 << std::endl;
    std::cout << "(DD) q            : " << q << std::endl;
    std::cout << "(DD) p            : " << p << std::endl;
    std::cout << "(DD) N            : " << N << std::endl;
    std::cout << "(DD) K            : " << K << std::endl;
    std::cout << "(DD) dec_type     : " << dec_type << std::endl;
    std::cout << "(DD) FER_STOP     : " << FER_STOP << std::endl;
    std::cout << "(DD) UART         : " << uart_dev << " @ " << FPGA_UART_BAUD_PRINT << " baud" << std::endl;
    std::cout << "(DD) rng_seed     : " << rng_seed << std::endl;

    base_code_t code_param(N, K, n, q, p, frozen_val);
    code_param.sig_mod = "CCSK_BIN";

    table_GF table;

    cout << "(II) Loading code_param [START]" << endl;
    LoadCode(code_param, EbN0, "../matrices/");
    cout << "(II) Loading code_param [END OK]" << endl;

    cout << EVAL(FWHT) " and " EVAL(FWHT_NORM) " are used for FWHT operations." << endl;


    float noise_sigma = sqrt(1.0 / (pow(10, EbN0 / 10.0)));
    if (llr_sigma < 0.f)
        llr_sigma = noise_sigma;

    CCSK_Simulator<_GF_, _N_> simulator(noise_sigma, llr_sigma, num_threads);

    int uart_fd = -1;
    try
    {
        uart_fd = uart_open_3mbaud(uart_dev);
        printf("#(II) Opened %s at 3 Mbaud\n", uart_dev.c_str());
        fpga_clear_counters(uart_fd);
    }
    catch (const std::exception &e)
    {
        fprintf(stderr, "(EE) UART init failed: %s\n", e.what());
        return EXIT_FAILURE;
    }

    uint64_t FER_out = 0, gen_frames_out = 0;
    std::atomic<uint64_t> global_counter(0);
    std::atomic<uint64_t> FER(0);
    std::atomic<uint64_t> SER(0);
    std::atomic<bool> stop(false);
    std::atomic<double> global_llr_min{std::numeric_limits<double>::max()};
    std::atomic<double> global_llr_max{std::numeric_limits<double>::lowest()};

    std::mt19937 rng(rng_seed);
    std::uniform_int_distribution<int> sym_dist(0, q - 1);

    auto start = std::chrono::high_resolution_clock::now();
    auto watchdod = std::chrono::high_resolution_clock::now();

    cout << "Simulation starts with random codewords and FPGA decoded-word readback..." << endl;

    // Single-thread loop: one UART link cannot be shared by OpenMP workers.
    {
        int thread_id = 0;
        uint16_t u_symb[N];
        std::vector<float> llrs_n(N * q);
        std::vector<uint16_t> prob_intgr(N * q);

        while (true)
        {
            bool succ_dec = true;

            // Generate a random source word on information positions and keep frozen positions at zero.
            for (int u = 0; u < N; u++)
                u_symb[u] = 0;
            for (int k = 0; k < K; k++)
                u_symb[code_param.reliab_sequence[k]] = static_cast<uint16_t>(sym_dist(rng));

            uint16_t x_symb[N];
            for (int u = 0; u < N; u++)
                x_symb[u] = u_symb[u];
            polar_encode<_N_>(x_symb);

            // Simulate CCSK transmission of the encoded codeword and produce LLR/probabilities.
            double *llr_values = simulator.simulate_frame(x_symb, thread_id);

            if constexpr (SimulationParams::method == SimulationParams::LLRMethod::EXP)
            {
                simulator.llr_to_probability<_GF_>(llr_values, N);
                for (int i = 0; i < N; i++)
                {
                    for (int j = 0; j < q; j++)
                        llrs_n[i * q + j] = static_cast<float>(llr_values[i * q + j]);

                    quantize_like_fixed_decoder(&llrs_n[i * q],
                                                &prob_intgr[i * q],
                                                q,
                                                nbits);
                }
            }
            else if constexpr (SimulationParams::method == SimulationParams::LLRMethod::FAST_LUT)
            {
                float *probabilities = simulator.llr_to_probability_fast<_GF_>(llr_values, N, thread_id);
                for (int i = 0; i < N; i++)
                {
                    for (int j = 0; j < q; j++)
                        llrs_n[i * q + j] = probabilities[i * q + j];

                    quantize_like_fixed_decoder(&llrs_n[i * q],
                                                &prob_intgr[i * q],
                                                q,
                                                nbits);
                }
            }

            // Send probabilities to hardware, read decoded u, and compare in software.
            try
            {
                fpga_send_bulk_frame(uart_fd, prob_intgr.data(), N, q);
                fpga_start_decode(uart_fd);
                fpga_wait_decode_done(uart_fd);
                std::vector<uint16_t> decoded_u = fpga_read_decoded_word(uart_fd, N, q);

                uint64_t symbol_errors = 0;
                for (int u = 0; u < N; u++)
                {
                    if (decoded_u[u] != u_symb[u])
                        symbol_errors++;
                }

                if (symbol_errors != 0)
                {
                    succ_dec = false;
                    SER.fetch_add(symbol_errors);
                }
            }
            catch (const std::exception &e)
            {
                fprintf(stderr, "\n(EE) FPGA UART error after %" PRIu64 " frames: %s\n", global_counter.load(), e.what());
                force_quit = true;
                succ_dec = false;
            }

            global_counter.fetch_add(1);
            if (!succ_dec)
                FER.fetch_add(1);

            auto curr = std::chrono::high_resolution_clock::now();
            double sec_since_print = std::chrono::duration<double>(curr - watchdod).count();
            if (sec_since_print >= 1.0)
            {
                if ((global_counter.load() >= NbMonteCarlo) || (FER.load() >= (uint64_t)FER_STOP))
                    stop.store(true);
                FER_out = FER.load();
                gen_frames_out = global_counter.load();

                double FER_ratio = (gen_frames_out == 0) ? 0.0 : (double)FER_out / gen_frames_out;
                std::ostringstream oss;
                oss << std::scientific << std::setprecision(3) << std::setw(10) << FER_ratio;
                std::string FER_str = oss.str();

                auto now2 = std::chrono::high_resolution_clock::now();
                double sec = std::chrono::duration<double>(now2 - start).count();
                double fps = (sec > 0.0) ? (double)gen_frames_out / sec : 0.0;

                std::cout << "\r" << std::fixed << std::setprecision(1) << EbN0
                          << " dB, FER = " << std::setw(8) << FER_out
                          << "/" << std::setw(8) << gen_frames_out
                          << " = " << FER_str
                          << " | " << std::setw(6) << (int)sec << " sec. | "
                          << std::fixed << std::setprecision(1) << fps << " fps | ";

                if (FER_out == 0)
                    printf("------");
                else
                {
                    double tps_p_err = sec / (double)FER_out;
                    double restant = (FER_STOP - (int)FER_out) >= 0 ? (FER_STOP - FER_out) : 0;
                    double tps_rest = restant * tps_p_err;
                    printf("%6d sec. | ", (int)tps_rest);
                }
                std::cout << std::flush << "\r";
                watchdod = std::chrono::high_resolution_clock::now();
            }

            if (stop.load() || force_quit)
                break;
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    double sec = std::chrono::duration<double>(end - start).count();

    FER_out = FER.load();
    gen_frames_out = global_counter.load();

    // Optional consistency check with FPGA-side counters.
    try
    {
        uint32_t fpga_total = 0, fpga_errors = 0;
        fpga_read_counters(uart_fd, fpga_total, fpga_errors);
        std::cout << "\nFPGA counters: total=" << fpga_total << " errors=" << fpga_errors << std::endl;
    }
    catch (const std::exception &e)
    {
        std::cerr << "\n(WW) Could not read FPGA counters: " << e.what() << std::endl;
    }

    if (uart_fd >= 0)
        close(uart_fd);

    double FER_ratio = (gen_frames_out == 0) ? 0.0 : (double)FER_out / gen_frames_out;
    std::ostringstream oss;
    oss << std::scientific << std::setprecision(3) << std::setw(10) << FER_ratio;
    std::string FER_str = oss.str();

    std::cout << "\rSNR: " << std::fixed << std::setprecision(1) << EbN0
              << " dB, FER = " << std::setw(8) << FER_out
              << "/" << std::setw(8) << gen_frames_out
              << " = " << FER_str
              << std::flush;

    append_results_to_file1(dec_type, q, N, K, EbN0, FER_out, gen_frames_out, llr_sigma, (int)sec, nbits);

    std::cout << "\nPolar Code: N=" << N << ", K=" << K << ", GF=" << q << std::endl;
    std::cout << "Decoder: " << dec_type << std::endl;
    std::cout << "Eb/N0: " << EbN0 << " dB, noise_sigma: " << noise_sigma << std::endl;
    std::cout << "Actual frames: " << gen_frames_out << std::endl;
    std::cout << "Symbol errors: " << SER.load() << std::endl;
    std::cout << "Time: " << sec << " seconds" << std::endl;
    std::cout << "Throughput: " << ((sec > 0.0) ? gen_frames_out / sec : 0.0) << " fps" << std::endl;
    std::cout << "Throughput info: " << ((sec > 0.0) ? (gen_frames_out * K * p) / sec / 1e6 : 0.0) << " Mbps" << std::endl;
#ifdef find_llr_rang
    std::cout << "global_llr_max: " << global_llr_max << std::endl;
    std::cout << "global_llr_min: " << global_llr_min << std::endl;
#endif

    return 0;
}
