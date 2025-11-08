// #pragma once

// #include "features/archi.hpp"

//

#pragma once

#include "features/archi.hpp"
#include "definitions/custom_types.hpp"

#ifdef ABD_OPTIM
#if _GF_ == 16
#include "hadamard/hadamard_64.hpp"
#elif _GF_ == 32
#include "hadamard/hadamard_128.hpp"
#elif _GF_ == 64
#include "hadamard/hadamard_64.hpp"
#elif _GF_ == 128
#include "hadamard/hadamard_128.hpp"
#elif _GF_ == 256
#include "hadamard/hadamard_256.hpp"
#elif _GF_ == 512
#include "hadamard/hadamard_512.hpp"
#elif _GF_ == 1024
#include "hadamard/hadamard_1024.hpp"
#elif _GF_ == 1024
#include "hadamard/hadamard_2048.hpp"
#elif _GF_ == 1024
#include "hadamard/hadamard_4096.hpp"
#endif
#endif

#ifndef ABD_OPTIM
// #define debug_g_function
template <int gf_size>
inline __attribute__((always_inline)) void g_function_freq_in(
    symbols_t *__restrict dst,   // the data to be computed for the left side of the graph
    symbols_t *__restrict src_a, // the upper value set from the right side of the graph
    symbols_t *__restrict src_b, // the lower value set from the right side of the graph
    const uint32_t src_c)        // the computed symbols coming from the left side of the graph
{
    FWHT_NORM<gf_size>(src_a->value);
    src_a->is_freq = false;

    FWHT_NORM<gf_size>(src_b->value);
    src_b->is_freq = false;

// #define BAD_OPTIMIZATION
#if defined(BAD_OPTIMIZATION)
    float sum = 0.f;
#endif
    for (size_t i = 0; i < gf_size; i++)
    {
        const int idx = src_c ^ i;
        const float val = src_a->value[i] * src_b->value[idx];
        dst->value[idx] = val;
#if defined(BAD_OPTIMIZATION)
        sum += val;
#endif
    }
#if defined(BAD_OPTIMIZATION)
    const float inv_sum = 1.f / sum;
    for (size_t i = 0; i < gf_size; i++)
    {
        dst->value[i] *= inv_sum;
    }
#else
    normalize<gf_size>(dst->value);
#endif
    dst->is_freq = false;
}

#else
// #define debug_g_function
template <int gf_size>
inline __attribute__((always_inline)) void g_function_freq_in(
    symbols_t *__restrict dst,   // the data to be computed for the left side of the graph
    symbols_t *__restrict src_a, // the upper value set from the right side of the graph
    symbols_t *__restrict src_b, // the lower value set from the right side of the graph
    const uint32_t src_c)        // the computed symbols coming from the left side of the graph
{
    for (size_t i = 0; i < gf_size; i++)
        dst->value[i] = src_a->value[i] * Hadamard[src_c][i];

    fwht<gf_size>(dst->value);
    dst->is_freq = false;

    fwht<gf_size>(src_b->value);
    // const float fact = 1/float(gf_size); if src_b will not be further used, no need to normalize
    // normalize<gf_size>(src_b->value, fact);
    src_b->is_freq = false;

    for (size_t i = 0; i < gf_size; i++)
        dst->value[i] = dst->value[i] * src_b->value[i];

    normalize<gf_size>(dst->value);
    dst->is_freq = false;
}
#endif