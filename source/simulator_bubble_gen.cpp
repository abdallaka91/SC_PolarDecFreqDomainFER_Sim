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
    // Pt1 = stod(argv[10]);
    // Pt2 = stod(argv[11]);
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

    vector<vector<vector<vector<uint16_t>>>> Bt;
    vector<vector<vector<vector<float>>>> Cs;
    vector<vector<vector<vector<uint64_t>>>> Rs;
    Cs.resize(n);
    Bt.resize(n);
    Rs.resize(n);

    for (uint16_t l = 0; l < n; l++)
    {
        dec_param.Roots_V[l].resize(1U << l, false);
        dec_param.Roots_indices[l].resize(pow(2, l));
        dec_param.clusts_CNs[l].resize(pow(2, l));
        dec_param.clusts_VNs[l].resize(pow(2, l));
        dec_param.coefs_id[l].resize(pow(2, l));
        Cs[l].resize(pow(2, l));
        Bt[l].resize(pow(2, l));
        for (uint16_t s = 0; s < dec_param.Roots_V[l].size(); s++)
        {
            Cs[l][s].assign(nH, vector<float>(nL, 0));
            Bt[l][s].assign(nH, vector<uint16_t>(nL, 0));
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

    vector<vector<decoder_t>> L(n + 1, vector<decoder_t>(N));
    vector<uint16_t> info_sec_rec(K, dec_param.MxUS);
    unsigned int FER = 0;
    vector<uint16_t> KSYMB(K);
    bool succ_dec, succ_writing, newsim;
    int succ_dec_frame = 0, i0 = 1;
    dec_param.ucap.resize(n + 1, vector<uint16_t>(N, dec_param.MxUS));
    dec_param.ucap[n].assign(N, dec_param.frozen_val);

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

    while (succ_dec_frame < NbMonteCarlo)
    {
        // dec_param.cnd1.assign(n, vector<int16_t>(N, -1));
        succ_dec = 1;
        for (int i = 0; i <= n; i++)
            for (int j = 0; j < N; j++)
                L[i][j] = decoder_t(vector<softdata_t>(q), vector<uint16_t>(q));
        if (code_param.sig_mod == "CCSK_BIN")
            EncodeChanBPSK_BinCCSK(dec_param, table, EbN0, CCSK_rotated_codes, L[0], KSYMB, bin_mod_dict);
        else if (code_param.sig_mod == "CCSK_NB")
            EncodeChanGF_CCSK(dec_param, table, EbN0, CCSK_rotated_codes, L[0], KSYMB);
        else
            EncodeChanBPSK_BinCCSK(dec_param, table, EbN0, table.BINDEC, L[0], KSYMB, bin_mod_dict);

        decode_SC_bubble_gen(dec_param, table.ADDGF, table.MULGF, table.DIVGF, L, info_sec_rec, Bt);
        for (uint16_t i = 0; i < dec_param.K; i++)
            if (KSYMB[i] != info_sec_rec[i])
            {
                succ_dec = false;
                break;
            }
        if (succ_dec)
        {
            succ_dec_frame++;
            for (uint16_t l = 0; l < n; l++)
                for (uint16_t s = 0; s < N >> (n - l); s++)
                    for (int j0 = 0; j0 < nH; j0++)
                        for (int j1 = 0; j1 < nL; j1++)
                            Cs[l][s][j0][j1] += (float)Bt[l][s][j0][j1];
        }
        else
            FER++;

        for (uint16_t l = 0; l < n; l++)
            for (uint16_t s = 0; s < N >> (n - l); s++)
                for (auto &rw : Bt[l][s])
                    for (auto &elem : rw)
                        elem = 0;

        if ((i0 % 100 == 0))
            cout << "\rSNR: " << EbN0 << " dB, FER = " << FER << "/" << (float)i0 << " = " << (float)FER / (float)i0 << std::flush;
        i0++;
    }
    i0--;

    cout << "\rSNR: " << EbN0 << " dB, FER = " << FER << "/" << (float)i0 << " = " << (float)FER / (float)i0 << std::flush;
    cout << endl;

    std::ostringstream fname;
    bool newclust = false;
    newsim = true;
    vector<vector<uint16_t>> cnt_1st(n), cnt_1st_1;

    int j00, j11, cnt0, cnt1;
    for (uint16_t l = 0; l < n; l++)
    {
        // if (l < 3)
        //     Pt = Pt1;
        // else
        //     Pt = Pt2;
        cnt_1st[l].assign(1 << l, 0);
        for (uint16_t s = 0; s < N >> (n - l); s++)
        {
            cnt0 = 0;
            cnt1 = 0;
            for (int j0 = 0; j0 < nH; j0++)
            {
                for (int j1 = 0; j1 < nL; j1++)
                {
                    Cs[l][s][j0][j1] /= (float)(1 << (n - (l + 1))); // divide over nb of kernels in cluster (2^(n-l-1))
                    Cs[l][s][j0][j1] /= (float)NbMonteCarlo;         // sum of Vs buble is notmalized to be ~1 (if all bubbles are inside the matrix then sum=1)
                    // Cs[l][s][j0][j1] *= 10000; //for easier readabiliy now scale the bubbles by 10000 and take the integer part only (sum now ~10000)
                    // Cs[l][s][j0][j1] = std::round(Cs[l][s][j0][j1]);
                    // if (Cs[l][s][j0][j1] > Pt)
                    // {
                    //     Bt[l][s][j0][j1] = 1;
                    //     if (j0 == 0)
                    //     {
                    //         j11 = j1;
                    //         cnt1++;
                    //     }
                    //     if (j1 == 0)
                    //     {
                    //         j00 = j0;
                    //         cnt0++;
                    //     }
                    // }
                }
            }
            // if (Bt[l][s][0][0])
            // {
            //     cnt_1st[l][s] = j11 + 1;
            //     if (cnt1 < j11 + 1)
            //         for (int j1 = 0; j1 <= j11; j1++)
            //             Bt[l][s][0][j1] = 1;
            //     if (cnt0 < j00 + 1)
            //         for (int j0 = 0; j0 <= j00; j0++)
            //             Bt[l][s][j0][0] = 1;
            // }
        }
    }

    string bubble_direct;
    if (code_param.sig_mod == "BPSK")
        bubble_direct = "./BubblesPattern/bpsk/N";
    else if (code_param.sig_mod == "CCSK_BIN")
        bubble_direct = "./BubblesPattern/ccsk_bin/N";
    else
        bubble_direct = "./BubblesPattern/ccsk_nb/N";
    // fname.str("");
    // fname.clear();
    // fname << "/mnt/c/Users/Abdallah Abdallah/Desktop/BubblePattern/"
    //       << "bubbles_N" << code_param.N << "_K" << code_param.K << "_GF" << code_param.q << "_SNR" << std::fixed << std::setprecision(3)
    //       << EbN0 << "_" << dec_param.nH << "x" << dec_param.nL "_Cs_mat.txt";
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

    file << "\rSNR: " << EbN0 << " dB, FER = " << FER << "/" << (float)i0 << " = " << (float)FER / (float)i0 << std::flush;
    file << endl;

    file.close();
    if (succ_writing)
        std::cout << "Cs Matrices written to: " << filename << std::endl;
}