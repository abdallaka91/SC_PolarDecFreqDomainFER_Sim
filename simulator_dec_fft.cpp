#include "Decoder_functions.h"
#include "GF_tools.h"
#include "HelperFunc.h"
#include "channel.h"
#include "init.h"
#include "struct.h"
#include "tools.h"
#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <omp.h>
#include <random>
#include <string>
#include <vector>
// #include "decoders/specialized_pruning/decoder_specialized_pruning.hpp"
// #include "decoders/naive_cfloat/decoder_naive_cfloat.hpp"
// #include "decoders/naive/decoder_naive.hpp"
#include "decoders/naive_fixed1/decoder_naive_fixed.hpp"
// #include "definitions/code.hpp"
#include "decoders/naive_fixed1/f_type.hpp"

// #include <omp.h>

using namespace PoAwN::structures;
using namespace PoAwN::tools;
using namespace PoAwN::init;
using namespace PoAwN::decoding;
using namespace PoAwN::channel;
using std::array;
using std::cout;
using std::endl;
using std::stod;
using std::stoi;
using std::string;
using std::vector;

#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>

namespace fs = std::filesystem;

void append_results_to_file(
    const std::string &modulation,
    int                GFx,
    int                Nx,
    int                Kx,
    double             SNR,
    unsigned long      nb_err,
    unsigned long      nb_gen_frame,
    float              debit,
    int                tSimuSec)
{
    // Directory path
    fs::path dir = "results";

    // Create directory if not exists
    std::error_code ec;
    if (!fs::exists(dir))
    {
        if (!fs::create_directories(dir, ec))
        {
            std::cerr << "Error creating directory " << dir << ": " << ec.message() << "\n";
            return;
        }
    }

    // Compose filename
    fs::path filename = dir / (modulation + "_GF" + std::to_string(GFx) +
                               "_N" + std::to_string(Nx) +
                               "_K" + std::to_string(Kx) + ".txt");

    // Open file in append mode
    FILE *fou = fopen(filename.c_str(), "a");

    if (fou == nullptr)
    {
        std::cerr << "Error opening file " << filename << " for appending.\n";
        return;
    }

    double FER_value = (nb_gen_frame == 0) ? 0.0 : static_cast<double>(nb_err) / nb_gen_frame;

    fprintf(fou, "%+6.2f ", SNR);
    fprintf(fou, "%1.16f ", FER_value);
    fprintf(fou, "%1.2e ", FER_value);
    fprintf(fou, "%5.2f ", debit);
    fprintf(fou, "%6d\n", tSimuSec);
    fclose(fou);
}

void append_results_to_file(
    const std::string &modulation,
    int                GFx,
    int                Nx,
    int                Kx,
    double             SNR,
    unsigned long      nb_err,
    unsigned long      nb_gen_frame)
{
    // Directory path
    fs::path dir = "results";

    // Create directory if not exists
    std::error_code ec;
    if (!fs::exists(dir))
    {
        if (!fs::create_directories(dir, ec))
        {
            std::cerr << "Error creating directory " << dir << ": " << ec.message() << "\n";
            return;
        }
    }

    // Compose filename
    fs::path filename = dir / (modulation + "_GF" + std::to_string(GFx) +
                               "_N" + std::to_string(Nx) +
                               "_K" + std::to_string(Kx) + ".txt");

    // Open file in append mode
    std::ofstream file(filename, std::ios::app);
    if (!file.is_open())
    {
        std::cerr << "Error opening file " << filename << " for appending.\n";
        return;
    }

    double FER_value = (nb_gen_frame == 0) ? 0.0 : static_cast<double>(nb_err) / nb_gen_frame;

    file << "SNR=" << SNR << " db,    FER = "
         << nb_err << "/" << nb_gen_frame << " = " << FER_value << "\n";
}

