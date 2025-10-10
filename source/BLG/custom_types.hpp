#pragma once

#include <cstdint>
#include "code.hpp"

//
// For generic NB polar decoders
//
struct symbols_t {
    float value[_GF_];
    bool is_freq;
};

//
// For specialized NB polar decoders
//
template<int gf_size>
struct symbols_s {
    float value[gf_size];
};

