// #pragma once
// //
// //
// //
// //
// //
// #include "decoders/features/archi.hpp"
// //
// //
// //
// //
// //
// // #define debug_g_function
// template <int gf_size>
// void g_function(
//     symbols_t * __restrict dst,   // the data to be computed for the left side of the graph
//     symbols_t * __restrict src_a, // the upper value set from the right side of the graph
//     symbols_t * __restrict src_b, // the lower value set from the right side of the graph
//     uint32_t    src_c)    // the computed symbols coming from the left side of the graph
// {
//     if (src_a->is_freq == true) {
//         FWHT_NORM<gf_size>(src_a->value);
//         src_a->is_freq = false;
//     }

//     if (src_b->is_freq == true) {
//         FWHT_NORM<gf_size>(src_b->value);
//         src_b->is_freq = false;
//     }

//     for (size_t i = 0; i < gf_size; i++) {
//         const int idx   = src_c ^ i;
//         dst->value[idx] = src_a->value[i] * src_b->value[idx];
//     }

//     normalize<gf_size>(dst->value); // temporal
//     dst->is_freq = false;
// }
// //
// //
// //
// //
// //

#pragma once

#include "decoders/features/archi.hpp"
#include "utilities/utility_functions.hpp"
#include "hadamard/hadamard_64.hpp"

template <int gf_size>
void g_function(
    symbols_t *__restrict dst,   // the data to be computed for the left side of the graph
    symbols_t *__restrict src_a, // the upper value set from the right side of the graph
    symbols_t *__restrict src_b, // the lower value set from the right side of the graph
    uint32_t src_c)              // the computed symbols coming from the left side of the graph
{
    if (src_a->is_freq == true)
    {
        for (size_t i = 0; i < gf_size; i++)
        {
            dst->value[i] = src_a->value[i] * Hadamard[src_c][i];
        }

        // FWHT_NORM<gf_size>(src_a->value);
        FWHT_NORM<gf_size>(dst->value);
        // src_a->is_freq = false;
        dst->is_freq = false;
    }
    else
    {
        for (size_t i = 0; i < gf_size; i++)
        {
            const int idx = src_c ^ i;
            dst->value[idx] = src_a->value[i];
        }
        dst->is_freq = false;
    }

    if (src_b->is_freq == true)
    {
        FWHT_NORM<gf_size>(src_b->value);
        src_b->is_freq = false;
    }

    for (size_t i = 0; i < gf_size; i++)
    {
        dst->value[i] = dst->value[i] * src_b->value[i];
    }

    normalize<gf_size>(dst->value); // temporal
    dst->is_freq = false;
}