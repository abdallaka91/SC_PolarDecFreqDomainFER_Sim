#ifndef CHANNEL
#define CHANNEL
#include "struct.h"

#include <cstdint>
#include <vector>
#include "struct.h"
#include <random>

namespace PoAwN
{
    namespace channel
    {
        using structures::decoder_t;
        using structures::softdata_t;
        using structures::vector;
        void EncodeChanBPSK_BinCCSK(std::mt19937 &gen,
                                    uint16_t N,
                                    uint16_t q,
                                    uint16_t n,
                                    uint16_t frozen_val,
                                    const vector<uint16_t> &reliability_order,
                                    const float SNR,
                                    vector<decoder_t> &chan_LLR_sorted,
                                    vector<uint16_t> &KSYMB);
    }
}

#endif
