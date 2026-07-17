//
// Created by legal on 03/07/2025.
//
#pragma once
//
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//
#warning "Basic c code was included (normalize_c.hpp)
#include <cstdint>
//
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//
template <int gf_size>
void normalize(float * tab) {
    float sum = 1e-32f;
    for (int i = 0; i < gf_size; i += 1) {
        sum += tab[i];
    }
    const float factor = 1.f / sum;
    for (int i = 0; i < gf_size; i++) {
        tab[i] *= factor;
    }
}
//
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//
int countLeadingZeros(uint32_t x) {
    if (x == 0) return 32;
    int n = 0;
    if ((x >> 16) == 0) { n += 16; x <<= 16; }
    if ((x >> 24) == 0) { n += 8; x <<= 8; }
    if ((x >> 28) == 0) { n += 4; x <<= 4; }
    if ((x >> 30) == 0) { n += 2; x <<= 2; }
    if ((x >> 31) == 0) { n += 1; }
    return n;
}
//
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//
template <int gf_size>
void normalize(int32_t* tab)
{
    uint32_t sum = 0;
    for (int i = 0; i < gf_size; i += 1)
    {
        sum |= tab[i];
    }

    const int nZeros = countLeadingZeros(sum);
    const int nShift = nZeros - 2; // on conserve 2 bits

    for (int i = 0; i < gf_size; i++) {
        tab[i] = tab[i] << nShift;
    }
}
//
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//
