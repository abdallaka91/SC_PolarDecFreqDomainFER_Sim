#include "decoder_naive_fixed.hpp"
#include "f_argmax.hpp"
#include "f_function.hpp"
#include "f_fwht.hpp"
#include "f_fwht_norm.hpp"
#include "f_normalize.hpp"
#include "g_function.hpp"
//
//
//
//
//
template <int gf_size>
symbols_f conversion(const symbols_t s) {
    symbols_f f;
    for (int i = 0; i < gf_size; i++) {
        f.value[i] = s.value[i];
    }
    f.is_freq = s.is_freq;
    return f;
}
//
//
//
//
//
template <int gf_size>
decoder_naive_fixed<gf_size>::decoder_naive_fixed(const int n, const int * frozen_symb) : N(n) {
    symbols = new uint16_t[N];
    frozen  = new uint32_t[N];

    f_channel  = new symbols_f[N];
    f_internal = new symbols_f[N];

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
decoder_naive_fixed<gf_size>::decoder_naive_fixed() : N(0) {
    f_internal = nullptr;
    symbols    = nullptr;
    frozen     = nullptr;
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
decoder_naive_fixed<gf_size>::~decoder_naive_fixed() {
    delete[] symbols;
    delete[] frozen;

    delete[] f_channel;
    delete[] f_internal;
}

template <int gf_size>
void decoder_naive_fixed<gf_size>::execute(symbols_t * channel, uint16_t * decoded) {
    const int n = N / 2; // Assuming size is the number of symbols
    //
    //
    //
    for (int i = 0; i < N; i++) {
        f_channel[i] = conversion<gf_size>(channel[i]);
    }
    //
    //
    //
    for (int i = 0; i < n; i++) {
        f_function<gf_size>(f_internal + i, f_channel + i, f_channel + n + i);
    }
    //
    //
    //
    middle_node(f_internal, f_internal + n, decoded, symbols, n, 0); // On descend à gauche
    //
    //
    //
    for (int i = 0; i < n; i++) {
        g_function<gf_size>(f_internal + i, f_channel + i, f_channel + n + i, symbols[i]);
    }
    //
    //
    //
    middle_node(f_internal, f_internal + n, decoded, symbols, n, n); // On descend à droite
    //
    //
    //
    // No H computations as we are at the top node and we have a non systematic code !!!
    //
    //
    //
}
//
//
//
//
//
template <int gf_size>
void decoder_naive_fixed<gf_size>::middle_node(
    symbols_f * inputs,   // Inputs are the symbols from the channel (from the right)
    symbols_f * internal, // Internal nodes are the symbols computed during the process (to the left)
    uint16_t *  decoded,  // Decoded symbols are the final output of the decoder (done on the left)
    uint16_t *  symbols,  // Symbols are the ones going from leafs to root (done on the left)
    int         size,     // Size is the number of symbols (should be a power of 2)
    const int   symbol_id)  // Symbol ID is the index of the FIRST symbol in the symbols array
{
    const int n = size / 2; // Assuming size is the number of symbols
    //
    //
    //
    for (int i = 0; i < n; i++) {
        f_function<gf_size>(internal + i, inputs + i, inputs + n + i);
    }
    //
    //
    //
    if (n == 1) {
        leaf_node(internal, decoded, symbols, symbol_id);
    } else {
        middle_node(internal, internal + n, decoded, symbols, n, symbol_id);
    }
    //
    //
    //
    for (int i = 0; i < n; i++) {
        g_function<gf_size>(internal + i, inputs + i, inputs + n + i, symbols[symbol_id + i]);
    }
    //
    //
    //
    if (n == 1) {
        leaf_node(internal, decoded, symbols, symbol_id + n);
    } else {
        middle_node(internal, internal + n, decoded, symbols, n, symbol_id + n);
    }
    //
    //
    //
    for (int i = 0; i < n; i++) {
        symbols[symbol_id + i] ^= symbols[symbol_id + n + i];
    }
    //
    //
    //
}
template <int gf_size>
void decoder_naive_fixed<gf_size>::leaf_node(
    symbols_f * var,
    uint16_t *  decoded,
    uint16_t *  symbols,
    const int   symbol_id) {
    //
    // Switch from frequency to time domain if needed
    //
    if (frozen[symbol_id] == true) {
        decoded[symbol_id] = 0;
        symbols[symbol_id] = 0;
        return;
    }

    if (var->is_freq) {
        const float factor = norm_factor_lwht<gf_size>();
        f_normalize<gf_size>(var->value, factor);
        f_normalize<gf_size>(var->value, factor);
        fwht<gf_size>(var->value);
        var->is_freq = false;
    }

    const int max_index = f_argmax<gf_size>(var->value);
    decoded[symbol_id]  = max_index;
    symbols[symbol_id]  = max_index;
}
//
//
//
//
//
#if _GF_ == 8
template class decoder_naive_fixed<8>;
#elif _GF_ == 16
template class decoder_naive_fixed<16>;
#elif _GF_ == 32
template class decoder_naive_fixed<32>;
#elif _GF_ == 64
template class decoder_naive_fixed<64>;
#elif _GF_ == 128
template class decoder_naive_fixed<128>;
#elif _GF_ == 256
template class decoder_naive_fixed<256>;
#elif _GF_ == 512
template class decoder_naive_fixed<512>;
#elif _GF_ == 1024
template class decoder_naive_fixed<1024>;
#elif _GF_ == 2048
template class decoder_naive_fixed<2048>;
#elif _GF_ == 4096
template class decoder_naive_fixed<4096>;
#endif
//
//
//
//
//
