//
// Created by legal on 03/07/2025.
//
#pragma once
#include <immintrin.h>
//
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//
template <int gf_size>
inline void scale_by_inverse(float * data, float factor) {
    for (int i = 0; i < gf_size; i++) {
        data[i] *= factor;
    }
}