int main(int argc, char *argv[])
{
#ifdef __AVX512BW__
    printf("#(II) Non-binary FFT Successive Cancellation decoder evaluation program (AVX512 version)\n");
#elif __AVX2__
    printf("#(II) Non-binary FFT Successive Cancellation decoder evaluation program (AVX2 version)\n");
#else
    printf("#(II) Non-binary FFT Successive Cancellation decoder evaluation program (ARM NEON version)\n");
#endif

    printf("#(II) + developped by Abdallah ABDALLAH in 2025...\n");
    printf("#(II) +        and by Camille MONIERE   in 2025...\n");
    printf("#(II) +        and by Bertrand LE GAL   in 2025...\n");
    printf("#(II)\n");
    printf("#(II) Binary generated : %s - %s\n", __DATE__, __TIME__);

    if (argc != 6)
    {
        cout << "validate: NbMonteCarlo, SNR, q, N, K" << std::endl;
        return 1;
    }
    uint16_t   q, N, K, n, p, frozen_val = 0;
    softdata_t offset;
    uint64_t   NbMonteCarlo = stoi(argv[1]);
    float      EbN0         = stod(argv[2]);
    q                       = stoi(argv[3]);
    p                       = log2(q);
    N                       = stoi(argv[4]);
    K                       = stoi(argv[5]);
    n                       = log2(N);
    int FER_STOP            = 100000;

    base_code_t code_param(N, K, n, q, p, frozen_val);
    code_param.sig_mod = "CCSK_BIN";

    int   gf_rand_SEED       = 0;
    float nse_rand_SEED      = 0.2;
    bool  repeatable_randgen = 0;

    table_GF table;

    cout << "(II) Loading code_param [START]" << endl;
    LoadCode(code_param, EbN0);

    for (int i = 0; i < code_param.N; i++)
        cout << code_param.reliab_sequence[i] << " ";
    cout << endl;
    cout << "(II) Loading code_param [END OK]" << endl;
    // void LoadTables(base_code_t & code, table_GF & table,  const uint16_t *GF_polynom_primitive)

    cout << "(II) Loading tables [START]" << endl;
    LoadTables(code_param, table, GF_polynom_primitive.data());
    cout << "(II) Loading tables [END OK]" << endl;

    cout << "Simulation starts..." << endl;

    decoder_parameters dec_param(code_param);

    CCSK_seq                 ccsk_seq;
    vector<vector<uint16_t>> CCSK_rotated_codes(q, vector<uint16_t>());
    if (code_param.sig_mod == "CCSK_BIN")
        create_ccsk_rotated_table(ccsk_seq.CCSK_bin_seq[code_param.p - 2], ccsk_seq.CCSK_bin_seq[code_param.p - 2].size(), CCSK_rotated_codes);
    else if (code_param.sig_mod == "CCSK_NB")
        create_ccsk_rotated_table(ccsk_seq.CCSK_GF_seq[code_param.p - 2], ccsk_seq.CCSK_GF_seq[code_param.p - 2].size(), CCSK_rotated_codes);

    vector<vector<vector<int16_t>>> hst1(n, vector<vector<int16_t>>(N, vector<int16_t>(dec_param.nm, 0)));

    q = code_param.q;
    p = code_param.p;
    vector<vector<softdata_t>> bin_mod_dict;
    if (code_param.sig_mod == "CCSK_BIN")
    {
        bin_mod_dict.resize(q, vector<softdata_t>(q, 0));

        for (int i = 0; i < q; i++)
            for (int j = 0; j < q; j++)
                bin_mod_dict[i][j] = (CCSK_rotated_codes[i][j] == 0) ? 1 : -1;
    }

    dec_param.ucap.resize(n + 1, vector<uint16_t>(N, dec_param.MxUS));
    dec_param.ucap[n].assign(N, dec_param.frozen_val);
    uint64_t          FER_out = 0, gen_frames_out = 0;
    std::atomic<int>  global_counter(0);
    std::atomic<int>  FER(0);
    std::atomic<bool> stop(false);
    unsigned          base_seed = 0; // std::chrono::system_clock::now().time_since_epoch().count();

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //
    int frozen_symbols[N];
    for (int i = 0; i < N; i += 1)
        frozen_symbols[i] = true;
    for (int i = 0; i < K; i += 1)
        frozen_symbols[dec_param.reliab_sequence[i]] = false;
    //
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    double time_base[64];
    for (int i = 0; i < 64; i += 1)
        time_base[i] = 0.0;

    const auto s_start = std::chrono::system_clock::now();

#pragma omp parallel
    {
        PoAwN::structures::decoder_parameters dec_param_local = dec_param;
        int                                   thread_id       = omp_get_thread_num();
        std::mt19937                          gen(thread_id + base_seed);
        vector<vector<decoder_t>>             L(n + 1, vector<decoder_t>(N));

        for (int i = 0; i <= n; i++)
            for (int j = 0; j < N; j++)
            {
                L[i][j].intrinsic_LLR.resize(q, 0);
                L[i][j].is_freq = false;
            }

        vector<uint16_t> info_sec_rec(K, dec_param_local.MxUS);

        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //
        std::vector<uint16_t> decoded_n(N);

        // decoder_specialized_pruning<_GF_> dec(N, frozen_symbols);
        decoder *dec;
        dec = new decoder_naive_fixed<_GF_>(N, frozen_symbols);
        // dec = new decoder_naive_cfloat<_GF_>(N, frozen_symbols);

        symbols_t1 *chan_llr;
        chan_llr = new symbols_t1[_N_];

        //
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        vector<uint16_t> NSYMB(N);
        while (true)
        {

            bool             succ_dec = true;
            vector<uint16_t> KSYMB(K);

            EncodeChanBPSK_BinCCSK(gen, dec_param_local, table, EbN0, CCSK_rotated_codes, L[0], KSYMB, NSYMB, bin_mod_dict);
            // Hard decision

            for (int j0 = 0; j0 < N; j0++)
            {
                for (int j1 = 0; j1 < q; j1++)
                    chan_llr[j0].value[j1] = 0.95 * L[0][j0].intrinsic_LLR[j1];
            }
            vector<uint16_t> HD(code_param.N);
            for (int j0 = 0; j0 < N; j0++)
            {
                HD[j0]         = 0;
                softdata_t prb = chan_llr[j0].value[0];
                for (int j1 = 1; j1 < q; j1++)
                    if (chan_llr[j0].value[j1] > prb)
                    {
                        HD[j0] = j1;
                    }
            }

            const auto m_start = std::chrono::system_clock::now();
            dec->execute(chan_llr, decoded_n.data());
            const auto m_stop = std::chrono::system_clock::now();
            time_base[thread_id] += std::chrono::duration_cast<std::chrono::microseconds>(m_stop - m_start).count();

            for (int i = 0; i < K; i++)
                info_sec_rec[i] = decoded_n[dec_param.reliab_sequence[i]];

            for (uint16_t i = 0; i < dec_param_local.K; i++)
            {
                if (KSYMB[i] != info_sec_rec[i])
                {
                    succ_dec = false;
                    break;
                }
            }

            global_counter.fetch_add(1);
            int succ_now = global_counter.load() - FER.load();
            if (!succ_dec)
            {
                FER.fetch_add(1);
            }
            succ_now = global_counter.load() - FER.load();
            if ((global_counter % 100) == 0)
            {

#pragma omp critical
                {
                    int local_success = global_counter.load() - FER.load();
                    if ((global_counter.load() >= NbMonteCarlo) || (FER.load() >= FER_STOP))
                        stop.store(true); // Set the flag
                    FER_out        = FER.load();
                    gen_frames_out = global_counter.load();
                    cout << "\rSNR: " << EbN0 << " dB, FER = " << FER
                         << "/" << global_counter << " = "
                         << (float)FER_out / gen_frames_out << std::flush;
                }
            }
            if (stop.load())
                break;
        }
    }
    const auto s_stop   = std::chrono::system_clock::now();
    const int  tSimuSec = std::chrono::duration_cast<std::chrono::seconds>(s_stop - s_start).count();

    double total_us = 0.0;
    for (int i = 0; i < 64; i += 1)
        total_us = (total_us >= time_base[i]) ? total_us : time_base[i];
    const float time_run = (total_us / (double)gen_frames_out);
    const float debit    = ((double)N * (double)_logGF_) / time_run;

    cout << "\rSNR: " << EbN0 << " dB, FER = " << FER_out << "/" << gen_frames_out
         << " = " << (float)FER_out / (float)gen_frames_out << std::flush;
    cout << " :: débit = " << debit << " Mbps";
    cout << endl;

    append_results_to_file(
        dec_param.sig_mod.c_str(),
        dec_param.q,
        dec_param.N,
        dec_param.K,
        EbN0,
        FER_out,
        gen_frames_out,
        debit,
        tSimuSec);
}