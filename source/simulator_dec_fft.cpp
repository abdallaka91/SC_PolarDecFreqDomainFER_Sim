#include <cmath>
#include <cstdio>
#include <iostream>
#include <cstdlib>
#include <vector>
#include "Decoder_functions.h"
#include "GF_tools.h"
#include "init.h"
#include "struct.h"
#include "tools.h"
#include "HelperFunc.h"
#include <fstream>
#include <iomanip>
#include <cstring>
#include <string>
#include <iomanip>
#include <algorithm>
#include "channel.h"
#include <filesystem>
#include <omp.h>
#include <atomic>
#include <random>
#include "fwht.hpp"

#include "BLG/naive_pruning/decoder_naive_pruning.hpp"
#include "BLG/code.hpp"


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

#include <fstream>
#include <string>
#include <cstdio>
#include <iostream>

namespace fs = std::filesystem;

void append_results_to_file(
    const std::string &modulation,
    int GFx,
    int Nx,
    int Kx,
    double SNR,
    unsigned long nb_err,
    unsigned long nb_gen_frame)
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
    uint16_t q, N, K, n, p, frozen_val = 0;
    softdata_t offset;
    uint64_t NbMonteCarlo = stoi(argv[1]);
    float EbN0 = stod(argv[2]);
    q = stoi(argv[3]);
    p = log2(q);
    N = stoi(argv[4]);
    K = stoi(argv[5]);
    n = log2(N);
    int FER_STOP = 200;

    base_code_t code_param(N, K, n, q, p, frozen_val);
    code_param.sig_mod = "CCSK_BIN";

    int gf_rand_SEED = 0;
    float nse_rand_SEED = 1.2544;
    bool repeatable_randgen = 0;

    table_GF table;

    cout << "(II) Loading code_param [START]" << endl;
    LoadCode(code_param, EbN0);
    cout << "(II) Loading code_param [END OK]" << endl;
    // void LoadTables(base_code_t & code, table_GF & table,  const uint16_t *GF_polynom_primitive)

    cout << "(II) Loading tables [START]" << endl;
    LoadTables(code_param, table, GF_polynom_primitive.data());
    cout << "(II) Loading tables [END OK]" << endl;

    cout << "Simulation starts..." << endl;

    decoder_parameters dec_param(code_param);

    dec_param.Roots_V.resize(n + 1);
    dec_param.Roots_indices.resize(n);
    dec_param.clusts_CNs.resize(n);
    dec_param.clusts_VNs.resize(n);
    dec_param.coefs_id.resize(n);

    dec_param.Roots_V[n].resize(1U << n, false);

    for (uint16_t l = 0; l < n; l++)
    {
        dec_param.Roots_V[l].resize(1U << l, false);
        dec_param.Roots_indices[l].resize(pow(2, l));
        dec_param.clusts_CNs[l].resize(pow(2, l));
        dec_param.clusts_VNs[l].resize(pow(2, l));
        dec_param.coefs_id[l].resize(pow(2, l));

        for (uint16_t s = 0; s < dec_param.Roots_V[l].size(); s++)
        {
            uint16_t sz1 = N >> (l + 1U), sz2 = sz1 << 1U;
            dec_param.clusts_CNs[l][s].resize(sz1);
            dec_param.clusts_VNs[l][s].resize(sz1);
            dec_param.coefs_id[l][s].resize(sz1);
            for (uint16_t t = 0; t < sz1; ++t)
            {
                dec_param.clusts_CNs[l][s][t] = s * sz2 + (t << 1U) - (t % sz1);
                dec_param.clusts_VNs[l][s][t] = dec_param.clusts_CNs[l][s][t] + (N >> (l + 1U));
                dec_param.coefs_id[l][s][t] = dec_param.clusts_VNs[l][s][t] - (s + 1) * sz1;
            }
        }
        for (uint16_t s = 0; s < dec_param.Roots_indices[l].size(); s++)
        {

            uint16_t sz1 = N >> l;
            dec_param.Roots_indices[l][s].resize(sz1);
            for (uint16_t t = 0; t < sz1; ++t)
                dec_param.Roots_indices[l][s][t] = s * sz1 + t;
        }
    }
    frozen_lay_pos(dec_param, dec_param.ufrozen, dec_param.clst_frozen);
    CCSK_seq ccsk_seq;
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
    else if (code_param.sig_mod == "BPSK")
    {
        bin_mod_dict.resize(q, vector<softdata_t>(p, 0));
        for (int i = 0; i < q; i++)
            for (int j = 0; j < p; j++)
                bin_mod_dict[i][j] = (table.BINDEC[i][j] == 0) ? 1 : -1;
    }

    dec_param.ucap.resize(n + 1, vector<uint16_t>(N, dec_param.MxUS));
    dec_param.ucap[n].assign(N, dec_param.frozen_val);

    uint64_t FER_out = 0, gen_frames_out = 0;
    std::atomic<int> global_counter(0);
    std::atomic<int> FER(0);
    std::atomic<bool> stop(false);
    unsigned base_seed = 0; //std::chrono::system_clock::now().time_since_epoch().count();


    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //
    int frozen_symbols[N];
    for (int i = 0; i < N; i += 1)
        frozen_symbols[i] = true;
    for (int i = 0; i < K; i += 1)
        frozen_symbols[ dec_param.reliab_sequence[i] ] = false;
    //
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


    // const int base_seed = 42;
