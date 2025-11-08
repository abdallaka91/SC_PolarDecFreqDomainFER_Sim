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
template <int gf_size> inline void normalize(float * tab) {
    float sum = 1e-32f;
    for (int i = 0; i < gf_size; i += 1) {
        sum += tab[i];
    }
    const float factor = 1.f / sum;
    for (int i = 0; i < gf_size; i++) {
        tab[i] *= factor;
    }
}
//
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//
template <> inline void normalize<8>(float * data) {
    // Charger les 8 floats
    const __m256 v = _mm256_loadu_ps(data);

    // Étape 1 : somme horizontale avec initialisation à 1e-32f
    __m128 sum128 = _mm_set_ss(1e-32f);  // Valeur initiale pour éviter /0

    __m128 low = _mm256_castps256_ps128(v);     // [0..3]
    __m128 high = _mm256_extractf128_ps(v, 1);  // [4..7]

    sum128 = _mm_add_ps(sum128, low);
    sum128 = _mm_add_ps(sum128, high);

    __m128 shuf = _mm_movehdup_ps(sum128);
    sum128 = _mm_add_ps(sum128, shuf);

    shuf = _mm_movehl_ps(shuf, sum128);
    sum128 = _mm_add_ss(sum128, shuf);

    float sum = _mm_cvtss_f32(sum128);

    // Étape 2 : normalisation
    __m256 inv_sum = _mm256_set1_ps(1.0f / sum);
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
template <> inline void normalize<16>(float * data) {
    // Charger les 16 floats en 2 vecteurs
    __m256 v0 = _mm256_loadu_ps(data);       // éléments [0..7]
    __m256 v1 = _mm256_loadu_ps(data + 8);   // éléments [8..15]

    // Étape 1 : somme horizontale avec initialisation à 1e-32f
    __m128 sum128 = _mm_set_ss(1e-32f); // Évite la division par zéro

    // Addition des deux vecteurs en 128 bits
    __m128 low0  = _mm256_castps256_ps128(v0);
    __m128 high0 = _mm256_extractf128_ps(v0, 1);
    __m128 low1  = _mm256_castps256_ps128(v1);
    __m128 high1 = _mm256_extractf128_ps(v1, 1);

    sum128 = _mm_add_ps(sum128, low0);
    sum128 = _mm_add_ps(sum128, high0);
    sum128 = _mm_add_ps(sum128, low1);
    sum128 = _mm_add_ps(sum128, high1);

    // Réduction horizontale sur 128 bits
    __m128 shuf = _mm_movehdup_ps(sum128);
    sum128 = _mm_add_ps(sum128, shuf);

    shuf = _mm_movehl_ps(shuf, sum128);
    sum128 = _mm_add_ss(sum128, shuf);

    float sum = _mm_cvtss_f32(sum128);

    // Étape 2 : normalisation
    __m256 inv_sum = _mm256_set1_ps(1.0f / sum);
    v0 = _mm256_mul_ps(v0, inv_sum);
    v1 = _mm256_mul_ps(v1, inv_sum);

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
template <> inline void normalize<32>(float * data) {
    // Charger les deux vecteurs de 16 floats chacun
    __m512 v0 = _mm512_loadu_ps(data);
    __m512 v1 = _mm512_loadu_ps(data + 16);

    // Somme des vecteurs avec 1e-32f pour éviter la division par zéro
    __m512 sum = _mm512_add_ps(_mm512_set1_ps(1e-32f), v0);
    sum = _mm512_add_ps(sum, v1);

    // Réduction horizontale pour obtenir la somme totale
    float total_sum = _mm512_reduce_add_ps(sum);

    // Calcul de l'inverse pour la normalisation
    __m512 inv_sum = _mm512_set1_ps(1.0f / total_sum);

    // Normalisation in-place des vecteurs
    v0 = _mm512_mul_ps(v0, inv_sum);
    v1 = _mm512_mul_ps(v1, inv_sum);

    // Stockage des résultats normalisés
    _mm512_storeu_ps(data, v0);
    _mm512_storeu_ps(data + 16, v1);
}
//
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//
template <> inline void normalize<64>(float * data) {
    __m512 v0 = _mm512_loadu_ps(data);
    __m512 v1 = _mm512_loadu_ps(data + 16);
    __m512 v2 = _mm512_loadu_ps(data + 32);
    __m512 v3 = _mm512_loadu_ps(data + 48);

    // Somme des vecteurs avec 1e-32f pour éviter division par zéro
    __m512 sum = _mm512_add_ps(_mm512_set1_ps(1e-32f), v0);
    sum = _mm512_add_ps(sum, v1);
    sum = _mm512_add_ps(sum, v2);
    sum = _mm512_add_ps(sum, v3);

    // Réduction horizontale pour somme totale
    float total_sum = _mm512_reduce_add_ps(sum);

    // Calcul de l'inverse pour normalisation
    __m512 inv_sum = _mm512_set1_ps(1.0f / total_sum);

    // Normalisation in-place des vecteurs
    v0 = _mm512_mul_ps(v0, inv_sum);
    v1 = _mm512_mul_ps(v1, inv_sum);
    v2 = _mm512_mul_ps(v2, inv_sum);
    v3 = _mm512_mul_ps(v3, inv_sum);

    // Stockage des résultats normalisés
    _mm512_storeu_ps(data, v0);
    _mm512_storeu_ps(data + 16, v1);
    _mm512_storeu_ps(data + 32, v2);
    _mm512_storeu_ps(data + 48, v3);
}
//
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//
template <> inline void normalize<128>(float * data) {
    __m512 v[8];  // 8 × 16 = 128
    __m512 sum = _mm512_set1_ps(1e-32f);

    for (int i = 0; i < 8; ++i) {
        v[i] = _mm512_loadu_ps(data + i * 16);
        sum = _mm512_add_ps(sum, v[i]);
    }

    float total_sum = _mm512_reduce_add_ps(sum);

    __m512 inv_sum = _mm512_set1_ps(1.0f / total_sum);
    for (int i = 0; i < 8; ++i) {
        v[i] = _mm512_mul_ps(v[i], inv_sum);
        _mm512_storeu_ps(data + i * 16, v[i]);
    }
}
//
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//
template <> inline void normalize<256>(float * data) {
    __m512 v[16];  // 16 x 16 = 256
    __m512 sum = _mm512_set1_ps(1e-32f);

    for (int i = 0; i < 16; ++i) {
        v[i] = _mm512_loadu_ps(data + i * 16);
        sum = _mm512_add_ps(sum, v[i]);
    }

    // Réduction horizontale
    float total_sum = _mm512_reduce_add_ps(sum);

    // Normalisation
    __m512 inv_sum = _mm512_set1_ps(1.0f / total_sum);
    for (int i = 0; i < 16; ++i) {
        v[i] = _mm512_mul_ps(v[i], inv_sum);
        _mm512_storeu_ps(data + i * 16, v[i]);
    }
}
//
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//
template <> inline void normalize<512>(float * data) {
    __m512 v[32];  // 32 x 16 = 512
    __m512 sum = _mm512_set1_ps(1e-32f);

    for (int i = 0; i < 32; ++i) {
        v[i] = _mm512_loadu_ps(data + i * 16);
        sum = _mm512_add_ps(sum, v[i]);
    }

    float total_sum = _mm512_reduce_add_ps(sum);

    __m512 inv_sum = _mm512_set1_ps(1.0f / total_sum);
    for (int i = 0; i < 32; ++i) {
        v[i] = _mm512_mul_ps(v[i], inv_sum);
        _mm512_storeu_ps(data + i * 16, v[i]);
    }
}
//
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//
template <> inline void normalize<1024>(float * data) {
    __m512 v[64];  // 64 x 16 = 1024
    __m512 sum = _mm512_set1_ps(1e-32f);

    for (int i = 0; i < 64; ++i) {
        v[i] = _mm512_loadu_ps(data + i * 16);
        sum = _mm512_add_ps(sum, v[i]);
    }

    float total_sum = _mm512_reduce_add_ps(sum);

    __m512 inv_sum = _mm512_set1_ps(1.0f / total_sum);
    for (int i = 0; i < 64; ++i) {
        v[i] = _mm512_mul_ps(v[i], inv_sum);
        _mm512_storeu_ps(data + i * 16, v[i]);
    }
}
//
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//
