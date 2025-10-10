#ifndef DECODER_FUNCTIONS
#define DECODER_FUNCTIONS
#include "struct.h"

#include <cstdint>
#include <vector>
#include <array>

namespace PoAwN
{

    namespace decoding
    {

        using structures::decoder_parameters;
        using structures::decoder_t;
        using structures::softdata_t;
        using structures::table_GF;
        using structures::vector;

        template <typename T, std::size_t N>
        inline std::array<T, N> elementwise_mul(const T *a, const T *b)
        {
            std::array<T, N> result{};
            for (std::size_t i = 0; i < N; ++i)
            {
                result[i] = a[i] * b[i];
                // result[i] /= static_cast<softdata_t>(N);
            }

            return result;
        }

        void Channel_LLR(const vector<vector<softdata_t>> &chan_observ,
                         const vector<vector<uint16_t>> &bin_symb_seq,
                         uint16_t q,
                         softdata_t sigma,
                         vector<vector<softdata_t>> &chan_LLR);

    }

}

#endif