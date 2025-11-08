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

template <int gf_size>
void g_function_freq_in(
    symbols_t *__restrict dst,        // the data to be computed for the left side of the graph
    symbols_t *__restrict src_a,      // the upper value set from the right side of the graph
    symbols_t *__restrict src_b,      // the lower value set from the right side of the graph
    const uint16_t *__restrict src_c, // the computed symbols coming from the left side of the graph
    const int n_symbols)
{
    for (int s = 0; s < n_symbols; s++)
    {
        FWHT_NORM<gf_size>(src_a[s].value);
        src_a[s].is_freq = false;

        FWHT_NORM<gf_size>(src_b[s].value);
        src_b[s].is_freq = false;

        for (int i = 0; i < gf_size; i++)
        {
            const int idx = src_c[s] ^ i;
            const float val = src_a[s].value[i] * src_b[s].value[idx];
            dst[s].value[idx] = val;
        }
        normalize<gf_size>(dst[s].value);
        dst[s].is_freq = false;
    }
}
//
//
//
//
//
template <int gf_size, int n_symbols>
inline __attribute__((always_inline)) void g_function_freq_in(
    symbols_t *__restrict dst,       // the data to be computed for the left side of the graph
    symbols_t *__restrict src_a,     // the upper value set from the right side of the graph
    symbols_t *__restrict src_b,     // the lower value set from the right side of the graph
    const uint16_t *__restrict src_c // the computed symbols coming from the left side of the graph
)
{
    for (int s = 0; s < n_symbols; s++)
    {
        FWHT_NORM<gf_size>(src_a[s].value);
        src_a[s].is_freq = false;

        FWHT_NORM<gf_size>(src_b[s].value);
        src_b[s].is_freq = false;

        for (int i = 0; i < gf_size; i++)
        {
            const int idx = src_c[s] ^ i;
            const float val = src_a[s].value[i] * src_b[s].value[idx];
            dst[s].value[idx] = val;
        }
        normalize<gf_size>(dst[s].value);
        dst[s].is_freq = false;
    }
}
//
//
//
//
//
template <int gf_size>
inline __attribute__((always_inline)) void g_function_freq_in_after_rate_0(
    symbols_t *__restrict dst,   // the data to be computed for the left side of the graph
    symbols_t *__restrict src_a, // the upper value set from the right side of the graph
    symbols_t *__restrict src_b, // the lower value set from the right side of the graph
    const int n_symbols)
{
    for (int s = 0; s < n_symbols; s++)
    {
        FWHT_NORM<gf_size>(src_a[s].value);
        src_a[s].is_freq = false;

        FWHT_NORM<gf_size>(src_b[s].value);
        src_b[s].is_freq = false;

        for (int i = 0; i < gf_size; i++)
        {
            // const int   idx = src_c[s] ^ i;
            const float val = src_a[s].value[i] * src_b[s].value[/*idx*/ i];
            dst[s].value[/*idx*/ i] = val;
        }
        normalize<gf_size>(dst[s].value);
        dst[s].is_freq = false;
    }
}

#else

template <int gf_size>
void g_function_freq_in(
    symbols_t *__restrict dst,        // the data to be computed for the left side of the graph
    symbols_t *__restrict src_a,      // the upper value set from the right side of the graph
    symbols_t *__restrict src_b,      // the lower value set from the right side of the graph
    const uint16_t *__restrict src_c, // the computed symbols coming from the left side of the graph
    const int n_symbols)
{
    for (int s = 0; s < n_symbols; s++)
    {
        for (size_t i = 0; i < gf_size; i++)
            dst[s].value[i] = src_a[s].value[i] * Hadamard[src_c[s]][i];

        FWHT_NORM<gf_size>(dst[s].value);
        dst[s].is_freq = false;

        FWHT_NORM<gf_size>(src_b[s].value);
        // const float fact = 1/float(gf_size); if src_b will not be further used, no need to normalize
        // normalize<gf_size>(src_b[s].value, fact);
        src_b[s].is_freq = false;

        for (size_t i = 0; i < gf_size; i++)
            dst[s].value[i] = dst[s].value[i] * src_b[s].value[i];

        normalize<gf_size>(dst[s].value);
        dst[s].is_freq = false;
    }
}
//
//
//
//
//
template <int gf_size, int n_symbols>
inline __attribute__((always_inline)) void g_function_freq_in(
    symbols_t *__restrict dst,       // the data to be computed for the left side of the graph
    symbols_t *__restrict src_a,     // the upper value set from the right side of the graph
    symbols_t *__restrict src_b,     // the lower value set from the right side of the graph
    const uint16_t *__restrict src_c // the computed symbols coming from the left side of the graph
)
{
    for (int s = 0; s < n_symbols; s++)
    {
        for (size_t i = 0; i < gf_size; i++)
            dst[s].value[i] = src_a[s].value[i] * Hadamard[src_c[s]][i];

        FWHT_NORM<gf_size>(dst[s].value);
        dst[s].is_freq = false;

        FWHT_NORM<gf_size>(src_b[s].value);
        // const float fact = 1/float(gf_size); if src_b will not be further used, no need to normalize
        // normalize<gf_size>(src_b[s].value, fact);
        src_b[s].is_freq = false;

        for (size_t i = 0; i < gf_size; i++)
            dst[s].value[i] = dst[s].value[i] * src_b[s].value[i];

        normalize<gf_size>(dst[s].value);
        dst[s].is_freq = false;
    }
}
//
//
//
//
//
template <int gf_size>
inline __attribute__((always_inline)) void g_function_freq_in_after_rate_0(
    symbols_t *__restrict dst,   // the data to be computed for the left side of the graph
    symbols_t *__restrict src_a, // the upper value set from the right side of the graph
    symbols_t *__restrict src_b, // the lower value set from the right side of the graph
    const int n_symbols)
{
    for (int s = 0; s < n_symbols; s++)
    {
        FWHT_NORM<gf_size>(src_a[s].value);
        src_a[s].is_freq = false;

        FWHT_NORM<gf_size>(src_b[s].value);
        src_b[s].is_freq = false;

        for (int i = 0; i < gf_size; i++)
        {
            // const int   idx = src_c[s] ^ i;
            const float val = src_a[s].value[i] * src_b[s].value[/*idx*/ i];
            dst[s].value[/*idx*/ i] = val;
        }
        normalize<gf_size>(dst[s].value);
        dst[s].is_freq = false;
    }
}
#endif
//
//
//
//
//