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
void f_function_proba_in(
    symbols_t *__restrict dst,
    const symbols_t *__restrict src_a,
    const symbols_t *__restrict src_b,
    const int n_symbols)
{
    for (int s = 0; s < n_symbols; s++)
    {
        symbols_t tmp_a;
        for (int i = 0; i < gf_size; i++)
            tmp_a.value[i] = src_a[s].value[i];

        FWHT_NORM<gf_size>(tmp_a.value);
        tmp_a.is_freq = true;

        symbols_t tmp_b;
        for (int i = 0; i < gf_size; i++)
            tmp_b.value[i] = src_b[s].value[i];

        FWHT_NORM<gf_size>(tmp_b.value);
        tmp_b.is_freq = true;

        //
        // Element-wise multiplication of the two input symbols because we are in frequency domain !
        //
        for (size_t i = 0; i < gf_size; i++)
        {
            dst[s].value[i] = tmp_a.value[i] * tmp_b.value[i]; // TODO : attention au facteur 10x qui est magique !!!
        }
        dst[s].is_freq = true; // a.a we do CN in FD
    }
}
//
//
//
//
//
template <int gf_size, int n_symbols>
inline __attribute__((always_inline)) void f_function_proba_in(
    symbols_t *__restrict dst,
    const symbols_t *__restrict src_a,
    const symbols_t *__restrict src_b)
{
    for (int s = 0; s < n_symbols; s++)
    {
        symbols_t tmp_a;
        for (int i = 0; i < gf_size; i++)
            tmp_a.value[i] = src_a[s].value[i];

        FWHT_NORM<gf_size>(tmp_a.value);
        tmp_a.is_freq = true;

        symbols_t tmp_b;
        for (int i = 0; i < gf_size; i++)
            tmp_b.value[i] = src_b[s].value[i];

        FWHT_NORM<gf_size>(tmp_b.value);
        tmp_b.is_freq = true;

        //
        // Element-wise multiplication of the two input symbols because we are in frequency domain !
        //
        for (size_t i = 0; i < gf_size; i++)
        {
            dst[s].value[i] = tmp_a.value[i] * tmp_b.value[i]; // TODO : attention au facteur 10x qui est magique !!!
        }
        dst[s].is_freq = true; // a.a we do CN in FD
    }
}
//
//
//
//
//
