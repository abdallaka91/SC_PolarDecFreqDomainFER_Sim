#pragma once

#include "definitions/custom_types.hpp"

template <int gf_size>
inline __attribute__((always_inline)) void leaf_node_rate_0(
    uint16_t * __restrict decoded,
    uint16_t * __restrict symbols,
    const int  symbol_id) {
    decoded[symbol_id] = 0;
    symbols[symbol_id] = 0;
}
