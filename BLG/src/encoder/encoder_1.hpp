
#ifndef POLAR_ENCODE
#define POLAR_ENCODE
#pragma once
#include <cassert>
#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <cmath>

template <uint16_t GF>
inline void polar_encode(uint16_t x[])
{
    assert(x != 0);
    assert(true);
    exit(x != NULL);
}

inline void polar_encode_tuile(const uint16_t inp[8], uint16_t outp[8])
{
    uint16_t L1[8], L2[8];
    L1[0] = inp[0] ^ inp[1];
    L1[1] = inp[1];
    L1[2] = inp[2] ^ inp[3];
    L1[3] = inp[3];
    L1[4] = inp[4] ^ inp[5];
    L1[5] = inp[5];
    L1[6] = inp[6] ^ inp[7];
    L1[7] = inp[7];

    L2[0] = L1[0] ^ L1[2];
    L2[1] = L1[1] ^ L1[3];
    L2[2] = L1[2];
    L2[3] = L1[3];
    L2[4] = L1[4] ^ L1[6];
    L2[5] = L1[5] ^ L1[7];
    L2[6] = L1[6];
    L2[7] = L1[7];

    outp[0] = L2[0] ^ L2[4];
    outp[1] = L2[1] ^ L2[5];
    outp[2] = L2[2] ^ L2[6];
    outp[3] = L2[3] ^ L2[7];
    outp[4] = L2[4];
    outp[5] = L2[5];
    outp[6] = L2[6];
    outp[7] = L2[7];
}

template <>
inline void polar_encode<8>(uint16_t* inp)// inp[8])
{
    uint16_t part_1[8];
    for (int i = 0; i < 8; i++)
    {
        part_1[i] = inp[i];
    }
    polar_encode_tuile(part_1, inp);
}

template <>
inline void polar_encode<16>(uint16_t* inp)// inp[16])
{
    uint16_t part_1[8];
    uint16_t part_2[8];

    for (int i = 0; i < 8; i++)
    {
        part_1[i] = inp[i];
        part_2[i] = inp[i + 8];
    }

    polar_encode_tuile(part_1, inp + 0);
    polar_encode_tuile(part_2, inp + 8);
    for (int i = 0; i < 8; i++)
    {
        inp[i] = inp[i] ^ inp[i + 8];
    }
}

template <>
inline void polar_encode<32>(uint16_t* inp)// inp[32])
{
    polar_encode<16>(inp);
    polar_encode<16>(inp + 16);

    for (int i = 0; i < 16; i++)
    {
        inp[i] = inp[i] ^ inp[i + 16];
    }
}

template <>
inline void polar_encode<64>(uint16_t* inp)// inp[64])
{
    polar_encode<32>(inp);
    polar_encode<32>(inp + 32);

    for (int i = 0; i < 32; i++)
    {
        inp[i] = inp[i] ^ inp[i + 32];
    }
}

template <>
inline void polar_encode<128>(uint16_t* inp)// inp[128])
{
    polar_encode<64>(inp);
    polar_encode<64>(inp + 64);

    for (int i = 0; i < 64; i++)
    {
        inp[i] = inp[i] ^ inp[i + 64];
    }
}

template <>
inline void polar_encode<256>(uint16_t* inp)// inp[256])
{
    polar_encode<128>(inp);
    polar_encode<128>(inp + 128);

    for (int i = 0; i < 128; i++)
    {
        inp[i] = inp[i] ^ inp[i + 128];
    }
}

template <>
inline void polar_encode<512>(uint16_t* inp)// inp[512])
{
    polar_encode<256>(inp);
    polar_encode<256>(inp + 256);

    for (int i = 0; i < 256; i++)
    {
        inp[i] = inp[i] ^ inp[i + 256];
    }
}

template <>
inline void polar_encode<1024>(uint16_t* inp)// inp[1024])
{
    polar_encode<512>(inp);
    polar_encode<512>(inp + 512);

    for (int i = 0; i < 512; i++)
    {
        inp[i] = inp[i] ^ inp[i + 512];
    }
}
#endif