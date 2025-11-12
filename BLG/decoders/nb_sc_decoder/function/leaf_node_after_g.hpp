#pragma once
//
//
//
//
//
#include "utilities/utility_functions.hpp"
#include "decoders/features/archi.hpp"
//
//
//
//
//
template <int gf_size>
inline __attribute__((always_inline)) void leaf_node_after_g(
    symbols_s<gf_size> *__restrict var,
    uint16_t *__restrict decoded,
    uint16_t *__restrict symbols,
    const int symbol_id,
    const bool frozen)
{

    if (frozen == true)
    {
        decoded[symbol_id] = 0;
        symbols[symbol_id] = 0;
        return;
    }

    const int max_index = argmax<gf_size>(var->value);
    decoded[symbol_id] = max_index;
    symbols[symbol_id] = max_index;
}
//
//
//
//
//
