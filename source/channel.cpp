#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <vector>
#include "Decoder_functions.h"
#include "GF_tools.h"
#include "init.h"
#include "struct.h"
#include "tools.h"
#include "channel.h"
#include "HelperFunc.h"
#include <random>

#define PI 3.14159265358979323846
#define RepRndGn false

using namespace PoAwN::structures;
using namespace PoAwN::tools;
using namespace PoAwN::init;
using namespace PoAwN::decoding;
using std::array;
using std::cout;
using std::endl;
using std::stod;
using std::stoi;
using std::string;
using std::vector;

void PoAwN::channel::EncodeChanBPSK_BinCCSK(std::mt19937 &gen,
                                            decoder_parameters &dec_param,
                                            const table_GF &table,
                                            const float SNR,
                                            const vector<vector<uint16_t>> &bin_table,
                                            vector<decoder_t> &chan_LLR_sorted,
                                            vector<uint16_t> &KSYMB,
                                            const vector<vector<softdata_t>> &bin_mod_dict)
{
    uint16_t N = dec_param.N, K = dec_param.K, q = dec_param.q;
    uint16_t nm = dec_param.nm;
    float sigma = sqrt(1.0 / (pow(10, SNR / 10.0))); // N0/2 or N0?
    vector<uint16_t> NSYMB(N);
    // RandomSymbGenerator(K, q, RepRndGn, 0, KSYMB);
    // std::mt19937 gen(0);
    std::uniform_int_distribution<int> unif_dist(0, q - 1);
    for (uint16_t k = 0; k < K; k++)
    {
        KSYMB[k] = (uint16_t)unif_dist(gen);
    }
    for (int i = 0; i < K; i++)
        dec_param.ucap[dec_param.n][dec_param.reliab_sequence[i]] = KSYMB[i];
    Encoder(table.ADDGF, table.MULGF, dec_param.polar_coeff, dec_param.ucap, NSYMB);

    vector<vector<softdata_t>> noisy_sig(N, vector<softdata_t>(bin_table[0].size(), (softdata_t)0.0));

    for (int i = 0; i < int(noisy_sig.size()); i++)
        noisy_sig[i] = bin_mod_dict[NSYMB[i]];
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


}



