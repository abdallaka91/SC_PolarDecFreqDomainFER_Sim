#pragma once
//
//
//
//
//
#include "decoders/features/archi.hpp"
#include "definitions/custom_types.hpp"
//
//
//
//
//
template <int gf_size>
void f_function_freq_in(
    symbols_t *__restrict dst,
    const symbols_t *__restrict src_a,
    const symbols_t *__restrict src_b,
    const int n_symbols)
{
    for (int s = 0; s < n_symbols; s++)
    {
        for (int i = 0; i < gf_size; i++)
        {
            // TODO : attention au facteur 10x qui est magique !!!
            dst[s].value[i] = src_a[s].value[i] * src_b[s].value[i];
        }
        dst[s].is_freq = true;
    }
}
//
//
//
//
//
template <int gf_size, int n_symbols>
inline __attribute__((always_inline)) void f_function_freq_in(
    symbols_t *__restrict dst,
    const symbols_t *__restrict src_a,
    const symbols_t *__restrict src_b)
{
    for (int s = 0; s < n_symbols; s++)
    {
        for (int i = 0; i < gf_size; i++)
        {
            // TODO : attention au facteur 10x qui est magique !!!
            dst[s].value[i] = src_a[s].value[i] * src_b[s].value[i];
        }
        dst[s].is_freq = true;
    }
}
//
//
//
//
//
