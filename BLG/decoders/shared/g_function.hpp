#pragma once

#include "definitions/code.hpp"
#include "features/archi.hpp"
#include "utilities/utility_functions.hpp"

#ifdef ABD_OPTIM
#if _GF_ == 8
#include "hadamard/Hadamard_8.hpp"
#elif _GF_ == 16
#include "hadamard/Hadamard_16.hpp"
#elif _GF_ == 32
#include "hadamard/Hadamard_32.hpp"
#elif _GF_ == 64
#include "hadamard/Hadamard_64.hpp"
#elif _GF_ == 128
#include "hadamard/Hadamard_128.hpp"
#elif _GF_ == 256
#include "hadamard/Hadamard_256.hpp"
#elif _GF_ == 512
#include "hadamard/Hadamard_512.hpp"
#elif _GF_ == 1024
#include "hadamard/Hadamard_1024.hpp"
#elif _GF_ == 2048
#include "hadamard/Hadamard_2048.hpp"
#elif _GF_ == 4096
#include "hadamard/Hadamard_4096.hpp"
#endif
#endif

template <int gf_size>
void g_function(
    symbols_t * __restrict dst,   // the data to be computed for the left side of the graph
    symbols_t * __restrict src_a, // the upper value set from the right side of the graph
    symbols_t * __restrict src_b, // the lower value set from the right side of the graph
    uint32_t src_c)               // the computed symbols coming from the left side of the graph
{
#ifndef ABD_OPTIM
    if (src_a->is_freq == true)
    {
        FWHT_NORM<gf_size>(src_a->value);
        src_a->is_freq = false;
    }

    if (src_b->is_freq == true)
    {
        FWHT_NORM<gf_size>(src_b->value);
        src_b->is_freq = false;
    }

    for (size_t i = 0; i < gf_size; i++)
    {
        const int idx   = src_c ^ i;
        dst->value[idx] = src_a->value[i] * src_b->value[idx];
    }

    normalize<gf_size>(dst->value); // temporal
    dst->is_freq = false;
#else
    if (src_a->is_freq == true)
    {
        for (size_t i = 0; i < gf_size; i++)
        {
            dst->value[i] = src_a->value[i] * Hadamard[src_c][i];
        }

        FWHT_NORM<gf_size>(dst->value);
        dst->is_freq = false;
    }
    else
    {
        for (size_t i = 0; i < gf_size; i++)
        {
            const int idx   = src_c ^ i;
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
#endif
}