#pragma once
//
//
//
//
//
#include "features/archi.hpp"

#include "definitions/const_config_GF64_N64.hpp"
//
//
//
//
//
// #define debug_g_function
template <int gf_size> inline __attribute__((always_inline))
void g_function_freq_in(
    symbols_t * __restrict dst,   // the data to be computed for the left side of the graph
    symbols_t * __restrict src_a, // the upper value set from the right side of the graph
    symbols_t * __restrict src_b, // the lower value set from the right side of the graph
    const uint32_t    src_c)    // the computed symbols coming from the left side of the graph
{
    FWHT_NORM<gf_size>(src_a->value);
    src_a->is_freq = false;

    FWHT_NORM<gf_size>(src_b->value);
    src_b->is_freq = false;

//#define BAD_OPTIMIZATION
#if defined(BAD_OPTIMIZATION)
    float sum = 0.f;
#endif
    for (size_t i = 0; i < gf_size; i++) {
        const int   idx = src_c ^ i;
        const float val = src_a->value[i] * src_b->value[idx];
        dst->value[idx] = val;
#if defined(BAD_OPTIMIZATION)
        sum            += val;
#endif
    }
#if defined(BAD_OPTIMIZATION)
    const float inv_sum = 1.f / sum;
    for (size_t i = 0; i < gf_size; i++) {
        dst->value[i] *= inv_sum;
    }
#else
    normalize<gf_size>(dst->value);
#endif
    dst->is_freq = false;
}
//
//
//
//
//
