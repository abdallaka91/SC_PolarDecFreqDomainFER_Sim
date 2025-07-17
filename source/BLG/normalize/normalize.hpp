//
// Created by legal on 03/07/2025.
//
#pragma once

template <int gf_size>
void normalize(float * tab) {
    float sum = 1e-32f;
    for (int i = 0; i < gf_size; i += 1) {
        sum += tab[i];
    }
    const float factor = 1.f / sum;
    for (int i = 0; i < gf_size; i++) {
        tab[i] *= factor;
    }
}
