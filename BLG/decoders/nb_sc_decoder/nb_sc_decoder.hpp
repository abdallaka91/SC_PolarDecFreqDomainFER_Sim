#pragma once

#include "nb_sc_dec.hpp"
#include "definitions/custom_types.hpp"
#include "decoders/decoder.hpp"
#include "decoders/shared/f_function_proba_in.hpp"
#include "decoders/shared/g_function_proba_in.hpp"
#include <vector>

template <int gf_size>
class nb_sc_decoder : public nb_sc_dec{
public:
    nb_sc_decoder();
    nb_sc_decoder(int n, const int* frozen_symb);
    ~nb_sc_decoder();

    int GF() {
        return gf_size;
    }

    void execute(
        void* channel,         // Channel symbols are the input symbols (from the right)
        uint16_t *  decoded);  // Symbols are the ones going from leafs to root (done on the left)

private:
    void middle_node_after_f(
        symbols_s<gf_size> * inputs,   // Inputs are the symbols from the channel (from the right)
        symbols_s<gf_size> * internal, // Internal nodes are the symbols computed during the process (to the left)
        uint16_t *  decoded,  // Decoded symbols are the final output of the decoder (done on the left)
        uint16_t *  symbols,  // Symbols are the ones going from leafs to root (done on the left)
        int         size,     // Size is the number of symbols (should be a power of 2)
        const int   symbol_id); // Symbol ID is the index of the FIRST symbol in the symbols array

    void middle_node_after_g(
        symbols_s<gf_size> * inputs,   // Inputs are the symbols from the channel (from the right)
        symbols_s<gf_size> * internal, // Internal nodes are the symbols computed during the process (to the left)
        uint16_t *  decoded,  // Decoded symbols are the final output of the decoder (done on the left)
        uint16_t *  symbols,  // Symbols are the ones going from leafs to root (done on the left)
        int         size,     // Size is the number of symbols (should be a power of 2)
        const int   symbol_id); // Symbol ID is the index of the FIRST symbol in the symbols array

private:
    symbols_s<gf_size>* internal;
    uint16_t*            symbols;
    uint32_t*             frozen;

    const int N;
};
