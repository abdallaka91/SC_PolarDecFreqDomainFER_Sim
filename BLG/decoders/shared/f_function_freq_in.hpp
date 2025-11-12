#pragma once

#include "decoders/features/archi.hpp"

template <uint32_t gf_size>
inline __attribute__((always_inline)) void f_function_freq_in(
    symbols_t *__restrict dst,
    const symbols_t *__restrict src_a,
    const symbols_t *__restrict src_b)
{
    //
    // Element-wise multiplication of the two input symbols because we are in frequency domain !
    //
    for (size_t i = 0; i < int(gf_size); i++)
    {
        dst->value[i] = src_a->value[i] * src_b->value[i]; // TODO : attention au facteur 10x qui est magique !!!
    }
    dst->is_freq = true;
    //  normalize<gf_size>(dst->value);
}
//
//
//
//
//
