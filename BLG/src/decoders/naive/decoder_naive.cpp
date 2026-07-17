#include "decoder_naive.hpp"
#include "f_function.hpp"
#include "g_function.hpp"
#include "utilities/utility_functions.hpp"
//
//
//
//
//
template <int gf_size>
decoder_naive<gf_size>::decoder_naive(const int n, const int * frozen_symb) : N(n) {
    channel  = new symbols_t[N];
    internal = new symbols_t[N];
    symbols  = new uint16_t[N];
    frozen   = new uint32_t[N];

    for (int i = 0; i < N; i++) {
        frozen[i] = frozen_symb[i];
    }
}
//
//
//
//
//
template <int gf_size>
decoder_naive<gf_size>::decoder_naive() : N(0) {
    internal = nullptr;
    symbols  = nullptr;
    frozen   = nullptr;
    printf("(EE) Error we should never be there...\n");
    printf("(EE) %s %d\n", __FILE__, __LINE__);
    exit(EXIT_FAILURE);
}
//
//
//
//
//
template <int gf_size>
decoder_naive<gf_size>::~decoder_naive() {
    delete[] channel;
    delete[] internal;
    delete[] symbols;
    delete[] frozen;
}

template <int gf_size>
void decoder_naive<gf_size>::execute(void * s_channel, uint16_t * decoded, uint16_t * ksymb, float * entrop, float * one_err_prob) {
    symbols_s<gf_size> * i_channel = static_cast<symbols_s<gf_size> *>(s_channel);
    for (int i = 0; i < N; i++) {
        channel[i] = convert_to_symbols_t(i_channel[i].value, gf_size, false);
    }
    const int n = N / 2; // Assuming size is the number of symbols
    //
    for (int i = 0; i < n; i++) {
        f_function<gf_size>(internal + i, channel + i, channel + n + i);
    }
    //
    middle_node(internal, internal + n, decoded, symbols, n, 0, ksymb, entrop, one_err_prob); // On descend à gauche
    //
    for (int i = 0; i < n; i++) {
        g_function<gf_size>(internal + i, channel + i, channel + n + i, symbols[i]);
    }
    //
    middle_node(internal, internal + n, decoded, symbols, n, n, ksymb, entrop, one_err_prob); // On descend à droite
    //
    // No H computations as we are at the top node and we have a non systematic code !!!
    //
}
//
//
//
//
//
template <int gf_size>
void decoder_naive<gf_size>::middle_node(
    symbols_t * inputs,   // Inputs are the symbols from the channel (from the right)
    symbols_t * internal, // Internal nodes are the symbols computed during the process (to the left)
    uint16_t *  decoded,  // Decoded symbols are the final output of the decoder (done on the left)
    uint16_t *  symbols,  // Symbols are the ones going from leafs to root (done on the left)
    int         size,     // Size is the number of symbols (should be a power of 2)
    const int   symbol_id,
    uint16_t *  ksymb,
    float * entrop, float * one_err_prob) // Symbol ID is the index of the FIRST symbol in the symbols array
{
    const int n = size / 2; // Assuming size is the number of symbols
    //
    for (int i = 0; i < n; i++) {
        f_function<gf_size>(internal + i, inputs + i, inputs + n + i);
    }
    //
    if (n == 1) {
        leaf_node(internal, decoded, symbols, symbol_id, ksymb, entrop, one_err_prob);
    } else {
        middle_node(internal, internal + n, decoded, symbols, n, symbol_id, ksymb, entrop, one_err_prob);
    }
    //
    for (int i = 0; i < n; i++) {
        g_function<gf_size>(internal + i, inputs + i, inputs + n + i, symbols[symbol_id + i]);
    }
    //
    if (n == 1) {
        leaf_node(internal, decoded, symbols, symbol_id + n, ksymb, entrop, one_err_prob);
    } else {
        middle_node(internal, internal + n, decoded, symbols, n, symbol_id + n, ksymb, entrop, one_err_prob);
    }
    //
    for (int i = 0; i < n; i++) {
        symbols[symbol_id + i] ^= symbols[symbol_id + n + i];
    }
    //
}
template <int gf_size>
void decoder_naive<gf_size>::leaf_node(
    symbols_t * var,
    uint16_t *  decoded,
    uint16_t *  symbols,
    const int   symbol_id,
    uint16_t *  ksymb,
    float * entrop, float * one_err_prob) {
    //
    // Switch from frequency to time domain if needed
    //

    if (var->is_freq) {
        FWHT<gf_size>(var->value);
        var->is_freq = false;
#if FWHT_COUNTER_ENABLE
        fwht_call_counter += 1;
#endif
    }
    normalize<gf_size>(var->value);
      for (size_t i = 0; i < gf_size; i++) {
        if (var->value[i] <= 1e-12)
            var->value[i] = 1e-12;
    }
    entrop[symbol_id]       = compute_entropy<gf_size>(var->value);
    one_err_prob[symbol_id] = (1 - var->value[ksymb[symbol_id]]);
    const int max_index
        = argmax<gf_size>(var->value);
    decoded[symbol_id] = max_index;
    symbols[symbol_id] = ksymb[symbol_id];
}
//
//
//
//
//
#if (_GF_ == 8) || defined(ALL_GFs)
template class decoder_naive<8>;
#endif
#if (_GF_ == 16) || defined(ALL_GFs)
template class decoder_naive<16>;
#endif
#if (_GF_ == 32) || defined(ALL_GFs)
template class decoder_naive<32>;
#endif
#if (_GF_ == 64) || defined(ALL_GFs)
template class decoder_naive<64>;
#endif
#if (_GF_ == 128) || defined(ALL_GFs)
template class decoder_naive<128>;
#endif
#if (_GF_ == 256) || defined(ALL_GFs)
template class decoder_naive<256>;
#endif
#if (_GF_ == 512) || defined(ALL_GFs)
template class decoder_naive<512>;
#endif
#if (_GF_ == 1024) || defined(ALL_GFs)
template class decoder_naive<1024>;
#endif
#if (_GF_ == 2048) || defined(ALL_GFs)
template class decoder_naive<2048>;
#endif
#if (_GF_ == 4096) || defined(ALL_GFs)
template class decoder_naive<4096>;
#endif
//
//
//
//
//
