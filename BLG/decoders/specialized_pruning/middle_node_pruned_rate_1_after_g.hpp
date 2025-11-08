#pragma once

#include "decoder_specialized_pruning.hpp"
#include "utilities/utility_functions.hpp"

template <int gf_size>
void decoder_specialized_pruning<gf_size>::middle_node_pruned_rate_1_after_g(
    symbols_t* __restrict inputs,  // Inputs are the symbols from the channel (from the right)
    symbols_t *,         // Internal nodes are the symbols computed during the process (to the left)
    uint16_t*  __restrict decoded,  // Decoded symbols are the final output of the decoder (done on the left)
    uint16_t*  __restrict symbols,  // Symbols are the ones going from leafs to root (done on the left)
    int        size,     // Size is the number of symbols (should be a power of 2)
    const int  symbol_id) // Symbol ID is the index of the FIRST symbol in the symbols array
{
    for (int i = 0; i < size; i++) {
        const int value        = argmax<gf_size>(inputs[i].value);
        symbols[symbol_id + i] = value;
        decoded[symbol_id + i] = value;
    }
    local_remove_xors(decoded + symbol_id, size);
}

