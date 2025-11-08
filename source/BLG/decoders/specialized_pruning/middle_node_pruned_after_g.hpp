#pragma once

#include "decoder_specialized_pruning.hpp"
#include "decoders/shared/f_function_proba_in.hpp"
#include "decoders/shared/g_function_proba_in.hpp"
#include "decoders/shared/leaf_node_rate_0.hpp"
#include "decoders/shared/middle_node_pruned_rep_after_f.hpp"
#include "decoders/shared/middle_node_pruned_rep_after_g.hpp" // IWYU pragma: keep

template <int gf_size>
void decoder_specialized_pruning<gf_size>::middle_node_pruned_after_g(
    symbols_t *__restrict inputs,   // Inputs are the symbols from the channel (from the right)
    symbols_t *__restrict internal, // Internal nodes are the symbols computed during the process (to the left)
    uint16_t *__restrict decoded,   // Decoded symbols are the final output of the decoder (done on the left)
    uint16_t *__restrict symbols,   // Symbols are the ones going from leafs to root (done on the left)
    int size,                       // Size is the number of symbols (should be a power of 2)
    const int symbol_id)            // Symbol ID is the index of the FIRST symbol in the symbols array
{
    const int n = size / 2; // Assuming size is the number of symbols
                            //    printf("-> middle_node_pruned_after_g (size = %d, f_tree_cnt =%d) - left  = %s\n", size, f_tree_cnt, s_next_node[f_tree->next_node_status[f_tree_cnt]].c_str());
    //
    //
    //
    for (int i = 0; i < n; i++)
    {
        f_function_proba_in<gf_size>(internal + i, inputs + i, inputs + n + i); // Example operation
    }
    //
    //
    //
    next_node left_edge = f_tree->next_node_status[f_tree_cnt++];
    if (left_edge == RATE_0)
    {
        middle_node_pruned_rate_0(internal, internal + n, decoded, symbols, n, symbol_id);
    }
    else if (left_edge == RATE_1_FROM_F)
    {
        middle_node_pruned_rate_1_after_f(internal, internal + n, decoded, symbols, n, symbol_id);
    }
    else if (left_edge == LEAF_RATE_0)
    {
        leaf_node_rate_0<gf_size>(decoded, symbols, symbol_id);
    }
    else if (left_edge == LEAF_RATE_1_FROM_F)
    {
        leaf_node_after_f<gf_size>(internal, decoded, symbols, symbol_id, frozen[symbol_id]);
    }
    else if (left_edge == REP_FROM_F)
    {
        middle_node_pruned_rep_after_f<gf_size>(internal, internal + n, decoded, symbols, n, symbol_id);
    }
    else if (left_edge == SPC_FROM_F)
    {
        middle_node_pruned_spc_after_f(internal, decoded, symbols, n, symbol_id);
    }
    else if (left_edge == MID_NODE_FROM_F)
    {
        middle_node_pruned_after_f(internal, internal + n, decoded, symbols, n, symbol_id); // l'identifiant du symbole (à gauche)
    }
    else if (left_edge == RATE_1_FROM_G)
    {
        printf("(EE) We should never be there (%s, %d)\n", __FILE__, __LINE__);
        exit(EXIT_FAILURE);
    }
    else if (left_edge == LEAF_RATE_1_FROM_G)
    {
        printf("(EE) We should never be there (%s, %d)\n", __FILE__, __LINE__);
        exit(EXIT_FAILURE);
    }
    else if (left_edge == MID_NODE_FROM_G)
    {
        printf("(EE) We should never be there (%s, %d)\n", __FILE__, __LINE__);
        exit(EXIT_FAILURE);
    }
    else
    {
        printf("(EE) We should never be there (%s, %d)\n", __FILE__, __LINE__);
        exit(EXIT_FAILURE);
    }
    //
    //
    //
    for (int i = 0; i < n; i++)
    {
        g_function_proba_in<gf_size>(
            internal + i,
            inputs + i,
            inputs + n + i,
            symbols[symbol_id + i]); // Example operation
    }
    //
    //
    //
    //  printf("-> middle_node_pruned_after_g (size = %d, f_tree_cnt = %d) - right = %s\n", size, f_tree_cnt, s_next_node[f_tree->next_node_status[f_tree_cnt]].c_str());
    next_node right_edge = f_tree->next_node_status[f_tree_cnt++];
    if (right_edge == RATE_0)
    {
        middle_node_pruned_rate_0(internal, internal + n, decoded, symbols, n, symbol_id + n);
    }
    else if (right_edge == RATE_1_FROM_G)
    {
        middle_node_pruned_rate_1_after_g(internal, internal + n, decoded, symbols, n, symbol_id + n);
    }
    else if (right_edge == LEAF_RATE_0)
    {
        leaf_node_rate_0<gf_size>(decoded, symbols, symbol_id + n);
    }
    else if (right_edge == LEAF_RATE_1_FROM_G)
    {
        leaf_node_after_g<gf_size>(internal, decoded, symbols, symbol_id + n, frozen[symbol_id + n]);
    }
    else if (right_edge == REP_FROM_G)
    {
        middle_node_pruned_rep_after_g<gf_size>(internal, decoded, symbols, n, symbol_id + n);
    }
    else if (right_edge == SPC_FROM_G)
    {
        middle_node_pruned_spc_after_g(internal, decoded, symbols, n, symbol_id + n);
    }
    else if (right_edge == MID_NODE_FROM_G)
    {
        middle_node_pruned_after_g(internal, internal + n, decoded, symbols, n, symbol_id + n);
    }
    else if (right_edge == LEAF_RATE_1_FROM_F)
    {
        printf("(EE) We should never be there (%s, %d)\n", __FILE__, __LINE__);
        exit(EXIT_FAILURE);
    }
    else if (right_edge == MID_NODE_FROM_F)
    {
        printf("(EE) We should never be there (%s, %d)\n", __FILE__, __LINE__);
        exit(EXIT_FAILURE);
    }
    else if (right_edge == RATE_1_FROM_F)
    {
        exit(EXIT_FAILURE);
    }
    else
    {
        printf("(EE) We should never be there (%s, %d)\n", __FILE__, __LINE__);
        exit(EXIT_FAILURE);
    }
    //
    //
    //
    for (int i = 0; i < n; i++)
    {
        symbols[symbol_id + i] ^= symbols[symbol_id + n + i];
    }
    //
    //
    //
}
//
//
//
//
//
