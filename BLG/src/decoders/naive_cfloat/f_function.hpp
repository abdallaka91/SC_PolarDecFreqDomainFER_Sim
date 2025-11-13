#pragma once

#include "definitions/custom_types.hpp"
#include "features/archi.hpp"
#include "features/archi.hpp"

template <uint32_t gf_size>
void f_function(symbols_cf * __restrict dst, symbols_cf * __restrict src_a, symbols_cf * __restrict src_b)
{
    if (src_a->is_freq == false) // Switch from time to frequency domain
    {
        fwht<gf_size>(src_a->value);
        src_a->is_freq = true;
    }
    if (src_b->is_freq == false)
    {
        fwht<gf_size>(src_b->value);
        src_b->is_freq = true;
    }
    //
    // Element-wise multiplication of the two input symbols because we are in frequency domain !
    //
    for (size_t i = 0; i < gf_size; i++) {
        dst->value[i] =  src_a->value[i] * src_b->value[i];
    }
    dst->is_freq = true;
}
//
//
//
//
//
