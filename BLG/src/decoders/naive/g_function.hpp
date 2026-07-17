#pragma once

#include "definitions/code.hpp"
#include "features/archi.hpp"
#include "utilities/utility_functions.hpp"

#include "hadamard/Hadamard.hpp"
//
//
//
//
//
template <int gf_size>
void g_function(
    symbols_t * __restrict dst,   // the data to be computed for the left side of the graph
    symbols_t * __restrict src_a, // the upper value set from the right side of the graph
    symbols_t * __restrict src_b, // the lower value set from the right side of the graph
    uint32_t src_c)               // the computed symbols coming from the left side of the graph
{
    if (src_a->is_freq == true) {
        const float * H = get_Hadamard_line<gf_size>(src_c);
        for (size_t i = 0; i < gf_size; i++) {
            dst->value[i] = src_a->value[i] * H[i];
        }

        FWHT<gf_size>(dst->value);

#if FWHT_COUNTER_ENABLE
        fwht_call_counter += 1;
#endif
        dst->is_freq = false;
    } else {
        for (size_t i = 0; i < gf_size; i++) {
            const int idx   = src_c ^ i;
            dst->value[idx] = src_a->value[i];
        }
        dst->is_freq = false;
    }

    for (size_t i = 0; i < gf_size; i++) {
        if (dst->value[i] <= 1e-12)
            dst->value[i] = 1e-12;
    }

    if (src_b->is_freq == true) {
        FWHT<gf_size>(src_b->value);
#if FWHT_COUNTER_ENABLE
        fwht_call_counter += 1;
#endif
        src_b->is_freq = false;
    }

    for (size_t i = 0; i < gf_size; i++) {
        if (src_b->value[i] <= 1e-12)
            src_b->value[i] = 1e-12;
    }

    for (size_t i = 0; i < gf_size; i++) {
        dst->value[i] = dst->value[i] * src_b->value[i];
    }

    normalize<gf_size>(dst->value); // temporal

    for (size_t i = 0; i < gf_size; i++) {
        if (dst->value[i] <= 1e-12)
            dst->value[i] = 1e-12;
    }
    dst->is_freq = false;
}
//
//
//
//
//