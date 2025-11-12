#pragma once
//
//
//
//
//
#include "features/archi.hpp"
//
//
//
//
//
// #define debug_g_function
template <int gf_size> inline __attribute__((always_inline))
void g_function_freq_in(
    symbols_s<gf_size> * __restrict dst,   // the data to be computed for the left side of the graph
    symbols_s<gf_size> * __restrict src_a, // the upper value set from the right side of the graph
    symbols_s<gf_size> * __restrict src_b, // the lower value set from the right side of the graph
    const uint32_t    src_c)    // the computed symbols coming from the left side of the graph
{
    FWHT_NORM<gf_size>(src_a->value);
    FWHT_NORM<gf_size>(src_b->value);

    for (size_t i = 0; i < gf_size; i++) {
        const int   idx = src_c ^ i;
        const float val = src_a->value[i] * src_b->value[idx];
        dst->value[idx] = val;
    }
    normalize<gf_size>(dst->value);
}
//
//
//
//
//
