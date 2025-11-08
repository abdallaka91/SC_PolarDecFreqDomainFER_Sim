#pragma once

#include "decoder_specialized_pruning.hpp"
#include "utilities/utility_functions.hpp"
#include "decoders/dedicated/fix_xor_list.hpp"

extern void local_remove_xors(uint16_t * values, int size);
extern void       remove_xors(uint16_t * values, int size);

template <int gf_size>
void decoder_specialized_pruning<gf_size>::middle_node_pruned_spc_after_f(
    symbols_t * __restrict inputs,  // Inputs are the symbols from the channel (from the right)
    uint16_t * __restrict decoded,  // Decoded symbols are the final output of the decoder (done on the left)
    uint16_t * __restrict symbols,  // Symbols are the ones going from leafs to root (done on the left)
    int        size,     // Size is the number of symbols (should be a power of 2)
    const int  symbol_id) // Symbol ID is the index of the FIRST symbol in the symbols array
{
    //
    for(int i = 0; i < size; i++) {
        FWHT_NORM<gf_size>(inputs[i].value);
        inputs[i].is_freq = false;
    }
    //
    for(int i = 0; i < size; i++)
    {
        if ( inputs[i].is_freq == true ) {
            fwht<gf_size>( inputs[i].value );
            inputs[i].is_freq = false;
            normalize<gf_size>(inputs[i].value);
        }
    }
    //
    int check_node = 0;
    uint16_t arg_1[256];
    for (int i = 0; i < size; i++) {
        int value              = argmax<gf_size>(inputs[i].value);
        check_node            ^= value;
        symbols[symbol_id + i] = value;
        decoded[symbol_id + i] = value; // should be corrected (it is systematic solution actually)
        arg_1  [symbol_id + i] = value;
    }
    //
    if ( check_node == 0 ) {
        remove_xors(decoded + symbol_id, size);
        return;
    }
    //
    uint16_t arg_2[256];
    for (int j = 0; j < size; j++) {
        arg_2[j] = argmax2<gf_size>(inputs[j].value, arg_1[j]);
    }
    //
    fix_xor_list(arg_1, arg_2, inputs, size);
    //
    for (int j = 0; j < size; j++) {
        symbols[symbol_id + j] = arg_1[j];
        decoded[symbol_id + j] = arg_1[j]; // should be corrected (it is systematic solution actually)
    }
    //
    remove_xors(decoded + symbol_id, size);
    //
}

