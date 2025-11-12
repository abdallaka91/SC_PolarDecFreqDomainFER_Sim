#include "decoders/specialized/decoder_specialized.hpp"
#include "decoders/shared/f_function_proba_in.hpp"
#include "decoders/shared/g_function_proba_in.hpp"
// These headers are not used directly but defines template functions and MUST be included here
#include "middle_node_after_f.hpp"
#include "decoders/specialized/middle_node_after_g.hpp" // IWYU pragma: keep

/**
 *
 * @param n
 * @param frozen_symb
 */
template <int gf_size>
decoder_specialized<gf_size>::decoder_specialized(
    const int n,
    const int* frozen_symb ) : N(n)
{
    internal = new symbols_t[N];
    symbols  = new uint16_t [N];
    frozen   = new uint32_t [N];

    for (int i = 0; i < N; i++) {
        frozen[i] = frozen_symb[i];
    }
}

/**
 *
 */
template <int gf_size>
decoder_specialized<gf_size>::decoder_specialized() : N(0)
{
    internal = nullptr;
    symbols  = nullptr;
    frozen   = nullptr;
    printf("(EE) Error we should never be there...\n");
    printf("(EE) %s %d\n", __FILE__, __LINE__);
    exit(EXIT_FAILURE);
}

template <int gf_size>
decoder_specialized<gf_size>::~decoder_specialized()
{
    delete[] internal;
    delete[] symbols;
    delete[] frozen;
}

template <int gf_size>
void decoder_specialized<gf_size>::execute(symbols_t* channel, uint16_t*  decoded)
{
    const int n = N / 2;
    //
    //
    //
    for (int i = 0; i < n; i++) {
        f_function_proba_in<gf_size>(
            internal + i,
            channel + i,
            channel + n + i);
    }
    //
    //
    //
    middle_node_after_f(
        internal,
        internal + n,
        decoded,
        symbols,
        n,
        0); // On descend à gauche

    for (int i = 0; i < n; i++) {
        g_function_proba_in<gf_size>(internal + i, channel + i, channel + n + i, symbols[i]);
    }

    middle_node_after_g(internal, internal + n, decoded, symbols, n, n);
    //
    // No H computations as we are at the top node and we have a non systematic code !!!
    //
}
//
//
//

#if _GF_ == 8
    template class decoder_specialized< 8>;
#elif _GF_ == 16
template class decoder_specialized< 16>;
#elif _GF_ == 32
    template class decoder_specialized< 32>;
#elif _GF_ == 64
    template class decoder_specialized< 64>;
#elif _GF_ == 128
    template class decoder_specialized<128>;
#elif _GF_ == 256
    template class decoder_specialized<256>;
#elif _GF_ == 512
    template class decoder_specialized<512>;
#elif _GF_ == 1024
    template class decoder_specialized<1024>;
#elif _GF_ == 2048
    template class decoder_specialized<2048>;
#elif _GF_ == 4096
    template class decoder_specialized<4096>;
#endif