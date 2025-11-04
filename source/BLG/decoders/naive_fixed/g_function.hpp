#pragma once

#include "f_argmax.hpp"
#include "f_normalize.hpp"

template <int gf_size>
void g_function(
    symbols_f * __restrict dst,   // the data to be computed for the left side of the graph
    symbols_f * __restrict src_a, // the upper value set from the right side of the graph
    symbols_f * __restrict src_b, // the lower value set from the right side of the graph
    uint32_t    src_c)    // the computed symbols coming from the left side of the graph
{
    if (src_a->is_freq == true) {
        fwht_norm<gf_size>(src_a->value);
        src_a->is_freq = false;
    }

    if (src_b->is_freq == true) {
        fwht_norm<gf_size>(src_b->value);
        src_b->is_freq = false;
    }

    for (size_t i = 0; i < gf_size; i++) {
        const int idx   = src_c ^ i;
        dst->value[idx] = src_a->value[i] * src_b->value[idx];
    }

    f_normalize<gf_size>(dst->value); // temporal
    dst->is_freq = false;
}
//
//
//
//
//