//#pragma omp parallel
    {
        PoAwN::structures::decoder_parameters dec_param_local = dec_param;
        int thread_id = omp_get_thread_num();
        std::mt19937 gen(thread_id + base_seed);
        vector<vector<decoder_t>> L(n + 1, vector<decoder_t>(N));
        vector<vector<decoder_t>> L_F(n + 1, vector<decoder_t>(N));

        for (int i = 0; i <= n; i++)
            for (int j = 0; j < N; j++)
            {
                L[i][j].intrinsic_LLR.resize(q, 0);
                L[i][j].intrinsic_GF.resize(q);
                iota(L[i][j].intrinsic_GF.begin(), L[i][j].intrinsic_GF.end(), 0);
            }

        L_F = L;
        vector<uint16_t> info_sec_rec(K, dec_param_local.MxUS);

        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //
        std::vector<uint16_t>   decoded_n(N);

        decoder_naive_pruning< _GF_ > dec(N, frozen_symbols);

        std::vector<symbols_t>  llrs_n   (N);
        //
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        while (true)
        {

            bool succ_dec = true;
            vector<uint16_t> KSYMB(K);

            EncodeChanBPSK_BinCCSK(gen, dec_param_local, table, EbN0, CCSK_rotated_codes, L[0], KSYMB, bin_mod_dict);

#if 0
            ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            //
            decode_SC_FFT(dec_param_local, table, L, L_F, info_sec_rec);
            //
            ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#else
            ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            //
            for (int i = 0; i < N; i++)
            {
                llrs_n[i].is_freq = false;
                for (int j = 0; j < _GF_; j++)
                    llrs_n[i].value[j] = L[0][i].intrinsic_LLR[j];
            }

            dec.execute(llrs_n.data(), decoded_n.data());


            for (int i = 0; i < K; i++)
                info_sec_rec[i] = decoded_n[ dec_param.reliab_sequence[i] ];
            //
            ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif

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
            if (
                ((global_counter % 10000) == 0) ||
                (succ_now == NbMonteCarlo)      ||
                (succ_dec == false)
            )
            {

#pragma omp critical
                {
                    int local_success = global_counter.load() - FER.load();
                    if ((global_counter.load() >= NbMonteCarlo) || (FER.load() >= FER_STOP))
                        stop.store(true); // Set the flag
                    FER_out = FER.load();
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
    cout << "\rSNR: " << EbN0 << " dB, FER = " << FER_out << "/" << gen_frames_out
         << " = " << (float)FER_out / (float)gen_frames_out << std::flush;
    cout << endl;

    append_results_to_file(
        dec_param.sig_mod.c_str(),
        dec_param.q,
        dec_param.N,
        dec_param.K,
        EbN0,
        FER_out,
        gen_frames_out);
}