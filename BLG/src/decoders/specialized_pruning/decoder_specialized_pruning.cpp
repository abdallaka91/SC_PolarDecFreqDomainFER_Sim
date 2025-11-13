#include "decoder_specialized_pruning.hpp"

/**
 *
 * @param n
 * @param frozen_symb
 */
template <int gf_size>
decoder_specialized_pruning<gf_size>::decoder_specialized_pruning(const int n, const int* frozen_symb)
    : N(n), f_tree_cnt(0), f_tree(nullptr), pruned_tree(_N_)
{
    internal = new symbols_t[N];
    symbols  = new uint16_t [N];
    frozen   = new int      [N];

    for (int i = 0; i < N; i++) {
        frozen[i] = frozen_symb[i];
    }

    pruned_tree.analyze(frozen, _N_);
    pruned_tree.dump();
    f_tree = &pruned_tree;                   // Ici
}

/**
 *
 */
template <int gf_size>
decoder_specialized_pruning<gf_size>::decoder_specialized_pruning() :
    N(0), f_tree_cnt(0), f_tree(nullptr), pruned_tree(_N_)

{
    internal = nullptr;
    symbols  = nullptr;
    frozen   = nullptr;
    printf("(EE) Error we should never be there...\n");
    printf("(EE) %s %d\n", __FILE__, __LINE__);
    exit(EXIT_FAILURE);
}

template <int gf_size>
decoder_specialized_pruning<gf_size>::~decoder_specialized_pruning()
{
    delete[]internal;
    delete[]symbols;
    delete[]frozen;
}

// These headers are not used directly but defines template functions and MUST be included here
#include "middle_node_pruned_after_f.hpp"
#include "middle_node_pruned_rate_1_after_f.hpp"
#include "decoders/shared/middle_node_pruned_rep_after_f.hpp"
#include "decoders/shared/middle_node_pruned_rep_after_g.hpp"    // IWYU pragma: keep
#include "decoders/specialized_pruning/middle_node_pruned_after_g.hpp"        // IWYU pragma: keep
#include "decoders/specialized_pruning/middle_node_pruned_rate_0.hpp"         // IWYU pragma: keep
#include "decoders/specialized_pruning/middle_node_pruned_rate_1_after_g.hpp" // IWYU pragma: keep

#include "decoders/specialized_pruning/middle_node_pruned_spc_after_f.hpp" // IWYU pragma: keep
#include "decoders/specialized_pruning/middle_node_pruned_spc_after_g.hpp" // IWYU pragma: keep

template <int gf_size>
void decoder_specialized_pruning<gf_size>::execute(symbols_t * channel, uint16_t * decoded)
{
    f_tree_cnt = 0;

    const int n = N / 2; // Assuming size is the number of symbols
    //
    //
    //
    for (int i = 0; i < n; i++) {
        f_function_proba_in<gf_size>(internal + i, channel + i, channel + n + i);
    }
    //
    //
    //
    next_node left_edge = f_tree->next_node_status[f_tree_cnt++];
    if (left_edge == RATE_0) {
       middle_node_pruned_rate_0( internal, internal + n, decoded, symbols, n, 0); // On descend à gauche
    }else if (left_edge == REP_FROM_F) {
        middle_node_pruned_rep_after_f<gf_size>( internal, internal + n, decoded, symbols, n, 0); // On descend à gauche
    }else if (left_edge == SPC_FROM_F) {
        middle_node_pruned_spc_after_f/*<gf_size>*/( internal, decoded, symbols, n, 0); // On descend à gauche
    }else if (left_edge == MID_NODE_FROM_F) {
       middle_node_pruned_after_f( internal, internal + n, decoded, symbols, n, 0); // On descend à gauche
    }else {
        printf("(EE) Error we should never be there...\n");
        printf("(EE) %s %d\n", __FILE__, __LINE__);
        exit(EXIT_FAILURE);
    }
    //
    //
    //
    for (int i = 0; i < n; i++) {
        g_function_proba_in<gf_size>(
            internal + i,    // memory space for the result
            channel + i,     // values from the right child
            channel + n + i, // values from the right child
            symbols[i]);     // decoded symbols from the left child
    }
    //
    //
    //
    next_node right_edge = f_tree->next_node_status[f_tree_cnt++];
    if ( right_edge == RATE_1_FROM_G ) {
        middle_node_pruned_rate_1_after_g( internal, internal + n, decoded, symbols, n, n); // On descend à droite
    }else if ( right_edge == SPC_FROM_G ){
        middle_node_pruned_spc_after_g( internal, decoded, symbols, n, n); // On descend à droite
    }else if ( right_edge == REP_FROM_G ){
        middle_node_pruned_rep_after_g<gf_size>( internal, /*internal + n,*/ decoded, symbols, n, n); // On descend à droite
    }else if ( right_edge == MID_NODE_FROM_G ){
        middle_node_pruned_after_g( internal, internal + n, decoded, symbols, n, n); // On descend à droite
    }else{
        printf("(EE) Error we should never be there...\n");
        printf("(EE) %s %d\n", __FILE__, __LINE__);
        exit(EXIT_FAILURE);
    }

    // No H computations as we are at the top node and we have a non systematic code !!!
}
//
//
//
#if _GF_ == 8
    template class decoder_specialized_pruning< 8>;
#elif _GF_ == 16
    template class decoder_specialized_pruning< 16>;
#elif _GF_ == 32
    template class decoder_specialized_pruning< 32>;
#elif _GF_ == 64
    template class decoder_specialized_pruning< 64>;
#elif _GF_ == 128
    template class decoder_specialized_pruning<128>;
#elif _GF_ == 256
    template class decoder_specialized_pruning<256>;
#elif _GF_ == 512
    template class decoder_specialized_pruning<512>;
#elif _GF_ == 1024
    template class decoder_specialized_pruning<1024>;
#elif _GF_ == 2048
    template class decoder_specialized_pruning<2048>;
#elif _GF_ == 4096
    template class decoder_specialized_pruning<4096>;
#endif