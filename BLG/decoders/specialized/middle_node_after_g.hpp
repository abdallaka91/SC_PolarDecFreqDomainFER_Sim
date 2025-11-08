#pragma once

#include "decoder_specialized.hpp"
#include "decoders/shared/f_function_proba_in.hpp"
#include "decoders/shared/g_function_proba_in.hpp"
#include "decoders/shared/leaf_node_after_f.hpp"
#include "decoders/shared/leaf_node_after_g.hpp"

template <int gf_size>
void decoder_specialized<gf_size>::middle_node_after_g(
    symbols_t * inputs,   // Inputs are the symbols from the channel (from the right)
    symbols_t * internal, // Internal nodes are the symbols computed during the process (to the left)
    uint16_t *  decoded,  // Decoded symbols are the final output of the decoder (done on the left)
    uint16_t *  symbols,  // Symbols are the ones going from leafs to root (done on the left)
    int         size,     // Size is the number of symbols (should be a power of 2)
    const int   symbol_id)  // Symbol ID is the index of the FIRST symbol in the symbols array
{
    const int n = size / 2; // Assuming size is the number of symbols

    for (int i = 0; i < n; i++) {
        f_function_proba_in<gf_size>(internal + i, inputs + i, inputs + n + i);
    }

    if (n == 1) {
        leaf_node_after_f<gf_size>(internal, decoded, symbols, symbol_id, frozen[symbol_id]);
    } else {
        middle_node_after_f(internal, internal + n, decoded, symbols, n, symbol_id);
    }

    for (int i = 0; i < n; i++) {
        g_function_proba_in<gf_size>(internal + i, inputs + i, inputs + n + i, symbols[symbol_id + i]);
    }

    if (n == 1) {
        leaf_node_after_g<gf_size>( internal, decoded, symbols, symbol_id + n, frozen[symbol_id + n]);
    } else {
        middle_node_after_g(internal, internal + n, decoded, symbols, n, symbol_id + n);
    }

    for (int i = 0; i < n; i++) {
        symbols[symbol_id + i] ^= symbols[symbol_id + n + i];
    }
}
