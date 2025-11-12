#pragma once

#include "decoders/features/archi.hpp"

// #define debug_f_function
template <uint32_t gf_size>
inline __attribute__((always_inline)) void f_function_proba_in(
    symbols_s<gf_size> *__restrict dst,
    const symbols_s<gf_size> *__restrict src_a,
    const symbols_s<gf_size> *__restrict src_b)
{
    symbols_s<gf_size> tmp_a;
    for (int i = 0; i < int(gf_size); i++)
        tmp_a.value[i] = src_a->value[i];

    FWHT_NORM<gf_size>(tmp_a.value);

    symbols_s<gf_size> tmp_b;
    for (int i = 0; i < int(gf_size); i++)
        tmp_b.value[i] = src_b->value[i];

    FWHT_NORM<gf_size>(tmp_b.value);

    //
    // Element-wise multiplication of the two input symbols because we are in frequency domain !
    //
    for (size_t i = 0; i < gf_size; i++)
    {
        dst->value[i] = 10.f * tmp_a.value[i] * tmp_b.value[i]; // TODO : attention au facteur 10x qui est magique !!!
    }
}
//
//
//
//
//
