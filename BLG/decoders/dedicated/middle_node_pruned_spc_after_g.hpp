#pragma once
//
//
//
//
//
#include "features/archi.hpp"
#include "definitions/custom_types.hpp"
#include "fix_xor_list.hpp"
//
//
//
//
//
extern void local_remove_xors(uint16_t * values, int size);
extern void       remove_xors(uint16_t * values, int size);
//
//
//
//
//
template <int gf_size> void middle_node_pruned_spc_after_g(
    symbols_t* __restrict inputs,  // Inputs are the symbols from the channel (from the right)
    uint16_t*  __restrict decoded, // Decoded symbols are the final output of the decoder (done on the left)
    uint16_t*  __restrict symbols, // Symbols are the ones going from leafs to root (done on the left)
    const int size                 // Size is the number of symbols (should be a power of 2)
) {
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
    uint16_t arg_1[512];
    for (int i = 0; i < size; i++) {
        int value              = argmax<gf_size>(inputs[i].value);
        check_node            ^= value;
        symbols[i] = value;
        decoded[i] = value; // should be corrected (it is systematic solution actually)
        arg_1  [i] = value;
    }
    //
    if ( check_node == 0 ) {
        remove_xors(decoded, size);
        return;
    }
    //
    uint16_t arg_2[512];
    for (int j = 0; j < size; j++) {
        arg_2[j] = argmax2<gf_size>(inputs[j].value, arg_1[j]);
    }
    //
    fix_xor_list(arg_1, arg_2, inputs, size);
    //
    for (int j = 0; j < size; j++) {
        symbols[j] = arg_1[j];
        decoded[j] = arg_1[j]; // should be corrected (it is systematic solution actually)
    }
    //
    remove_xors(decoded, size);
    //
}
//
//
//
//
//
