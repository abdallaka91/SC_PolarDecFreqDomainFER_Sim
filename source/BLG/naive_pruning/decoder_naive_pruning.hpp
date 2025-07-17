#pragma once
#include "../custom_types.hpp"
#include "../decoder.hpp"
//
//
//
//
//
template <int gf_size>
class decoder_naive_pruning : public decoder {
public:
    decoder_naive_pruning();
    decoder_naive_pruning(const int n, const int* frozen_symb);
    ~decoder_naive_pruning();

    void execute(
        symbols_t * channel,   // Channel symbols are the input symbols (from the right)
        uint16_t *  decoded);  // Symbols are the ones going from leafs to root (done on the left)

    void middle_node_with_pruning(
        symbols_t * inputs,   // Inputs are the symbols from the channel (from the right)
        symbols_t * internal, // Internal nodes are the symbols computed during the process (to the left)
        uint16_t *  decoded,  // Decoded symbols are the final output of the decoder (done on the left)
        uint16_t *  symbols,  // Symbols are the ones going from leafs to root (done on the left)
        int         size,     // Size is the number of symbols (should be a power of 2)
        const int   symbol_id);  // Symbol ID is the index of the FIRST symbol in the symbols array

    void middle_node_rate1(
        symbols_t * inputs,   // Inputs are the symbols from the channel (from the right)
        uint16_t *  decoded,  // Decoded symbols are the final output of the decoder (done on the left)
        uint16_t *  symbols,  // Symbols are the ones going from leafs to root (done on the left)
        int         size,     // Size is the number of symbols (should be a power of 2)
        const int   symbol_id);  // Symbol ID is the index of the FIRST symbol in the symbols array

    static void middle_node_rate0(
        uint16_t *  symbols,  // Symbols are the ones going from leafs to root (done on the left)
        int         size,     // Size is the number of symbols (should be a power of 2)
        const int   symbol_id);  // Symbol ID is the index of the FIRST symbol in the symbols array

    void leaf_node(
        symbols_t * var,
        uint16_t *  decoded,
        uint16_t *  symbols,
        const int   symbol_id) const;

private:
    symbols_t* internal;
    uint16_t*  symbols;
    uint32_t*  frozen;

    const int N;
};
//
//
//
//
//
