#pragma once
#include "definitions/custom_types.hpp"
#include "decoders/decoder.hpp"
#include "frozen_tree.hpp"
#include <vector>

template <int gf_size>
class decoder_dedicated : public decoder {
public:
    decoder_dedicated();
    decoder_dedicated(const int n, const int* frozen_symb);
    virtual ~decoder_dedicated();

    void execute(symbols_t * channel, uint16_t* decoded);

private:
    symbols_t* internal;
    uint16_t*  symbols;
//    int*       frozen;

//    const int N;
//    int           f_tree_cnt;

//    frozen_tree * f_tree;
//    frozen_tree pruned_tree;

    static const int N_gen;
    static const int K_gen;
};
