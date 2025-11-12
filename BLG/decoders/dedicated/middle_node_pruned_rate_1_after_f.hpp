#pragma once
//
//
//
//
//
#include "decoders/features/archi.hpp"
#include "definitions/custom_types.hpp"
//
//
//
//
//
template <int gf_size>
void middle_node_pruned_rate_1_after_f(
    symbols_t *__restrict inputs, // Inputs are the symbols from the channel (from the right)
    uint16_t *__restrict decoded, // Decoded symbols are the final output of the decoder (done on the left)
    uint16_t *__restrict symbols, // Symbols are the ones going from leafs to root (done on the left)
    int size                      // Size is the number of symbols (should be a power of 2)
)
{
    for (int i = 0; i < size; i++)
    {
        FWHT_NORM<gf_size>(inputs[i].value);
        const int value = argmax<gf_size>(inputs[i].value);
        symbols[i] = value;
        decoded[i] = value;
    }
    local_remove_xors(decoded, size);
}
