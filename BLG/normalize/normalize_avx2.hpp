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
inline void normalize(float *data)
{
    float sum = 1e-32f;
    for (int i = 0; i < gf_size; i += 1)
    {
        sum += data[i];
    }
    const float factor = 1.f / sum;
    for (int i = 0; i < gf_size; i++)
    {
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
void inline normalize<8>(float *data)
{
    // Charger les 8 floats
    const __m256 v = _mm256_loadu_ps(data);

    // Étape 1 : somme horizontale avec initialisation à 1e-32f
    __m128 sum128 = _mm_set_ss(1e-32f); // Valeur initiale pour éviter /0

    __m128 low = _mm256_castps256_ps128(v);    // [0..3]
    __m128 high = _mm256_extractf128_ps(v, 1); // [4..7]

    sum128 = _mm_add_ps(sum128, low);
    sum128 = _mm_add_ps(sum128, high);

    __m128 shuf = _mm_movehdup_ps(sum128);
    sum128 = _mm_add_ps(sum128, shuf);

    shuf = _mm_movehl_ps(shuf, sum128);
    sum128 = _mm_add_ss(sum128, shuf);

    float sum = _mm_cvtss_f32(sum128);

    // Étape 2 : normalisation
    __m256 inv_sum = _mm256_set1_ps(1.0f / sum);
    __m256 v_norm = _mm256_mul_ps(v, inv_sum);

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
void inline normalize<16>(float *data)
{
    // Charger les 16 floats en 2 vecteurs
    __m256 v0 = _mm256_loadu_ps(data);     // éléments [0..7]
    __m256 v1 = _mm256_loadu_ps(data + 8); // éléments [8..15]

    // Étape 1 : somme horizontale avec initialisation à 1e-32f
    __m128 sum128 = _mm_set_ss(1e-32f); // Évite la division par zéro

    // Addition des deux vecteurs en 128 bits
    __m128 low0 = _mm256_castps256_ps128(v0);
    __m128 high0 = _mm256_extractf128_ps(v0, 1);
    __m128 low1 = _mm256_castps256_ps128(v1);
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
template <>
void inline normalize<32>(float *data)
{
    __m256 v[4]; // 4 x 8 = 32
    __m128 sum128 = _mm_set_ss(1e-32f);

    // Charger les 32 floats
    for (int i = 0; i < 4; ++i)
    {
        v[i] = _mm256_loadu_ps(data + i * 8);

        // Somme horizontale partielle
        __m128 low = _mm256_castps256_ps128(v[i]);
        __m128 high = _mm256_extractf128_ps(v[i], 1);
        sum128 = _mm_add_ps(sum128, low);
        sum128 = _mm_add_ps(sum128, high);
    }

    // Réduction horizontale 128 bits
    __m128 shuf = _mm_movehdup_ps(sum128);
    sum128 = _mm_add_ps(sum128, shuf);
    shuf = _mm_movehl_ps(shuf, sum128);
    sum128 = _mm_add_ss(sum128, shuf);
    float sum = _mm_cvtss_f32(sum128);

    // Normalisation
    __m256 inv_sum = _mm256_set1_ps(1.0f / sum);
    for (int i = 0; i < 4; ++i)
    {
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
void inline normalize<64>(float *data)
{
    __m256 v[8]; // 8 x 8 = 64
    __m128 sum128 = _mm_set_ss(1e-32f);

    for (int i = 0; i < 8; ++i)
    {
        v[i] = _mm256_loadu_ps(data + i * 8);
        __m128 low = _mm256_castps256_ps128(v[i]);
        __m128 high = _mm256_extractf128_ps(v[i], 1);
        sum128 = _mm_add_ps(sum128, low);
        sum128 = _mm_add_ps(sum128, high);
    }

    __m128 shuf = _mm_movehdup_ps(sum128);
    sum128 = _mm_add_ps(sum128, shuf);
    shuf = _mm_movehl_ps(shuf, sum128);
    sum128 = _mm_add_ss(sum128, shuf);
    float sum = _mm_cvtss_f32(sum128);

    __m256 inv_sum = _mm256_set1_ps(1.0f / sum);
    for (int i = 0; i < 8; ++i)
    {
        v[i] = _mm256_mul_ps(v[i], inv_sum);
        _mm256_storeu_ps(data + i * 8, v[i]);
    }
}

template <>
void inline normalize<64>(float *data, float fact)
{
    __m256 v[8]; // 8 x 8 = 64

    for (int i = 0; i < 8; ++i)
    {
        v[i] = _mm256_loadu_ps(data + i * 8);
    }

    __m256 Fact = _mm256_set1_ps(fact);
    for (int i = 0; i < 8; ++i)
    {
        v[i] = _mm256_mul_ps(v[i], Fact);
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
void inline normalize<128>(float *data)
{
    __m256 v[16]; // 16 x 8 = 128
    __m128 sum128 = _mm_set_ss(1e-32f);

    for (int i = 0; i < 16; ++i)
    {
        v[i] = _mm256_loadu_ps(data + i * 8);
        __m128 low = _mm256_castps256_ps128(v[i]);
        __m128 high = _mm256_extractf128_ps(v[i], 1);
        sum128 = _mm_add_ps(sum128, low);
        sum128 = _mm_add_ps(sum128, high);
    }

    __m128 shuf = _mm_movehdup_ps(sum128);
    sum128 = _mm_add_ps(sum128, shuf);
    shuf = _mm_movehl_ps(shuf, sum128);
    sum128 = _mm_add_ss(sum128, shuf);
    float sum = _mm_cvtss_f32(sum128);

    __m256 inv_sum = _mm256_set1_ps(1.0f / sum);
    for (int i = 0; i < 16; ++i)
    {
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
void inline normalize<256>(float *data)
{
    __m256 v[32]; // 32 x 8 = 256
    __m128 sum128 = _mm_set_ss(1e-32f);

    // Charger et sommer
    for (int i = 0; i < 32; ++i)
    {
        v[i] = _mm256_loadu_ps(data + i * 8);

        __m128 low = _mm256_castps256_ps128(v[i]);
        __m128 high = _mm256_extractf128_ps(v[i], 1);

        sum128 = _mm_add_ps(sum128, low);
        sum128 = _mm_add_ps(sum128, high);
    }

    // Réduction horizontale
    __m128 shuf = _mm_movehdup_ps(sum128);
    sum128 = _mm_add_ps(sum128, shuf);
    shuf = _mm_movehl_ps(shuf, sum128);
    sum128 = _mm_add_ss(sum128, shuf);

    float sum = _mm_cvtss_f32(sum128);

    // Normalisation
    __m256 inv_sum = _mm256_set1_ps(1.0f / sum);
    for (int i = 0; i < 32; ++i)
    {
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
void inline normalize<512>(float *data)
{
    __m256 v[64]; // 64 x 8 = 512
    __m128 sum128 = _mm_set_ss(1e-32f);

    // Charger tous les blocs
    for (int i = 0; i < 64; ++i)
    {
        v[i] = _mm256_loadu_ps(data + i * 8);

        __m128 low = _mm256_castps256_ps128(v[i]);
        __m128 high = _mm256_extractf128_ps(v[i], 1);

        sum128 = _mm_add_ps(sum128, low);
        sum128 = _mm_add_ps(sum128, high);
    }

    // Réduction horizontale 128 bits
    __m128 shuf = _mm_movehdup_ps(sum128);
    sum128 = _mm_add_ps(sum128, shuf);
    shuf = _mm_movehl_ps(shuf, sum128);
    sum128 = _mm_add_ss(sum128, shuf);

    float sum = _mm_cvtss_f32(sum128);

    // Normalisation
    __m256 inv_sum = _mm256_set1_ps(1.0f / sum);
    for (int i = 0; i < 64; ++i)
    {
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
void inline normalize<1024>(float *data)
{
    __m256 v[128]; // 128 x 8 = 1024
    __m128 sum128 = _mm_set_ss(1e-32f);

    for (int i = 0; i < 128; ++i)
    {
        v[i] = _mm256_loadu_ps(data + i * 8);

        __m128 low = _mm256_castps256_ps128(v[i]);
        __m128 high = _mm256_extractf128_ps(v[i], 1);

        sum128 = _mm_add_ps(sum128, low);
        sum128 = _mm_add_ps(sum128, high);
    }

    __m128 shuf = _mm_movehdup_ps(sum128);
    sum128 = _mm_add_ps(sum128, shuf);
    shuf = _mm_movehl_ps(shuf, sum128);
    sum128 = _mm_add_ss(sum128, shuf);

    float sum = _mm_cvtss_f32(sum128);

    __m256 inv_sum = _mm256_set1_ps(1.0f / sum);
    for (int i = 0; i < 128; ++i)
    {
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
