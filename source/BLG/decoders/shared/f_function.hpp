#pragma once

#include "custom_types.hpp"
#include "archi.hpp"

// template <uint32_t gf_size>

// void f_function(symbols_t * __restrict dst, symbols_t * __restrict src_a, symbols_t * __restrict src_b) {
//     if (src_a->is_freq == false) // Switch from time to frequency domain
//     {
//         FWHT_NORM<gf_size>(src_a->value);
//         src_a->is_freq = true;
//     }

//     if (src_b->is_freq == false) // Switch from time to frequency domain
//     {
//         FWHT_NORM<gf_size>(src_b->value);
//         src_b->is_freq = true;
//     }

//     //
//     // Element-wise multiplication of the two input symbols because we are in frequency domain !
//     //
//     for (size_t i = 0; i < gf_size; i++) {
//         dst->value[i] =  src_a->value[i] * src_b->value[i]; // TODO : attention au facteur 10x qui est magique !!!
//     }
//     dst->is_freq = true; // a.a we do CN in FD
// }

template <uint32_t gf_size>
void f_function(symbols_t *__restrict dst, symbols_t *__restrict src_a, symbols_t *__restrict src_b)
{
    if (src_a->is_freq == false) // Switch from time to frequency domain
    {
        float maxv = 0;
        for (size_t i = 0; i < gf_size; i++)
        {
            maxv = fmax(maxv, src_a->value[i].to_float());
            maxv = fmax(maxv, src_b->value[i].to_float());
        }
        ap_fixed<NBITS, NINTG> fact = 1.f / maxv;

        for (size_t i = 0; i < gf_size; i++)
        {
            dst->value[i] = fact * src_a->value[i] * src_b->value[i];
        }

        dst->is_freq = true; // a.a we do CN in FD
    }
}

    //
    //
    //
    //
    //
