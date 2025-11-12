#include "nb_sc_decoder.hpp"
#include "decoders/shared/f_function_proba_in.hpp"
#include "decoders/shared/g_function_proba_in.hpp"
// These headers are not used directly but defines template functions and MUST be included here
#include "middle_node_after_f.hpp"
#include "middle_node_after_g.hpp" // IWYU pragma: keep

/**
 *
 * @param n
 * @param frozen_symb
 */
template <int gf_size>
nb_sc_decoder<gf_size>::nb_sc_decoder(
    const int n,
    const int* frozen_symb ) : N(n)
{
    internal = new symbols_s<gf_size>[N];
    symbols  = new uint16_t [N];
    frozen   = new uint32_t [N];

    for (int i = 0; i < N; i++) {
        frozen[i] = frozen_symb[i];
    }
}

template <int gf_size>
nb_sc_decoder<gf_size>::~nb_sc_decoder()
{
    delete[] internal;
    delete[] symbols;
    delete[] frozen;
}

template <int gf_size>
void nb_sc_decoder<gf_size>::execute(void* s_channel, uint16_t* decoded)
{
    symbols_s<gf_size>* channel = static_cast< symbols_s<gf_size>* >(s_channel);
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
    template class nb_sc_decoder< 8>;
#elif _GF_ == 16
template class nb_sc_decoder< 16>;
#elif _GF_ == 32
    template class nb_sc_decoder< 32>;
#elif _GF_ == 64
    template class nb_sc_decoder< 64>;
#elif _GF_ == 128
    template class nb_sc_decoder<128>;
#elif _GF_ == 256
    template class nb_sc_decoder<256>;
#elif _GF_ == 512
    template class nb_sc_decoder<512>;
#elif _GF_ == 1024
    template class nb_sc_decoder<1024>;
#endif