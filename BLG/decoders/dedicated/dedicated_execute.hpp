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
void decoder_dedicated<gf_size>::execute(symbols_t *channel, uint16_t *decoded)
{
	// NODE LEVEL (1024)
	f_function_proba_in<64>(internal, channel, channel + 512, 512);
	// NODE LEVEL (512)
	f_function_freq_in<64>(internal + 512, internal + 0, internal + 256, 256);
	// NODE LEVEL (256)
	// f_function_freq_in<64>(....); NO F COMPUTATIONS AS WE HAVE A RATE 0 NODE AFTER !
	middle_node_pruned_rate_0<64>(decoded + 0, symbols + 0, 128);
	g_function_freq_in_after_rate_0<64>(internal + 768, internal + 512, internal + 640, 128);
	// NODE LEVEL (128)
	// f_function_freq_in<64>(....); NO F COMPUTATIONS AS WE HAVE A RATE 0 NODE AFTER !
	middle_node_pruned_rate_0<64>(decoded + 128, symbols + 128, 64);
	g_function_proba_in_after_rate_0<64>(internal + 896, internal + 768, internal + 832, 64);
	// NODE LEVEL (64)
	// f_function_freq_in<64>(....); NO F COMPUTATIONS AS WE HAVE A RATE 0 NODE AFTER !
	middle_node_pruned_rate_0<64>(decoded + 192, symbols + 192, 32);
	g_function_proba_in_after_rate_0<64>(internal + 960, internal + 896, internal + 928, 32);
	// NODE LEVEL (32)
	// f_function_freq_in<64>(....); NO F COMPUTATIONS AS WE HAVE A RATE 0 NODE AFTER !
	middle_node_pruned_rate_0<64>(decoded + 224, symbols + 224, 16);
	g_function_proba_in_after_rate_0<64>(internal + 992, internal + 960, internal + 976, 16);
	// NODE LEVEL (16)
	f_function_proba_in<64>(internal + 1008, internal + 992, internal + 1000, 8);
	middle_node_pruned_rep_after_f<64>(internal + 1008, decoded + 240, symbols + 240, 8);
	g_function_proba_in<64>(internal + 1008, internal + 992, internal + 1000, symbols + 240, 8);
	// NODE LEVEL (8)
	f_function_proba_in<64>(internal + 1016, internal + 1008, internal + 1012, 4);
	middle_node_pruned_rep_after_f<64>(internal + 1016, decoded + 248, symbols + 248, 4);
	g_function_proba_in<64>(internal + 1016, internal + 1008, internal + 1012, symbols + 248, 4);
	middle_node_pruned_spc_after_g<64>(internal + 1016, decoded + 252, symbols + 252, 4);
	for (int i = 0; i < 4; i += 1)
	{
		symbols[248 + i] ^= symbols[252 + i];
	}
	for (int i = 0; i < 8; i += 1)
	{
		symbols[240 + i] ^= symbols[248 + i];
	}
	for (int i = 0; i < 16; i += 1)
	{
		symbols[224 + i] ^= symbols[240 + i];
	}
	for (int i = 0; i < 32; i += 1)
	{
		symbols[192 + i] ^= symbols[224 + i];
	}
	for (int i = 0; i < 64; i += 1)
	{
		symbols[128 + i] ^= symbols[192 + i];
	}
	for (int i = 0; i < 128; i += 1)
	{
		symbols[0 + i] ^= symbols[128 + i];
	}
	g_function_freq_in<64>(internal + 512, internal + 0, internal + 256, symbols + 0, 256);
	// NODE LEVEL (256)
	f_function_proba_in<64>(internal + 768, internal + 512, internal + 640, 128);
	// NODE LEVEL (128)
	f_function_freq_in<64>(internal + 896, internal + 768, internal + 832, 64);
	middle_node_pruned_rep_after_f<64>(internal + 896, decoded + 256, symbols + 256, 64);
	g_function_freq_in<64>(internal + 896, internal + 768, internal + 832, symbols + 256, 64);
	// NODE LEVEL (64)
	f_function_proba_in<64>(internal + 960, internal + 896, internal + 928, 32);
	middle_node_pruned_rep_after_f<64>(internal + 960, decoded + 320, symbols + 320, 32);
	g_function_proba_in<64>(internal + 960, internal + 896, internal + 928, symbols + 320, 32);
	// NODE LEVEL (32)
	f_function_proba_in<64>(internal + 992, internal + 960, internal + 976, 16);
	// NODE LEVEL (16)
	// f_function_freq_in<64>(....); NO F COMPUTATIONS AS WE HAVE A RATE 0 NODE AFTER !
	middle_node_pruned_rate_0<64>(decoded + 352, symbols + 352, 8);
	g_function_freq_in_after_rate_0<64>(internal + 1008, internal + 992, internal + 1000, 8);
	// NODE LEVEL (8)
	f_function_proba_in<64>(internal + 1016, internal + 1008, internal + 1012, 4);
	middle_node_pruned_rep_after_f<64>(internal + 1016, decoded + 360, symbols + 360, 4);
	g_function_proba_in<64>(internal + 1016, internal + 1008, internal + 1012, symbols + 360, 4);
	middle_node_pruned_spc_after_g<64>(internal + 1016, decoded + 364, symbols + 364, 4);
	for (int i = 0; i < 4; i += 1)
	{
		symbols[360 + i] ^= symbols[364 + i];
	}
	for (int i = 0; i < 8; i += 1)
	{
		symbols[352 + i] ^= symbols[360 + i];
	}
	g_function_proba_in<64>(internal + 992, internal + 960, internal + 976, symbols + 352, 16);
	// NODE LEVEL (16)
	f_function_proba_in<64>(internal + 1008, internal + 992, internal + 1000, 8);
	// NODE LEVEL (8)
	f_function_freq_in<64>(internal + 1016, internal + 1008, internal + 1012, 4);
	middle_node_pruned_rep_after_f<64>(internal + 1016, decoded + 368, symbols + 368, 4);
	g_function_freq_in<64>(internal + 1016, internal + 1008, internal + 1012, symbols + 368, 4);
	middle_node_pruned_spc_after_g<64>(internal + 1016, decoded + 372, symbols + 372, 4);
	for (int i = 0; i < 4; i += 1)
	{
		symbols[368 + i] ^= symbols[372 + i];
	}
	g_function_proba_in<64>(internal + 1008, internal + 992, internal + 1000, symbols + 368, 8);
	middle_node_pruned_spc_after_g<64>(internal + 1008, decoded + 376, symbols + 376, 8);
	for (int i = 0; i < 8; i += 1)
	{
		symbols[368 + i] ^= symbols[376 + i];
	}
	for (int i = 0; i < 16; i += 1)
	{
		symbols[352 + i] ^= symbols[368 + i];
	}
	for (int i = 0; i < 32; i += 1)
	{
		symbols[320 + i] ^= symbols[352 + i];
	}
	for (int i = 0; i < 64; i += 1)
	{
		symbols[256 + i] ^= symbols[320 + i];
	}
	g_function_proba_in<64>(internal + 768, internal + 512, internal + 640, symbols + 256, 128);
	// NODE LEVEL (128)
	f_function_proba_in<64>(internal + 896, internal + 768, internal + 832, 64);
	// NODE LEVEL (64)
	f_function_freq_in<64>(internal + 960, internal + 896, internal + 928, 32);
	// NODE LEVEL (32)
	// f_function_freq_in<64>(....); NO F COMPUTATIONS AS WE HAVE A RATE 0 NODE AFTER !
	middle_node_pruned_rate_0<64>(decoded + 384, symbols + 384, 16);
	g_function_freq_in_after_rate_0<64>(internal + 992, internal + 960, internal + 976, 16);
	// NODE LEVEL (16)
	f_function_proba_in<64>(internal + 1008, internal + 992, internal + 1000, 8);
	middle_node_pruned_rep_after_f<64>(internal + 1008, decoded + 400, symbols + 400, 8);
	g_function_proba_in<64>(internal + 1008, internal + 992, internal + 1000, symbols + 400, 8);
	// NODE LEVEL (8)
	f_function_proba_in<64>(internal + 1016, internal + 1008, internal + 1012, 4);
	middle_node_pruned_rep_after_f<64>(internal + 1016, decoded + 408, symbols + 408, 4);
	g_function_proba_in<64>(internal + 1016, internal + 1008, internal + 1012, symbols + 408, 4);
	middle_node_pruned_spc_after_g<64>(internal + 1016, decoded + 412, symbols + 412, 4);
	for (int i = 0; i < 4; i += 1)
	{
		symbols[408 + i] ^= symbols[412 + i];
	}
	for (int i = 0; i < 8; i += 1)
	{
		symbols[400 + i] ^= symbols[408 + i];
	}
	for (int i = 0; i < 16; i += 1)
	{
		symbols[384 + i] ^= symbols[400 + i];
	}
	g_function_freq_in<64>(internal + 960, internal + 896, internal + 928, symbols + 384, 32);
	// NODE LEVEL (32)
	f_function_proba_in<64>(internal + 992, internal + 960, internal + 976, 16);
	// NODE LEVEL (16)
	f_function_freq_in<64>(internal + 1008, internal + 992, internal + 1000, 8);
	middle_node_pruned_rep_after_f<64>(internal + 1008, decoded + 416, symbols + 416, 8);
	g_function_freq_in<64>(internal + 1008, internal + 992, internal + 1000, symbols + 416, 8);
	middle_node_pruned_spc_after_g<64>(internal + 1008, decoded + 424, symbols + 424, 8);
	for (int i = 0; i < 8; i += 1)
	{
		symbols[416 + i] ^= symbols[424 + i];
	}
	g_function_proba_in<64>(internal + 992, internal + 960, internal + 976, symbols + 416, 16);
	middle_node_pruned_spc_after_g<64>(internal + 992, decoded + 432, symbols + 432, 16);
	for (int i = 0; i < 16; i += 1)
	{
		symbols[416 + i] ^= symbols[432 + i];
	}
	for (int i = 0; i < 32; i += 1)
	{
		symbols[384 + i] ^= symbols[416 + i];
	}
	g_function_proba_in<64>(internal + 896, internal + 768, internal + 832, symbols + 384, 64);
	// NODE LEVEL (64)
	f_function_proba_in<64>(internal + 960, internal + 896, internal + 928, 32);
	// NODE LEVEL (32)
	f_function_freq_in<64>(internal + 992, internal + 960, internal + 976, 16);
	// NODE LEVEL (16)
	f_function_freq_in<64>(internal + 1008, internal + 992, internal + 1000, 8);
	// NODE LEVEL (8)
	f_function_freq_in<64>(internal + 1016, internal + 1008, internal + 1012, 4);
	middle_node_pruned_rep_after_f<64>(internal + 1016, decoded + 448, symbols + 448, 4);
	g_function_freq_in<64>(internal + 1016, internal + 1008, internal + 1012, symbols + 448, 4);
	middle_node_pruned_spc_after_g<64>(internal + 1016, decoded + 452, symbols + 452, 4);
	for (int i = 0; i < 4; i += 1)
	{
		symbols[448 + i] ^= symbols[452 + i];
	}
	g_function_freq_in<64>(internal + 1008, internal + 992, internal + 1000, symbols + 448, 8);
	middle_node_pruned_spc_after_g<64>(internal + 1008, decoded + 456, symbols + 456, 8);
	for (int i = 0; i < 8; i += 1)
	{
		symbols[448 + i] ^= symbols[456 + i];
	}
	g_function_freq_in<64>(internal + 992, internal + 960, internal + 976, symbols + 448, 16);
	middle_node_pruned_spc_after_g<64>(internal + 992, decoded + 464, symbols + 464, 16);
	for (int i = 0; i < 16; i += 1)
	{
		symbols[448 + i] ^= symbols[464 + i];
	}
	g_function_proba_in<64>(internal + 960, internal + 896, internal + 928, symbols + 448, 32);
	middle_node_pruned_spc_after_g<64>(internal + 960, decoded + 480, symbols + 480, 32);
	for (int i = 0; i < 32; i += 1)
	{
		symbols[448 + i] ^= symbols[480 + i];
	}
	for (int i = 0; i < 64; i += 1)
	{
		symbols[384 + i] ^= symbols[448 + i];
	}
	for (int i = 0; i < 128; i += 1)
	{
		symbols[256 + i] ^= symbols[384 + i];
	}
	for (int i = 0; i < 256; i += 1)
	{
		symbols[0 + i] ^= symbols[256 + i];
	}
	g_function_proba_in<64>(internal, channel, channel + 512, symbols, 512);
	// NODE LEVEL (512)
	f_function_proba_in<64>(internal + 512, internal + 0, internal + 256, 256);
	// NODE LEVEL (256)
	f_function_freq_in<64>(internal + 768, internal + 512, internal + 640, 128);
	// NODE LEVEL (128)
	f_function_freq_in<64>(internal + 896, internal + 768, internal + 832, 64);
	// NODE LEVEL (64)
	// f_function_freq_in<64>(....); NO F COMPUTATIONS AS WE HAVE A RATE 0 NODE AFTER !
	middle_node_pruned_rate_0<64>(decoded + 512, symbols + 512, 32);
	g_function_freq_in_after_rate_0<64>(internal + 960, internal + 896, internal + 928, 32);
	// NODE LEVEL (32)
	f_function_proba_in<64>(internal + 992, internal + 960, internal + 976, 16);
	middle_node_pruned_rep_after_f<64>(internal + 992, decoded + 544, symbols + 544, 16);
	g_function_proba_in<64>(internal + 992, internal + 960, internal + 976, symbols + 544, 16);
	// NODE LEVEL (16)
	f_function_proba_in<64>(internal + 1008, internal + 992, internal + 1000, 8);
	middle_node_pruned_rep_after_f<64>(internal + 1008, decoded + 560, symbols + 560, 8);
	g_function_proba_in<64>(internal + 1008, internal + 992, internal + 1000, symbols + 560, 8);
	// NODE LEVEL (8)
	f_function_proba_in<64>(internal + 1016, internal + 1008, internal + 1012, 4);
	// NODE LEVEL (4)
	// f_function_freq_in<64>(....); NO F COMPUTATIONS AS WE HAVE A RATE 0 NODE AFTER !
	middle_node_pruned_rate_0<64>(decoded + 568, symbols + 568, 2);
	g_function_freq_in_after_rate_0<64>(internal + 1020, internal + 1016, internal + 1018, 2);
	middle_node_pruned_rate_1_after_g<64>(internal + 1020, decoded + 570, symbols + 570, 2);
	for (int i = 0; i < 2; i += 1)
	{
		symbols[568 + i] ^= symbols[570 + i];
	}
	g_function_proba_in<64>(internal + 1016, internal + 1008, internal + 1012, symbols + 568, 4);
	middle_node_pruned_rate_1_after_g<64>(internal + 1016, decoded + 572, symbols + 572, 4);
	for (int i = 0; i < 4; i += 1)
	{
		symbols[568 + i] ^= symbols[572 + i];
	}
	for (int i = 0; i < 8; i += 1)
	{
		symbols[560 + i] ^= symbols[568 + i];
	}
	for (int i = 0; i < 16; i += 1)
	{
		symbols[544 + i] ^= symbols[560 + i];
	}
	for (int i = 0; i < 32; i += 1)
	{
		symbols[512 + i] ^= symbols[544 + i];
	}
	g_function_freq_in<64>(internal + 896, internal + 768, internal + 832, symbols + 512, 64);
	// NODE LEVEL (64)
	f_function_proba_in<64>(internal + 960, internal + 896, internal + 928, 32);
	// NODE LEVEL (32)
	f_function_freq_in<64>(internal + 992, internal + 960, internal + 976, 16);
	// NODE LEVEL (16)
	// f_function_freq_in<64>(....); NO F COMPUTATIONS AS WE HAVE A RATE 0 NODE AFTER !
	middle_node_pruned_rate_0<64>(decoded + 576, symbols + 576, 8);
	g_function_freq_in_after_rate_0<64>(internal + 1008, internal + 992, internal + 1000, 8);
	// NODE LEVEL (8)
	// f_function_freq_in<64>(....); NO F COMPUTATIONS AS WE HAVE A RATE 0 NODE AFTER !
	middle_node_pruned_rate_0<64>(decoded + 584, symbols + 584, 4);
	g_function_proba_in_after_rate_0<64>(internal + 1016, internal + 1008, internal + 1012, 4);
	// NODE LEVEL (4)
	// f_function_freq_in<64>(....); NO F COMPUTATIONS AS WE HAVE A RATE 0 NODE AFTER !
	middle_node_pruned_rate_0<64>(decoded + 588, symbols + 588, 2);
	g_function_proba_in_after_rate_0<64>(internal + 1020, internal + 1016, internal + 1018, 2);
	middle_node_pruned_rate_1_after_g<64>(internal + 1020, decoded + 590, symbols + 590, 2);
	for (int i = 0; i < 2; i += 1)
	{
		symbols[588 + i] ^= symbols[590 + i];
	}
	for (int i = 0; i < 4; i += 1)
	{
		symbols[584 + i] ^= symbols[588 + i];
	}
	for (int i = 0; i < 8; i += 1)
	{
		symbols[576 + i] ^= symbols[584 + i];
	}
	g_function_freq_in<64>(internal + 992, internal + 960, internal + 976, symbols + 576, 16);
	// NODE LEVEL (16)
	f_function_proba_in<64>(internal + 1008, internal + 992, internal + 1000, 8);
	// NODE LEVEL (8)
	f_function_freq_in<64>(internal + 1016, internal + 1008, internal + 1012, 4);
	middle_node_pruned_rep_after_f<64>(internal + 1016, decoded + 592, symbols + 592, 4);
	g_function_freq_in<64>(internal + 1016, internal + 1008, internal + 1012, symbols + 592, 4);
	middle_node_pruned_spc_after_g<64>(internal + 1016, decoded + 596, symbols + 596, 4);
	for (int i = 0; i < 4; i += 1)
	{
		symbols[592 + i] ^= symbols[596 + i];
	}
	g_function_proba_in<64>(internal + 1008, internal + 992, internal + 1000, symbols + 592, 8);
	middle_node_pruned_spc_after_g<64>(internal + 1008, decoded + 600, symbols + 600, 8);
	for (int i = 0; i < 8; i += 1)
	{
		symbols[592 + i] ^= symbols[600 + i];
	}
	for (int i = 0; i < 16; i += 1)
	{
		symbols[576 + i] ^= symbols[592 + i];
	}
	g_function_proba_in<64>(internal + 960, internal + 896, internal + 928, symbols + 576, 32);
	// NODE LEVEL (32)
	f_function_proba_in<64>(internal + 992, internal + 960, internal + 976, 16);
	// NODE LEVEL (16)
	f_function_freq_in<64>(internal + 1008, internal + 992, internal + 1000, 8);
	// NODE LEVEL (8)
	f_function_freq_in<64>(internal + 1016, internal + 1008, internal + 1012, 4);
	middle_node_pruned_rep_after_f<64>(internal + 1016, decoded + 608, symbols + 608, 4);
	g_function_freq_in<64>(internal + 1016, internal + 1008, internal + 1012, symbols + 608, 4);
	middle_node_pruned_spc_after_g<64>(internal + 1016, decoded + 612, symbols + 612, 4);
	for (int i = 0; i < 4; i += 1)
	{
		symbols[608 + i] ^= symbols[612 + i];
	}
	g_function_freq_in<64>(internal + 1008, internal + 992, internal + 1000, symbols + 608, 8);
	middle_node_pruned_spc_after_g<64>(internal + 1008, decoded + 616, symbols + 616, 8);
	for (int i = 0; i < 8; i += 1)
	{
		symbols[608 + i] ^= symbols[616 + i];
	}
	g_function_proba_in<64>(internal + 992, internal + 960, internal + 976, symbols + 608, 16);
	middle_node_pruned_spc_after_g<64>(internal + 992, decoded + 624, symbols + 624, 16);
	for (int i = 0; i < 16; i += 1)
	{
		symbols[608 + i] ^= symbols[624 + i];
	}
	for (int i = 0; i < 32; i += 1)
	{
		symbols[576 + i] ^= symbols[608 + i];
	}
	for (int i = 0; i < 64; i += 1)
	{
		symbols[512 + i] ^= symbols[576 + i];
	}
	g_function_freq_in<64>(internal + 768, internal + 512, internal + 640, symbols + 512, 128);
	// NODE LEVEL (128)
	f_function_proba_in<64>(internal + 896, internal + 768, internal + 832, 64);
	// NODE LEVEL (64)
	f_function_freq_in<64>(internal + 960, internal + 896, internal + 928, 32);
	// NODE LEVEL (32)
	f_function_freq_in<64>(internal + 992, internal + 960, internal + 976, 16);
	// NODE LEVEL (16)
	f_function_freq_in<64>(internal + 1008, internal + 992, internal + 1000, 8);
	middle_node_pruned_rep_after_f<64>(internal + 1008, decoded + 640, symbols + 640, 8);
	g_function_freq_in<64>(internal + 1008, internal + 992, internal + 1000, symbols + 640, 8);
	// NODE LEVEL (8)
	f_function_proba_in<64>(internal + 1016, internal + 1008, internal + 1012, 4);
	middle_node_pruned_rep_after_f<64>(internal + 1016, decoded + 648, symbols + 648, 4);
	g_function_proba_in<64>(internal + 1016, internal + 1008, internal + 1012, symbols + 648, 4);
	middle_node_pruned_spc_after_g<64>(internal + 1016, decoded + 652, symbols + 652, 4);
	for (int i = 0; i < 4; i += 1)
	{
		symbols[648 + i] ^= symbols[652 + i];
	}
	for (int i = 0; i < 8; i += 1)
	{
		symbols[640 + i] ^= symbols[648 + i];
	}
	g_function_freq_in<64>(internal + 992, internal + 960, internal + 976, symbols + 640, 16);
	// NODE LEVEL (16)
	f_function_proba_in<64>(internal + 1008, internal + 992, internal + 1000, 8);
	// NODE LEVEL (8)
	f_function_freq_in<64>(internal + 1016, internal + 1008, internal + 1012, 4);
	// NODE LEVEL (4)
	// f_function_freq_in<64>(....); NO F COMPUTATIONS AS WE HAVE A RATE 0 NODE AFTER !
	middle_node_pruned_rate_0<64>(decoded + 656, symbols + 656, 2);
	g_function_freq_in_after_rate_0<64>(internal + 1020, internal + 1016, internal + 1018, 2);
	middle_node_pruned_rate_1_after_g<64>(internal + 1020, decoded + 658, symbols + 658, 2);
	for (int i = 0; i < 2; i += 1)
	{
		symbols[656 + i] ^= symbols[658 + i];
	}
	g_function_freq_in<64>(internal + 1016, internal + 1008, internal + 1012, symbols + 656, 4);
	middle_node_pruned_rate_1_after_g<64>(internal + 1016, decoded + 660, symbols + 660, 4);
	for (int i = 0; i < 4; i += 1)
	{
		symbols[656 + i] ^= symbols[660 + i];
	}
	g_function_proba_in<64>(internal + 1008, internal + 992, internal + 1000, symbols + 656, 8);
	middle_node_pruned_rate_1_after_g<64>(internal + 1008, decoded + 664, symbols + 664, 8);
	for (int i = 0; i < 8; i += 1)
	{
		symbols[656 + i] ^= symbols[664 + i];
	}
	for (int i = 0; i < 16; i += 1)
	{
		symbols[640 + i] ^= symbols[656 + i];
	}
	g_function_freq_in<64>(internal + 960, internal + 896, internal + 928, symbols + 640, 32);
	middle_node_pruned_spc_after_g<64>(internal + 960, decoded + 672, symbols + 672, 32);
	for (int i = 0; i < 32; i += 1)
	{
		symbols[640 + i] ^= symbols[672 + i];
	}
	g_function_proba_in<64>(internal + 896, internal + 768, internal + 832, symbols + 640, 64);
	middle_node_pruned_spc_after_g<64>(internal + 896, decoded + 704, symbols + 704, 64);
	for (int i = 0; i < 64; i += 1)
	{
		symbols[640 + i] ^= symbols[704 + i];
	}
	for (int i = 0; i < 128; i += 1)
	{
		symbols[512 + i] ^= symbols[640 + i];
	}
	g_function_proba_in<64>(internal + 512, internal + 0, internal + 256, symbols + 512, 256);
	// NODE LEVEL (256)
	f_function_proba_in<64>(internal + 768, internal + 512, internal + 640, 128);
	// NODE LEVEL (128)
	f_function_freq_in<64>(internal + 896, internal + 768, internal + 832, 64);
	// NODE LEVEL (64)
	f_function_freq_in<64>(internal + 960, internal + 896, internal + 928, 32);
	// NODE LEVEL (32)
	f_function_freq_in<64>(internal + 992, internal + 960, internal + 976, 16);
	// NODE LEVEL (16)
	f_function_freq_in<64>(internal + 1008, internal + 992, internal + 1000, 8);
	// NODE LEVEL (8)
	// f_function_freq_in<64>(....); NO F COMPUTATIONS AS WE HAVE A RATE 0 NODE AFTER !
	middle_node_pruned_rate_0<64>(decoded + 768, symbols + 768, 4);
	g_function_freq_in_after_rate_0<64>(internal + 1016, internal + 1008, internal + 1012, 4);
	// NODE LEVEL (4)
	// f_function_freq_in<64>(....); NO F COMPUTATIONS AS WE HAVE A RATE 0 NODE AFTER !
	middle_node_pruned_rate_0<64>(decoded + 772, symbols + 772, 2);
	g_function_proba_in_after_rate_0<64>(internal + 1020, internal + 1016, internal + 1018, 2);
	middle_node_pruned_rate_1_after_g<64>(internal + 1020, decoded + 774, symbols + 774, 2);
	for (int i = 0; i < 2; i += 1)
	{
		symbols[772 + i] ^= symbols[774 + i];
	}
	for (int i = 0; i < 4; i += 1)
	{
		symbols[768 + i] ^= symbols[772 + i];
	}
	g_function_freq_in<64>(internal + 1008, internal + 992, internal + 1000, symbols + 768, 8);
	middle_node_pruned_spc_after_g<64>(internal + 1008, decoded + 776, symbols + 776, 8);
	for (int i = 0; i < 8; i += 1)
	{
		symbols[768 + i] ^= symbols[776 + i];
	}
	g_function_freq_in<64>(internal + 992, internal + 960, internal + 976, symbols + 768, 16);
	middle_node_pruned_spc_after_g<64>(internal + 992, decoded + 784, symbols + 784, 16);
	for (int i = 0; i < 16; i += 1)
	{
		symbols[768 + i] ^= symbols[784 + i];
	}
	g_function_freq_in<64>(internal + 960, internal + 896, internal + 928, symbols + 768, 32);
	middle_node_pruned_spc_after_g<64>(internal + 960, decoded + 800, symbols + 800, 32);
	for (int i = 0; i < 32; i += 1)
	{
		symbols[768 + i] ^= symbols[800 + i];
	}
	g_function_freq_in<64>(internal + 896, internal + 768, internal + 832, symbols + 768, 64);
	middle_node_pruned_spc_after_g<64>(internal + 896, decoded + 832, symbols + 832, 64);
	for (int i = 0; i < 64; i += 1)
	{
		symbols[768 + i] ^= symbols[832 + i];
	}
	g_function_proba_in<64>(internal + 768, internal + 512, internal + 640, symbols + 768, 128);
	middle_node_pruned_spc_after_g<64>(internal + 768, decoded + 896, symbols + 896, 128);
	for (int i = 0; i < 128; i += 1)
	{
		symbols[768 + i] ^= symbols[896 + i];
	}
	for (int i = 0; i < 256; i += 1)
	{
		symbols[512 + i] ^= symbols[768 + i];
	}
}

template <int gf_size>
const int decoder_dedicated<gf_size>::N_gen = 1024;

template <int gf_size>
const int decoder_dedicated<gf_size>::K_gen = 512;
