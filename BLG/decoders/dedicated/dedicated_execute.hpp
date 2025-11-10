#pragma once

#include "f_function_freq_in.hpp"
#include "f_function_proba_in.hpp"
#include "g_function_freq_in.hpp"
#include "g_function_proba_in.hpp"
#include "decoder_dedicated.hpp"
#include "middle_node_pruned_rep_after_f.hpp"
#include "middle_node_pruned_rep_after_g.hpp"
#include "middle_node_pruned_rate_0.hpp"
#include "middle_node_pruned_rate_1_after_f.hpp"
#include "middle_node_pruned_rate_1_after_g.hpp"
#include "middle_node_pruned_spc_after_f.hpp"
#include "middle_node_pruned_spc_after_g.hpp"

template <int gf_size>
void decoder_dedicated<gf_size>::execute(symbols_t * channel, uint16_t * decoded)
{
// NODE LEVEL (16)
	f_function_proba_in<64>(internal, channel, channel + 8, 8);
// NODE LEVEL (8)
	// f_function_freq_in<64>(....); NO F COMPUTATIONS AS WE HAVE A RATE 0 NODE AFTER !
	middle_node_pruned_rate_0<64>(decoded + 0, symbols + 0, 4);
	g_function_freq_in_after_rate_0<64>(internal + 8, internal + 0, internal + 4, 4);
// NODE LEVEL (4)
	f_function_proba_in<64>(internal + 12, internal + 8, internal + 10, 2);
	middle_node_pruned_rep_after_f<64>(internal + 12, decoded + 4, symbols + 4, 2);
	g_function_proba_in<64>(internal + 12, internal + 8, internal + 10, symbols + 4, 2);
	middle_node_pruned_rate_0<64>(decoded + 4, symbols + 6, 2);
	for(int i = 0; i < 2; i += 1){
	  symbols[4 + i] ^= symbols[6 + i];
	}
	for(int i = 0; i < 4; i += 1){
	  symbols[0 + i] ^= symbols[4 + i];
	}
	g_function_proba_in<64>(internal, channel, channel + 8, symbols, 8);
	middle_node_pruned_rate_1_after_g<64>(internal + 0, decoded + 8, symbols + 8, 8);
}

template <int gf_size>
const int decoder_dedicated<gf_size>::N_gen = 16;

template <int gf_size>
const int decoder_dedicated<gf_size>::K_gen = 9;

