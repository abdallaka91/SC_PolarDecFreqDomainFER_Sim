#pragma once

#include "utility_functions.hpp"

template <int gf_size>
void middle_node_pruned_rep_after_f(
    symbols_t * __restrict inputs,  // Inputs are the symbols from the channel (from the right)
    symbols_t *,         // Internal nodes are the symbols computed during the process (to the left)
    uint16_t * __restrict decoded,  // Decoded symbols are the final output of the decoder (done on the left)
    uint16_t * __restrict symbols,  // Symbols are the ones going from leafs to root (done on the left)
    int        size,     // Size is the number of symbols (should be a power of 2)
    const int  symbol_id // Symbol ID is the index of the FIRST symbol in the symbols array
) {
        for(int i = 0; i < size; i++) {
            FWHT_NORM<gf_size>(inputs[i].value);
            inputs[i].is_freq = false;
        }

        float temp[gf_size];
        for (int j = 0; j < gf_size; j++)
            temp[j] = inputs[0].value[j] * inputs[1].value[j];

        for(int i = 2; i < size; i++){
            if( (i & 0x1) == 1)
            normalize<gf_size>( temp );
            for (int j = 0; j < gf_size; j++)
                temp[j] *= inputs[i].value[j] ;
        }

        const int value = argmax<gf_size>( temp );

        for(int i = 0; i < size; i++)
        {
            symbols[symbol_id + i] = value;
            decoded[symbol_id + i] = 0; // should be corrected (it is systematic solution actually)
        }
        decoded[symbol_id + (size-1)] = value; // should be corrected (it is systematic solution actually)
}

