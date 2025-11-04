#pragma once

#include "custom_types.hpp"
#include "archi.hpp"

template <uint32_t gf_size>
void f_function(symbols_f * __restrict dst, symbols_f * __restrict src_a, symbols_f * __restrict src_b) {
    if (src_a->is_freq == false) // Switch from time to frequency domain
    {
        fwht_norm<gf_size>(src_a->value);
        src_a->is_freq = true;
    }
    if (src_b->is_freq == false) // Switch from time to frequency domain
    {
        fwht_norm<gf_size>(src_b->value);
        src_b->is_freq = true;
    }
    //
    // Element-wise multiplication of the two input symbols because we are in frequency domain !
    //
    for (size_t i = 0; i < gf_size; i++) {
        ap_fixed<NBITS, NFRAC> dix = 6.f;
        dst->value[i] =  dix * src_a->value[i] * src_b->value[i]; // TODO : attention au facteur 10x qui est magique !!!
    }
    dst->is_freq = true; // a.a we do CN in FD
}
//
//
//
//
//
