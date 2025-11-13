
#include "../src/definitions/code.hpp"

#include "../src/decoders/specialized/decoder_specialized.hpp"

#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <thread>

using namespace std::chrono_literals;

#include "utilities/utility_functions.hpp"

// Regular bold text
#define BBLK "\e[1;30m"
#define BRED "\e[1;31m"
#define BGRN "\e[1;32m"
#define BYEL "\e[1;33m"
#define BBLU "\e[1;34m"
#define BMAG "\e[1;35m"
#define BCYN "\e[1;36m"
#define BWHT "\e[1;37m"

#include "decoders/dedicated/decoder_dedicated.hpp"
#include "decoders/naive/decoder_naive.hpp"
#include "decoders/naive_cfloat/decoder_naive_cfloat.hpp"
#include "decoders/naive_fixed/decoder_naive_fixed.hpp"
#include "decoders/naive_pruning/decoder_naive_pruning.hpp"
#include "decoders/specialized/decoder_specialized.hpp"
#include "decoders/specialized_pruning/decoder_specialized_pruning.hpp"

#include "demodulator/demodulator.hpp"
#include "encoder/polar_encoder.hpp"

struct env_simu {
    bool                   ended;
    int                    n_decoded;
    std::vector<symbols_t> llrs_n;
    std::vector<uint16_t>  decoded_n;
    decoder *              dec;
};

static void thread_run_decoder(env_simu * env) {
    while (env->ended == false) {
        env->dec->execute(env->llrs_n.data(), env->decoded_n.data());
        env->n_decoded += 1;
    }
}

