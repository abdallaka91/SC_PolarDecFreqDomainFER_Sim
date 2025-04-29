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

int main(int argc, char *argv[])
{

    if (argc != 10)
    {
        cout << "validate: NbMonteCarlo, SNR, sig_mod(BPSK, CCSK_bin, CCSK_NB), q, N, K, nH, nL, offset" << std::endl;
        return 1;
    }
    uint16_t q, N, K, n, nL, nH, nm, nb, Zc, nopM, p, frozen_val = 0;
    softdata_t offset;
    int NbMonteCarlo = stoi(argv[1]);
    float Pt1, Pt2, Pt, EbN0 = stod(argv[2]);
    string sig_mod = argv[3];
    std::transform(sig_mod.begin(), sig_mod.end(), sig_mod.begin(), ::toupper);
    q = stoi(argv[4]);
    p = log2(q);
    N = stoi(argv[5]);
    K = stoi(argv[6]);
    n = log2(N);
    nH = stoi(argv[7]);
    nL = stoi(argv[8]);
    nm = nL;
    Zc = 2;
    offset = stod(argv[9]);
    base_code_t code_param(N, K, n, q, p, frozen_val);
    code_param.sig_mod = sig_mod;

    int gf_rand_SEED = 0;
    float nse_rand_SEED = 1.2544;
    bool repeatable_randgen = 0;

    table_GF table;

    cout << "Loading code_param..." << endl;
    LoadCode(code_param, EbN0);
    cout << "OK!, " << "Loading tables..." << endl;
    // void LoadTables(base_code_t & code, table_GF & table,  const uint16_t *GF_polynom_primitive)
    LoadTables(code_param, table, GF_polynom_primitive.data());

    cout << "Done!" << endl;
    cout << "Simulation starts..." << endl;

    decoder_parameters dec_param(code_param, offset, nm, nL, nH, nb, Zc, nopM);

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

    vector<vector<vector<vector<uint16_t>>>> Bt(n, vector<vector<vector<uint16_t>>>());
    vector<vector<vector<vector<float>>>> Cs(n, vector<vector<vector<float>>>());
    for (uint16_t l = 0; l < n; l++)
    {
        Cs[l].resize(pow(2, l));
        Bt[l].resize(pow(2, l));
        for (uint16_t s = 0; s < 1 << l; s++)
        {
            Cs[l][s].assign(nH, vector<float>(nL, 0));
            Bt[l][s].resize(N >> (l + 1), vector<uint16_t>(2, 65535));
        }
    }
    unsigned int FER = 0, FER_out=0, gen_frames_out=0;
    std::atomic<int> global_counter(0);
    std::atomic<int> succ_dec_frame(0);

#pragma omp parallel
    {
        vector<vector<vector<vector<uint16_t>>>> Bt1 = Bt;
        vector<vector<vector<vector<float>>>> Cs1_local = Cs;
        PoAwN::structures::decoder_parameters dec_param_local = dec_param;

        while (true)
        {
            if (succ_dec_frame >= NbMonteCarlo)
                break;

            bool succ_dec = true;
            vector<uint16_t> KSYMB(K);
            vector<uint16_t> info_sec_rec(K, dec_param_local.MxUS);
            vector<vector<decoder_t>> L(n + 1, vector<decoder_t>(N));
            for (int i = 0; i <= n; i++)
                for (int j = 0; j < N; j++)
                    L[i][j] = decoder_t(vector<softdata_t>(q), vector<uint16_t>(q));

            if (code_param.sig_mod == "CCSK_BIN")
                EncodeChanBPSK_BinCCSK(dec_param_local, table, EbN0, CCSK_rotated_codes, L[0], KSYMB, bin_mod_dict);
            else if (code_param.sig_mod == "CCSK_NB")
                EncodeChanGF_CCSK(dec_param_local, table, EbN0, CCSK_rotated_codes, L[0], KSYMB);
            else
                EncodeChanBPSK_BinCCSK(dec_param_local, table, EbN0, table.BINDEC, L[0], KSYMB, bin_mod_dict);

            decode_SC_bubble_gen(dec_param_local, table.ADDGF, table.MULGF, table.DIVGF, L, info_sec_rec, Bt1);

            for (uint16_t i = 0; i < dec_param_local.K; i++)
            {
                if (KSYMB[i] != info_sec_rec[i])
                {
                    succ_dec = false;
                    break;
                }
            }

            global_counter++; // Count all frames

            if (succ_dec)
            {
                int succ_now = succ_dec_frame.fetch_add(1) + 1;
                if (succ_now > NbMonteCarlo)
                    break; // Exceeded required successful frames, exit immediately


                for (uint16_t l = 0; l < n; l++)
                    for (uint16_t s = 0; s < (1U << l); s++)
                        for (uint16_t t = 0; t < (N >> (l + 1)); t++)
                            if (Bt1[l][s][t][0] != 65535 && Bt1[l][s][t][1] != 65535)
                            {
                                Cs1_local[l][s][Bt1[l][s][t][0]][Bt1[l][s][t][1]]++;
                                Bt1[l][s][t][0] = 65535;
                                Bt1[l][s][t][1] = 65535;
                            }

                if ((global_counter % 100) == 0 || succ_now==NbMonteCarlo)
                {
#pragma omp critical
                    {
                        FER_out=FER, gen_frames_out=global_counter;
                        cout << "\rSNR: " << EbN0 << " dB, FER = " << FER
                             << "/" << global_counter << " = "
                             << (float)FER_out / global_counter << std::flush;
                    }
                }
            }
            else
                FER++;
        }

#pragma omp critical
        {
            for (uint16_t l = 0; l < n; l++)
                for (uint16_t s = 0; s < (1U << l); s++)
                    for (uint16_t i = 0; i < nH; i++)
                        for (uint16_t j = 0; j < nL; j++)
                            Cs[l][s][i][j] += Cs1_local[l][s][i][j];
        }
    }

    cout << "\rSNR: " << EbN0 << " dB, FER = " << FER_out << "/" << (float)gen_frames_out
         << " = " << (float)FER_out / (float)gen_frames_out << std::flush;
    cout << endl;

    bool succ_writing, newsim;

    std::ostringstream fname;
    bool newclust = false;
    newsim = true;

    for (uint16_t l = 0; l < n; l++)
        for (uint16_t s = 0; s < N >> (n - l); s++)
            for (int j0 = 0; j0 < nH; j0++)
                for (int j1 = 0; j1 < nL; j1++)
                {
                    Cs[l][s][j0][j1] /= (float)(1 << (n - (l + 1))); // divide over nb of kernels in cluster (2^(n-l-1))
                    Cs[l][s][j0][j1] /= (float)NbMonteCarlo;         // sum of Vs buble is notmalized to be ~1 (if all bubbles are inside the matrix then sum=1)
                }

    string bubble_direct;
    if (code_param.sig_mod == "BPSK")
        bubble_direct = "./BubblesPattern/bpsk/N";
    else if (code_param.sig_mod == "CCSK_BIN")
        bubble_direct = "./BubblesPattern/ccsk_bin/N";
    else
        bubble_direct = "./BubblesPattern/ccsk_nb/N";
    fname.str("");
    fname.clear();
    fname << bubble_direct << code_param.N << "/ContributionMatrices/" << "bubbles_N" << code_param.N << "_K" << code_param.K << "_GF" << code_param.q
          << "_SNR" << std::fixed << std::setprecision(3) << EbN0 << "_" << dec_param.nH << "x" << dec_param.nL
          << "_Cs_mat.txt";
    std::string filename = fname.str();

    newsim = 1;
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < 1u << i; j++)
        {
            succ_writing = AppendClustBubblesToFile(fname.str(), Cs[i][j], i, j, newsim,
                                                    "Observations nb: " + std::to_string(NbMonteCarlo) + "\n\n");
            newsim = 0;
        }
    }
    std::filesystem::path filepath(fname.str());
    std::ofstream file(fname.str(), std::ios::app);

    if (!file.is_open())
    {
        std::cerr << "Error: Could not open file to write FER " << std::endl;
        return false;
    }

    file << "\rSNR: " << EbN0 << " dB, FER = " << FER_out << "/" << (float)gen_frames_out << " = " << (float)FER_out / (float)gen_frames_out << std::flush;
    file << endl;

    file.close();
    if (succ_writing)
        std::cout << "Cs Matrices written to: " << filename << std::endl;
}