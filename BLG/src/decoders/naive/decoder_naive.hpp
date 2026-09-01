#pragma once
#include "decoders/decoder.hpp"
#include "definitions/custom_types.hpp"
#include "features/fwht/fwht_counter.hpp"
//
//
//
//
//
template <int gf_size>
class decoder_naive : public decoder {
public:
    decoder_naive();
    decoder_naive(const int n, const int * frozen_symb);
    ~decoder_naive();

    virtual void execute(void * channel, uint16_t * decoded, uint16_t * ksymb, float * entrop, float * one_err_prob);
    virtual int  GF() { return gf_size; }

private:
    void middle_node(
        symbols_t * inputs,   // Inputs are the symbols from the channel (from the right)
        symbols_t * internal, // Internal nodes are the symbols computed during the process (to the left)
        uint16_t *  decoded,  // Decoded symbols are the final output of the decoder (done on the left)
        uint16_t *  symbols,  // Symbols are the ones going from leafs to root (done on the left)
        int         size,     // Size is the number of symbols (should be a power of 2)
        const int   symbol_id,
        uint16_t *  ksymb,
        float * entrop, float * one_err_prob); // Symbol ID is the index of the FIRST symbol in the symbols array

    void leaf_node(
        symbols_t * var,
        uint16_t *  decoded,
        uint16_t *  symbols,
        const int   symbol_id,
        uint16_t *  ksymb,
        float * entrop, float * one_err_prob);

    symbols_t * channel;
    symbols_t * internal;
    uint16_t *  symbols;
    uint32_t *  frozen;

    const int N;
};
//
//
//
//
//