#include "features/fwht/fwht_norm_neon_v2.hpp"
//
//
//
// In frozen symbol array, the value -1 means the symbol is frozen => (symbol =
// 0)
//
int main(int argc, char * argv[]) {

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // initialize the frozen symbols array
    //
    const int N         = _N_;
    const int GF        = _GF_;
    float     code_rate = 0.75f;
    int       K         = (int) (((float) N) * code_rate);
    int       nThreads  = 1;
    auto      run_time  = 30000ms;

    std::string dec_type   = "dec4";
    bool        is_colored = true;

    for (int i = 1; i < argc; i++) {
        if (std::string(argv[i]) == "--dec") {
            dec_type = std::string(argv[i + 1]);
            i += 1;
        } else if (std::string(argv[i]) == "--decoder") {
            dec_type = std::string(argv[i + 1]);
            i += 1;
        } else if (std::string(argv[i]) == "--no-color") {
            is_colored = false;
        } else if (std::string(argv[i]) == "--nocolor") {
            is_colored = false;
        } else if (std::string(argv[i]) == "--rate") {
            code_rate = std::atof(argv[i + 1]);
            K         = round(code_rate * N);
            code_rate = (float) K / (float) N;
            i += 1;
        } else if (std::string(argv[i]) == "--code-rate") {
            code_rate = std::atof(argv[i + 1]);
            K         = round(code_rate * N);
            code_rate = (float) K / (float) N;
            i += 1;
        } else if (std::string(argv[i]) == "--thread") {
            nThreads = std::atoi(argv[i + 1]);
            i += 1;
        } else if (std::string(argv[i]) == "--threads") {
            nThreads = std::atoi(argv[i + 1]);
            i += 1;
        } else if (std::string(argv[i]) == "--cores") {
            nThreads = std::atoi(argv[i + 1]);
            i += 1;
        } else if (std::string(argv[i]) == "--time") {
            const int sec = std::atoi(argv[i + 1]);
            run_time      = sec * 1000ms;
            i += 1;
        }
    }

#ifdef __AVX512BW__
    printf("#(II) Non-binary FFT Successive Cancellation decoder evaluation "
           "program (AVX512 version)\n");
#elif __AVX2__
    printf("#(II) Non-binary FFT Successive Cancellation decoder evaluation "
           "program (AVX2 version)\n");
#else
    printf("#(II) Non-binary FFT Successive Cancellation decoder evaluation "
           "program (ARM NEON version)\n");
#endif

    printf("#(II) + developped by Bertrand LE GAL   in 2025...\n");
    printf("#(II) +        and by Abdallah ABDALLAH in 2025...\n");
    printf("#(II) +        and by Camille MONIERE   in 2025...\n");
    printf("#(II)\n");
    printf("#(II) Binary generated : %s - %s\n", __DATE__, __TIME__);

#if defined(__clang__)
    /* Clang/LLVM. ---------------------------------------------- */
    printf("#(II) + Clang/LLVM version %d.%d.%d\n", __clang_major__,
           __clang_minor__, __clang_patchlevel__);
#elif defined(__ICC) || defined(__INTEL_COMPILER)
    /* Intel ICC/ICPC. ------------------------------------------ */
    printf("# + Intel ICC/ICPC version %d.%d\n", __INTEL_COMPILER,
           __INTEL_COMPILER_BUILD_DATE);
#elif defined(__GNUC__) || defined(__GNUG__)
    /* GNU GCC/G++. --------------------------------------------- */
    printf("#(II) + GNU GCC/G++ version %d.%d.%d\n", __GNUC__, __GNUC_MINOR__,
           __GNUC_PATCHLEVEL__);
#elif defined(_MSC_VER)
    /* Microsoft Visual Studio. --------------------------------- */
    printf("#(II) + Microsoft Visual Studio\n");
#else
#error "#(II) + Undetected compiler !"
#endif

#if (defined(__ICC) || defined(__INTEL_COMPILER)) == 0
    std::time_t t = std::time(nullptr);
    std::cout << "#(II) + Trace date and time : "
              << std::put_time(std::localtime(&t), "%c %Z") << '\n';
    printf("#(II)\n");
#endif

    std::cout << "#(II) Polar code parameters" << std::endl;
    std::cout << "#(II) ---------------------" << std::endl;
    std::cout << "#(II)" << std::endl;
    std::cout << "#(II) + GF equals : " << GF << std::endl;
    std::cout << "#(II) +  N equals : " << N << std::endl;
    std::cout << "#(II) +  K equals : " << K << std::endl;
    std::cout << "#(II) +  R equals : " << (int) (100.f * code_rate) << "\%"
              << std::endl;
    std::cout << "#(II)" << std::endl;
    std::cout << "#(II) +  Decoder  : " << dec_type << std::endl;
    std::cout << "#(II) +  nThreads : " << nThreads << std::endl;
    std::cout << "#(II)" << std::endl;
    //
    //
    //
    int frozen_symbols[N];
    for (int i = 0; i < N; i += 1)
        frozen_symbols[i] = true;
    for (int i = 0; i < K; i += 1)
        frozen_symbols[reliab_seq[i]] = false;
    //
    //
    //
    printf("#(II) Frozen matrix:\n");
    printf("#(II) -------------\n");
    printf("#(II) %3d | ", 0);
    for (int i = 0; i < N; i += 1) {
        if (((i % 8) == 0) && (i != 0))
            printf(" | ");
        if (((i % 32) == 0) && (i != 0))
            printf("\n#(II) %3d | ", i);
        printf("%2d ", frozen_symbols[i]);
    }
    printf(" |\n");
    //
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    std::vector<uint16_t>  symbol_k(K);
    std::vector<uint16_t>  symbol_n(N);
    std::vector<symbols_t> llrs_n(N);
    std::vector<uint16_t>  decoded_n(N);
    std::vector<uint16_t>  decoded_k(K);

    //
    //
    //

    for (int i = 0; i < K; i++) {
        symbol_k[i] = rand() % GF;
    }
    printf("#(II)\n");
    printf("#(II) K Generatred symbols (%3d) :\n", K);
    printf("#(II) ---------------------------\n");
    printf("#(II)");
    for (int i = 0; i < K; i += 1) {
        if (((i % 16) == 0))
            printf("\n#(II) %3d | ", i);

        if (is_colored)
            printf("\e[1;32m%2d\e[0m ", symbol_k[i]);
        else
            printf("%2d ", symbol_k[i]);
    }
    printf("\n");

    polar_encoder encoder(reliab_seq, K, N);
    encoder.encode(symbol_n.data(), symbol_k.data()); // dst <= F(src)

    printf("#(II)\n");
    printf("#(II) N Encoded symbols (%d):\n", N);
    printf("#(II) ---------------------------\n");
    printf("#(II)");
    for (int i = 0; i < N; i += 1) {
        if (((i % 16) == 0))
            printf("\n#(II) %3d | ", i);
        if (is_colored)
            printf("\e[1;32m%2d\e[0m ", symbol_n[i]);
        else
            printf("%2d ", symbol_n[i]);
    }
    printf("\n");

    //
    // No modulation and no noise there
    //
#if 0
    std::vector< std::vector< softdata_t >> noisy_sig(N, std::vector<softdata_t>(bin_table[0].size(), (softdata_t)0.0));

    for (int i = 0; i < int(noisy_sig.size()); i++)
        noisy_sig[i] = bin_mod_dict[ NSYMB[i] ];
    // awgn_channel_noise(sigma, RepRndGn, 0, noisy_sig);
    {
        uint16_t q1 = noisy_sig[0].size();
        vector<vector<softdata_t>> noise_table(N, vector<softdata_t>(q1, 0));
        std::normal_distribution<double> norm_dist(0, sigma);
        {
            for (int i = 0; i < N; i++)
            {

                for (int j = 0; j < q1; j++)
                {
                    noise_table[i][j] = (softdata_t)norm_dist(gen);
                }
            }
        }

        for (int i = 0; i < N; i++)
            for (int j = 0; j < q1; j++)
                noisy_sig[i][j] += noise_table[i][j];
    }
    vector<vector<softdata_t>> chan_LLR(N, vector<softdata_t>(q, 0));
    Channel_LLR(noisy_sig, bin_table, q, sigma, chan_LLR);
    for (int i = 0; i < N; i++)
    {
        chan_LLR_sorted[i].intrinsic_LLR = chan_LLR[i];
    }
#endif
    //
    //
    //
    demodulator<GF> demod(N);
    demod.demodulate(llrs_n.data(), symbol_n.data()); // dst <= F(src)
#if 0
    printf("\n\nN demodulated symbol LLRs:\n");
    for (int i = 0; i < N; i += 1) {
        printf("(DD) Symbol n°%d", i);
        show_symbols<GF>( llrs_n[i].value );
    }
    printf("\n");
#endif
    //
    //  decoding
    //

    decoder * dec;
    if (dec_type == "dec1") {
        dec = new decoder_naive<GF>(N, frozen_symbols);
    } else if (dec_type == "dec1_fixed") {
        dec = new decoder_naive_fixed<GF>(N, frozen_symbols);
    } else if (dec_type == "dec1_cfloat") {
        dec = new decoder_naive_cfloat<GF>(N, frozen_symbols);
    } else if (dec_type == "dec2") {
        dec = new decoder_naive_pruning<GF>(N, frozen_symbols);
    } else if (dec_type == "dec3") {
        dec = new decoder_specialized<GF>(N, frozen_symbols);
    } else if (dec_type == "dec4") {
        dec = new decoder_specialized_pruning<GF>(N, frozen_symbols);
    } else if (dec_type == "dec5") {
        dec = new decoder_dedicated<GF>(N, frozen_symbols);
    } else {
        printf("#(II) Error : unknown decoder type\n");
        exit(1);
    }
    //    decoder_naive<GF> decoder(N, frozen_symbols);
    dec->execute(llrs_n.data(), decoded_n.data());

    printf("#(II)\n");
    printf("#(II) N Decoded symbols (%3d) :\n", N);
    printf("#(II) ------------------------\n");
    printf("#(II)");
    for (int i = 0; i < N; i += 1) {
        if (((i % 16) == 0))
            printf("\n#(II) %3d | ", i);
        printf("%2d ", decoded_n[i]);
    }
    printf("\n");

    //
    //  Extracting initial K symbols
    //
    encoder.decode(decoded_k.data(), decoded_n.data()); // dst <= F(src)

    printf("#(II)\n");
    printf("#(II) K decoded symbols (%3d) :\n", K);
    printf("#(II) ------------------------\n");
    printf("#(II)");
    for (int i = 0; i < K; i += 1) {
        if (((i % 16) == 0))
            printf("\n#(II) %3d | ", i);
        if (is_colored) {
            if (symbol_k[i] == decoded_k[i]) {
                printf("\e[1;32m%2d\e[0m ", decoded_k[i]);
            } else {
                printf("\e[1;31m%2d\e[0m ", decoded_k[i]);
            }
        } else {
            printf("%2d ", decoded_k[i]);
        }
    }
    /*
        printf("\n");
        printf("\n\n:\n");
        for (int i = 0; i < K; i += 1) {
            if ((i % 16) == 0)
                printf("\n ");
            if (symbol_k[i] == decoded_k[i]) {
                printf("\e[1;32m%2d\e[0m ", decoded_k[i]);
            } else {
                printf("\e[1;31m%2d\e[0m ", decoded_k[i]);
            }
        }
    */
    printf("\n");

    //
    //
    ///////////////////////////////////////////////////////////////////////////////
    //
    // Call the top node function to decode the symbols
    //

    // We should have the same symbols
    int nErrors = 0;
    for (int i = 0; i < K; i += 1) {
        if (symbol_k[i] != decoded_k[i]) {
            nErrors += 1;
        }
    }
    printf("#(II)\n");
    if (nErrors == 0) {
        printf("#(II) Decoder behavior : OK\n");
    } else {
        printf("#(II) Decoder behavior : ERROR\n");
    }

#if 0
    double nRunTest = 0;
    double nTotalus = 0;

    const int32_t nTest = (1024 * 1024 / GF);
    const auto debut = std::chrono::system_clock::now();
    for (int runs = 0; runs < 256*65536; runs += 1) {
        const auto start_x86 = std::chrono::system_clock::now();
        for(int32_t loop = 0; loop < nTest; loop += 1)
        {
            dec->execute(llrs_n.data(), decoded_n.data());
        }
        const auto stop_x86 = std::chrono::system_clock::now();

        const auto time_ns= std::chrono::duration_cast<std::chrono::nanoseconds>(stop_x86 - start_x86).count();
        const auto time_sec  = time_ns / 1000.f / 1000.f / 1000.f; // in seconds
        const auto time_msec = time_ns / 1000.f / 1000.f; // in seconds
        const auto time_usec = time_ns / 1000.f; // in seconds

        nRunTest += nTest;
        nTotalus += time_usec;

        const double time_run  = (nTotalus / nRunTest);

        const auto debit = ((double)N * (double)_logGF_) / time_run; // in Ksymbols/s
        if ( runs == 0 ) {
            printf("#(II)\n");
            printf("#(II) [GF=%d, N=%d, k=%d : SPEC] experiments    : %1.3f sec\n",  GF, N, K, time_sec);
            printf("#(II) [GF=%d, N=%d, k=%d : SPEC] experiments    : %1.2f ms\n",   GF, N, K, time_msec);
            printf("#(II) [GF=%d, N=%d, k=%d : SPEC] decoded frames : %d\n",   GF, N, K, nRunTest);
            printf("#(II) [GF=%d, N=%d, k=%d : SPEC] one decoding   : %1.2f us\n",   GF, N, K, time_run);
            printf("#(II) [GF=%d, N=%d, k=%d : SPEC] debit coded    : %1.2f Mbps\n", GF, N, K, debit);
            printf("#(II)\n");
            printf("#(II) Running 30s burning test !\n");
        }
        const auto curr = std::chrono::system_clock::now();
        const float ctime= std::chrono::duration_cast<std::chrono::seconds>(curr - debut).count();
        if ( ctime > 60.f ){
            printf("#(II) [GF=%d, N=%d, k=%d : SPEC] experiments    : %1.3f sec\n",  GF, N, K, ctime);
            printf("#(II) [GF=%d, N=%d, k=%d : SPEC] decoded frames : %d\n",   GF, N, K, nRunTest);
            printf("#(II) [GF=%d, N=%d, k=%d : SPEC] debit coded    : %1.2f Mbps\n", GF, N, K, debit);
            printf("#(II)\n");
            printf("%d %d %d %f %f\n", N, K, GF, debit, time_run);
            break;
        }
    }


    //
    // On cree les thread qui sont automatiquement lancés
    //
#if 0
    std::thread t [nThreads];
    for (int i = 0; i < length; i += step)
    {
        t[i/step] = std::thread(
            thread_vec_max_index, src,
            i, i + step,
            priv_maxv + (i/step),     // PAS de passage par REF sinon std::ref()
            priv_idx  + (i/step)      // PAS de passage par REF sinon std::ref()
            );
    }
#endif
    //
    // On attend que tous les threads aient terminé
    //
#endif

    printf("#(II)\n");
    std::cout << "#(II) +  Launching throughput measurement (" << dec_type << ")"
              << std::endl;
    printf("#(II)\n");

    if (nThreads != 0) {
        std::vector<env_simu> liste(nThreads);
        //        env_simu liste[nThreads];
        for (int i = 0; i < nThreads; i += 1) {
            liste[i].ended     = false;
            liste[i].n_decoded = 0;
            liste[i].llrs_n    = llrs_n;
            liste[i].decoded_n = decoded_n;
            if (dec_type == "dec1") {
                liste[i].dec = new decoder_naive<GF>(N, frozen_symbols);
            } else if (dec_type == "dec1_fixed") {
                liste[i].dec = new decoder_naive_pruning<GF>(N, frozen_symbols);
            } else if (dec_type == "dec1_cfloat") {
                liste[i].dec = new decoder_naive_cfloat<GF>(N, frozen_symbols);
            } else if (dec_type == "dec2") {
                liste[i].dec = new decoder_naive_pruning<GF>(N, frozen_symbols);
            } else if (dec_type == "dec3") {
                liste[i].dec = new decoder_specialized<GF>(N, frozen_symbols);
            } else if (dec_type == "dec4") {
                liste[i].dec = new decoder_specialized_pruning<GF>(N, frozen_symbols);
            } else if (dec_type == "dec5") {
                liste[i].dec = new decoder_dedicated<GF>(N, frozen_symbols);
            } else {
                printf("#(II) Error : unknown decoder type\n");
                exit(1);
            }
        }

        std::thread t_runs[128];
        const auto  m_start = std::chrono::system_clock::now();

        for (int i = 0; i < nThreads; i += 1)
            t_runs[i] = std::thread(thread_run_decoder, liste.data() + i);

        std::this_thread::sleep_for(run_time);

        for (int i = 0; i < nThreads; ++i)
            liste[i].ended = true;

        for (int i = 0; i < nThreads; ++i)
            t_runs[i].join();

        const auto m_stop = std::chrono::system_clock::now();

        int fRunTest = 0;
        for (int i = 0; i < nThreads; ++i)
            fRunTest += liste[i].n_decoded;

        const float nTotalus = std::chrono::duration_cast<std::chrono::microseconds>(m_stop - m_start)
                                   .count();
        const float time_run = (nTotalus / fRunTest);
        const float debit    = ((double) N * (double) _logGF_) / time_run;
        printf("#(II)\n");
        printf("#(II) #threads exec. : %d\n", nThreads);
        printf("#(II) MultiCore time : %1.3f sec\n", nTotalus / 1000000.f);
        printf("#(II) #decode frames : %d\n", fRunTest);
        printf("#(II) Coded through .: %1.3f Mbps\n", debit);
        printf("#(II)\n");

        const int   icode_rate = (int) (100.f * code_rate);
        const float debit_info = (int) (debit * code_rate);
        printf("%4d %4d %4d %4d %7.2f %7.2f %5d\n", N, K, icode_rate, GF, debit,
               debit_info, (int) time_run);

        for (int i = 0; i < nThreads; i++) {
            delete liste[i].dec;
        }
    }

    delete dec;

    return EXIT_SUCCESS;
}
