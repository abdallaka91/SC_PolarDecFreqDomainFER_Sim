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
//
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//
template <>
void inline scale_by_inverse<8>(float * data, float factor) {
    // Charger les 8 floats
    const __m256 v = _mm256_loadu_ps(data);

    // Étape 2 : normalisation
    __m256 inv_sum = _mm256_set1_ps(1.0f / factor);
    __m256 v_norm  = _mm256_mul_ps(v, inv_sum);

    // Écrire le résultat normalisé en place
    _mm256_storeu_ps(data, v_norm);
}
//
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//
template <>
void inline scale_by_inverse<16>(float * data, float factor) {
    // Charger les 16 floats en 2 vecteurs
    __m256 v0 = _mm256_loadu_ps(data);     // éléments [0..7]
    __m256 v1 = _mm256_loadu_ps(data + 8); // éléments [8..15]

    // Étape 2 : normalisation
    __m256 inv_sum = _mm256_set1_ps(1.0f / factor);
    v0             = _mm256_mul_ps(v0, inv_sum);
    v1             = _mm256_mul_ps(v1, inv_sum);

    // Stocker le résultat normalisé en place
    _mm256_storeu_ps(data, v0);
    _mm256_storeu_ps(data + 8, v1);
}
//
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//
template <>
void inline scale_by_inverse<32>(float * data, float factor) {
    __m256 v[4]; // 4 x 8 = 32

    // Charger les 32 floats
    for (int i = 0; i < 4; ++i) {
        v[i] = _mm256_loadu_ps(data + i * 8);
    }
    // Normalisation
    __m256 inv_sum = _mm256_set1_ps(1.0f / factor);
    for (int i = 0; i < 4; ++i) {
        v[i] = _mm256_mul_ps(v[i], inv_sum);
        _mm256_storeu_ps(data + i * 8, v[i]);
    }
}
//
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//
template <>
void inline scale_by_inverse<64>(float * data, float factor) {
    __m256 v[8]; // 8 x 8 = 64

    for (int i = 0; i < 8; ++i) {
        v[i] = _mm256_loadu_ps(data + i * 8);
    }

    __m256 inv_sum = _mm256_set1_ps(1.0f / factor);
    for (int i = 0; i < 8; ++i) {
        v[i] = _mm256_mul_ps(v[i], inv_sum);
        _mm256_storeu_ps(data + i * 8, v[i]);
    }
}
//
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//
template <>
void inline scale_by_inverse<128>(float * data, float factor) {
    __m256 v[16]; // 16 x 8 = 128

    for (int i = 0; i < 16; ++i) {
        v[i] = _mm256_loadu_ps(data + i * 8);
    }

    __m256 inv_sum = _mm256_set1_ps(1.0f / factor);
    for (int i = 0; i < 16; ++i) {
        v[i] = _mm256_mul_ps(v[i], inv_sum);
        _mm256_storeu_ps(data + i * 8, v[i]);
    }
}
//
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//
template <>
void inline scale_by_inverse<256>(float * data, float factor) {
    __m256 v[32]; // 32 x 8 = 256

    // Charger et sommer
    for (int i = 0; i < 32; ++i) {
        v[i] = _mm256_loadu_ps(data + i * 8);
    }

    // Normalisation
    __m256 inv_sum = _mm256_set1_ps(1.0f / factor);
    for (int i = 0; i < 32; ++i) {
        v[i] = _mm256_mul_ps(v[i], inv_sum);
        _mm256_storeu_ps(data + i * 8, v[i]);
    }
}
//
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//
template <>
void inline scale_by_inverse<512>(float * data, float factor) {
    __m256 v[64]; // 64 x 8 = 512

    // Charger tous les blocs
    for (int i = 0; i < 64; ++i) {
        v[i] = _mm256_loadu_ps(data + i * 8);
    }

    // Normalisation
    __m256 inv_sum = _mm256_set1_ps(1.0f / factor);
    for (int i = 0; i < 64; ++i) {
        v[i] = _mm256_mul_ps(v[i], inv_sum);
        _mm256_storeu_ps(data + i * 8, v[i]);
    }
}
//
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//
template <>
void inline scale_by_inverse<1024>(float * data, float factor) {
    __m256 v[128]; // 128 x 8 = 1024

    for (int i = 0; i < 128; ++i) {
        v[i] = _mm256_loadu_ps(data + i * 8);
    }
    __m256 inv_sum = _mm256_set1_ps(1.0f / factor);
    for (int i = 0; i < 128; ++i) {
        v[i] = _mm256_mul_ps(v[i], inv_sum);
        _mm256_storeu_ps(data + i * 8, v[i]);
    }
}
//
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//
