//!
//!     Copyright (c) 2020-2023, Bertrand LE GAL
//!     All rights reserved.
//!
//!     Redistribution and use in source and binary forms, with or without
//!     modification, are not permitted with written authorization.
//!
//!
#include "vec_cplx_max_index.hpp"
#include <cstdint>
#include <cmath>
#include <algorithm>
#include <limits.h>
#include <cfloat>
//
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//
#ifdef __AVX2__
#include <immintrin.h>

void vec_cplx_max_index_x86(
        const float* real,
        const float* imag,
        const int length, int* position, float* valeur
    )
{
    const int simd  = sizeof(__m256) / sizeof(float);
    //const int first = length & ~(simd - 1);

    const __m256i increment  = _mm256_setr_epi32(8, 8, 8, 8, 8, 8, 8, 8);
          __m256i indices    = _mm256_setr_epi32(0, 1, 2, 3, 4, 5, 6, 7);
          __m256i maxindices = _mm256_setzero_si256();
          __m256  maxvalues  = _mm256_setzero_ps();

#if defined (__clang__)
    #pragma unroll
#endif
    for (int i = 0; i < length; i += simd)
    {
        const __m256 re     = _mm256_loadu_ps (real + i);
        const __m256 im     = _mm256_loadu_ps (imag + i);
        const __m256 rr     = re * re;
        const __m256 ii     = im * im;
        const __m256 values = rr + ii;
        const __m256 gt     = _mm256_cmp_ps   (values, maxvalues, _CMP_GT_OQ);
        maxindices          = _mm256_blendv_epi8(maxindices, indices, _mm256_castps_si256(gt));
        maxvalues           = _mm256_max_ps   (values, maxvalues);
        indices             = _mm256_add_epi32(indices, increment);
    }

    float   values_array [simd];
    int32_t indices_array[simd];

    _mm256_storeu_ps   (           values_array,  maxvalues);
    _mm256_storeu_si256( (__m256i*)indices_array, maxindices);

    int   maxindex = indices_array[0];
    float maxvalue =  values_array[0];
    
#if defined (__clang__)
    #pragma unroll
#endif
    for (int i = 1; i < simd; i++)
    {
        if (values_array[i] > maxvalue)
        {
            maxvalue = values_array [i];
            maxindex = indices_array[i];
        }
        else if( values_array[i] == maxvalue )
        {
            if( maxindex > indices_array[i] )
            {
                maxvalue = values_array [i];
                maxindex = indices_array[i];
            }
        }
    }

    *position = maxindex;
    *valeur   = sqrtf( maxvalue );
};
//
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//
void vec_cplx_max_index_x86(
        const complex<float>* RI,
        const int length, int* position, float* valeur
)
{
    constexpr int simd    = sizeof(__m256) / sizeof(float);
    constexpr int simdx2  = 2 * simd;

    const float* real = ((const float*)RI);
    const float* imag = ((const float*)RI) + simd;

    const __m256i increment  = _mm256_setr_epi32(8, 8, 8, 8, 8, 8, 8, 8);
          __m256i indices    = _mm256_setr_epi32(0, 1, 4, 5, 2, 3, 6, 7);   // a cause des _mm256_shuffle_ps
          __m256i maxindices = _mm256_setzero_si256();
          __m256  maxvalues  = _mm256_setzero_ps();

    const int length_iq = 2 * length;
#if defined (__clang__)
    #pragma unroll
#endif
    for (int i = 0; i < length_iq; i += simdx2)
    {
        const __m256 inLo = _mm256_loadu_ps  (real); // Q3 I3 Q2 I2 Q1 I1 Q0 I0
        const __m256 inHi = _mm256_loadu_ps  (imag); // Q7 I7 Q6 I6 Q5 I5 Q4 I4
        const __m256 re   = _mm256_shuffle_ps(inLo, inHi, _MM_SHUFFLE (2, 0, 2, 0));
        const __m256 im   = _mm256_shuffle_ps(inLo, inHi, _MM_SHUFFLE (3, 1, 3, 1));
        const __m256 rr   = re * re;
        const __m256 ii   = im * im;
        const __m256 vals = rr + ii;

        const __m256 gt = _mm256_cmp_ps     (vals, maxvalues, _CMP_GT_OQ);
        maxindices      = _mm256_blendv_epi8(maxindices, indices, _mm256_castps_si256(gt));
        maxvalues       = _mm256_max_ps     (vals, maxvalues);
        indices         = _mm256_add_epi32(indices, increment);
        real           += simdx2;
        imag           += simdx2;
    }

    float   values_array [simd];
    int32_t indices_array[simd];

    _mm256_storeu_ps   (           values_array,  maxvalues);
    _mm256_storeu_si256( (__m256i*)indices_array, maxindices);

    int   maxindex = indices_array[0];
    float maxvalue =  values_array[0];
    
#if defined (__clang__)
    #pragma unroll
#endif
    for (int i = 1; i < simd; i++)
    {
        if (values_array[i] > maxvalue)
        {
            maxvalue = values_array [i];
            maxindex = indices_array[i];
        }
        else if( values_array[i] == maxvalue )
        {
            if( maxindex > indices_array[i] )
            {
                maxvalue = values_array [i];
                maxindex = indices_array[i];
            }
        }
    }
    *position = maxindex;
    *valeur   = sqrtf( maxvalue );
}
//
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//
int vec_cplx_max_index_x86(const complex<float>* RI, const int length)
{
    int   position;
    float valeur;
    vec_cplx_max_index_x86(RI, length, &position, &valeur);
    return position;
}
//
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//
int vec_cplx_max_index_x86(const float* real, const float* imag, const int length)
{
    int   position;
    float valeur;
    vec_cplx_max_index_x86(real, imag, length, &position, &valeur);
    return position;
}
//
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//
#else
//
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//
void vec_cplx_max_index_x86(const float* real, const float* imag, int* position, float* valeur, const int length)
{
    float maxvalue  = 0.0f;
    int maxindex = 0;
    for (int i = 0; i < length; i += 1)
    {
        const float R = real[i];
        const float I = imag[i];
        const float M = (R * R) + (I * I);
        if( maxvalue < M )
        {
            maxvalue  = M;
            maxindex = i;
        }
    }
    *position = maxindex;
    *valeur   = sqrtf( maxvalue );
}
//
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//
void vec_cplx_max_index_x86(const complex<float>* RI, int* position, float* valeur, const int length)
{
    float maxvalue  = 0.0f;
    int maxindex = 0;
    for (int i = 0; i < length; i += 1)
    {
        const float R = RI[i].real();
        const float I = RI[i].imag();
        const float M = (R * R) + (I * I);
        if( maxvalue < M )
        {
            maxvalue  = M;
            maxindex = i;
        }
    }
    *position = maxindex;
    *valeur   = sqrtf( maxvalue );
}
#endif
//
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//
