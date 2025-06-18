#ifndef POAWN_FWHT_HPP
#define POAWN_FWHT_HPP

#include <cassert>

namespace PoAwN
{
using namespace PoAwN::structures;
    template <int N>
    inline void copy(softdata_t *dst, const softdata_t *src)
    {
        for (int i = 0; i < N; ++i)
            dst[i] = src[i];
    }
    template <int GF>
    inline void fwht(softdata_t x[])
    {
        if (!x)
            std::abort();
    }

    inline void fwht_tuile(softdata_t inp[8], softdata_t outp[8])
    {
        softdata_t L1[8], L2[8];
        L1[0] = inp[0] + inp[4];
        L1[1] = inp[1] + inp[5];
        L1[2] = inp[2] + inp[6];
        L1[3] = inp[3] + inp[7];
        L1[4] = inp[0] - inp[4];
        L1[5] = inp[1] - inp[5];
        L1[6] = inp[2] - inp[6];
        L1[7] = inp[3] - inp[7];

        L2[0] = L1[0] + L1[2];
        L2[2] = L1[0] - L1[2];
        L2[1] = L1[1] + L1[3];
        L2[3] = L1[1] - L1[3];
        L2[4] = L1[4] + L1[6];
        L2[6] = L1[4] - L1[6];
        L2[5] = L1[5] + L1[7];
        L2[7] = L1[5] - L1[7];

        outp[0] = L2[0] + L2[1];
        outp[1] = L2[0] - L2[1];
        outp[2] = L2[2] + L2[3];
        outp[3] = L2[2] - L2[3];
        outp[4] = L2[4] + L2[5];
        outp[5] = L2[4] - L2[5];
        outp[6] = L2[6] + L2[7];
        outp[7] = L2[6] - L2[7];
    }
    //
    //
    /////////////////////////////////////////////////////////////////////////////////////////////////////////
    //
    //
    template <>
    inline void fwht<16>(softdata_t inp[16])
    {
        softdata_t part_1[8], part_2[8];

        for (int i = 0; i < 8; i++)
            part_1[i] = inp[i] + inp[i + 8];
        for (int i = 0; i < 8; i++)
            part_2[i] = inp[i] - inp[i + 8];

        fwht_tuile(part_1, inp + 0);
        fwht_tuile(part_2, inp + 8);
    }
    //
    //
    /////////////////////////////////////////////////////////////////////////////////////////////////////////
    //
    //
    template <>
    inline void fwht<32>(softdata_t inp[32])
    {
        softdata_t part_1[16], part_2[16];

        for (int i = 0; i < 16; i++)
            part_1[i] = inp[i] + inp[i + 16];
        for (int i = 0; i < 16; i++)
            part_2[i] = inp[i] - inp[i + 16];

        fwht<16>(part_1);
        fwht<16>(part_2);

        PoAwN::copy<16>(inp + 0, part_1);
        PoAwN::copy<16>(inp + 16, part_2);
    }
    //
    //
    /////////////////////////////////////////////////////////////////////////////////////////////////////////
    //
    //
    template <>
    inline void fwht<64>(softdata_t inp[64])
    {
        softdata_t part_1[32], part_2[32];

        for (int i = 0; i < 32; i++)
            part_1[i] = inp[i] + inp[i + 32];
        for (int i = 0; i < 32; i++)
            part_2[i] = inp[i] - inp[i + 32];

        fwht<32>(part_1);
        fwht<32>(part_2);

        PoAwN::copy<32>(inp + 0, part_1);
        PoAwN::copy<32>(inp + 32, part_2);
    }
    //
    //
    /////////////////////////////////////////////////////////////////////////////////////////////////////////
    //
    //
    template <>
    inline void fwht<128>(softdata_t inp[128])
    {
        softdata_t part_1[64], part_2[64];

        for (int i = 0; i < 64; i++)
            part_1[i] = inp[i] + inp[i + 64];
        for (int i = 0; i < 64; i++)
            part_2[i] = inp[i] - inp[i + 64];

        fwht<64>(part_1);
        fwht<64>(part_2);

        PoAwN::copy<64>(inp, part_1);
        PoAwN::copy<64>(inp + 64, part_2);
    }
    //
    //
    /////////////////////////////////////////////////////////////////////////////////////////////////////////
    //
    //
    template <>
    inline void fwht<256>(softdata_t inp[256])
    {
        softdata_t part_1[128], part_2[128];

        for (int i = 0; i < 128; i++)
            part_1[i] = inp[i] + inp[i + 128];
        for (int i = 0; i < 128; i++)
            part_2[i] = inp[i] - inp[i + 128];

        fwht<128>(part_1);
        fwht<128>(part_2);

        PoAwN::copy<128>(inp, part_1);
        PoAwN::copy<128>(inp + 128, part_2);
    }

}
#endif