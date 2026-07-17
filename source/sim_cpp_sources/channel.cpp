#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <vector>
#include "ccsk_simulator/simul_parameters.hpp"
#include "Decoder_functions.h"
#include "definitions/code.hpp"
#include "struct.h"
#include "tools.h"
#include "channel.h"
#include <random>

#define PI 3.14159265358979323846
#define RepRndGn false

using namespace PoAwN::structures;
using namespace PoAwN::tools;
using namespace PoAwN::decoding;
using std::vector;

namespace
{
uint16_t binary_ccsk_chip(const uint16_t index)
{
#if _GF_ == 64
    return CCSKSequences::BASE_SEQ_64[index];
#elif _GF_ == 128
    return CCSKSequences::BASE_SEQ_128[index];
#elif _GF_ == 256
    return CCSKSequences::BASE_SEQ_256[index];
#elif _GF_ == 512
    return CCSKSequences::BASE_SEQ_512[index];
#elif _GF_ == 1024
    return CCSKSequences::BASE_SEQ_1024[index];
#else
#error "Unsupported _GF_ for binary CCSK"
#endif
}
} // namespace

void PoAwN::channel::EncodeChanBPSK_BinCCSK(std::mt19937 &gen,
                                            uint16_t N,
                                            uint16_t q,
                                            uint16_t n,
                                            uint16_t frozen_val,
                                            const vector<uint16_t> &reliability_order,
                                            const float SNR,
                                            vector<decoder_t> &chan_LLR_sorted,
                                            vector<uint16_t> &KSYMB)
{
    float sigma = sqrt(1.0 / (pow(10, SNR / 10.0))); // N0/2 or N0?
    vector<uint16_t> NSYMB(N);
    std::uniform_int_distribution<int> unif_dist(0, q - 1);
    for (uint16_t k = 0; k < N; k++)
    {
        KSYMB[k] = (uint16_t)unif_dist(gen);
    }

    vector<vector<uint16_t>> ucap(n + 1, vector<uint16_t>(N, frozen_val));
    for (int i = 0; i < N; i++)
        ucap[n][reliability_order[i]] = KSYMB[i];
    Encoder(ucap, NSYMB);

    vector<vector<softdata_t>> noisy_sig(N, vector<softdata_t>(q, (softdata_t)0.0));
    std::normal_distribution<double> norm_dist(0, sigma);
    for (int i = 0; i < N; i++)
    {
        const uint16_t shift = q - NSYMB[i];
        for (int j = 0; j < q; j++)
        {
            const uint16_t chip = (shift + j) & (q - 1);
            noisy_sig[i][j] = static_cast<softdata_t>(binary_ccsk_chip(chip) + norm_dist(gen));
        }
    }

    vector<vector<softdata_t>> chan_LLR(N, vector<softdata_t>(q, 0.0f));
    Channel_LLR(noisy_sig, q, sigma, chan_LLR);
    for (int i = 0; i < N; i++)
    {
        chan_LLR_sorted[i].intrinsic_LLR = chan_LLR[i];
    }
}
