#pragma once
//
//
#include <cstdint>
#include <cmath>
#include "definitions/code.hpp"
#include "features/archi.hpp"
//
//
// For generic NB polar decoders
//
//
struct symbols_t {
    float value[_GF_];
    bool is_freq;
};
//
//
inline symbols_t convert_to_symbols_t(const float* values, const int gf_size, const bool is_freq) {
    symbols_t result;
    result.is_freq = is_freq;
    for (int i = 0; i < gf_size; i++) {
        result.value[i] = values[i];
    }
    return result;
}
//
//
inline void convert_from_symbols_t(float* dst, const symbols_t& src, const int gf_size) {
    for (int i = 0; i < gf_size; i++) {
        dst[i] = src.value[i];
    }
}
//
//
//
// For specialized NB polar decoders
//
//
//
template<int gf_size>
struct symbols_s {
    float value[gf_size];
};
//
//
template<int gf_size>
inline symbols_s<gf_size> convert_to_symbols_s(const float* values) {
    symbols_s<gf_size> result;
    for (int i = 0; i < gf_size; i++) {
        result.value[i] = values[i];
    }
    return result;
}
//
//
template<int gf_size>
inline void convert_from_symbols_s(float* dst, const symbols_s<gf_size>& src) {
    for (int i = 0; i < gf_size; i++) {
        dst[i] = src.value[i];
    }
}
//
//
//
// For specialized NB polar decoders
//
//
//
template<int gf_size>
struct symbols_i {
    int32_t value[gf_size];
    bool is_freq;
};
//
//
template<int gf_size>
inline symbols_i<gf_size> convert_to_symbols_i(const symbols_s<gf_size> symb) {
    symbols_i<gf_size> result;
    for (int i = 0; i < gf_size; i++) {
        const double  v = symb.value[i];
        const int32_t w = (int32_t)round(v * 268435456.0);
        result.value[i] = w;
    }
    result.is_freq = false;
    return result;
}
//
//
template<int gf_size>
inline void convert_from_symbols_i(float* dst, const symbols_i<gf_size>& src) {
    for (int i = 0; i < gf_size; i++) {
        const double  v = src.value[i];
        const float   w = (float)(v / 268435456.0);
        dst[i] = w;
    }
}
//
//
