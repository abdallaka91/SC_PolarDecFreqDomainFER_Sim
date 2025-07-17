//!
//!     Copyright (c) 2020-2023, Bertrand LE GAL
//!     All rights reserved.
//!
//!     Redistribution and use in source and binary forms, with or without
//!     modification, are not permitted with written authorization.
//!
//!
#include "argmax.hpp"
#include <cmath>
//
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//
#ifdef __AVX2__
#include <immintrin.h>

template<int length> void argmax(const float* values, int* position, float* valeur)
{
    const int simd  = sizeof(__m256) / sizeof(float);

    const __m256i increment  = _mm256_setr_epi32(8, 8, 8, 8, 8, 8, 8, 8);
          __m256i indices    = _mm256_setr_epi32(0, 1, 2, 3, 4, 5, 6, 7);
          __m256i maxindices = _mm256_setzero_si256();
          __m256  maxvalues  = _mm256_setzero_ps();

#if defined (__clang__)
    #pragma unroll
#endif
    for (int i = 0; i < length; i += simd)
    {
        const __m256 value  = _mm256_loadu_ps (values + i);
        const __m256 gt     = _mm256_cmp_ps   (value, maxvalues, _CMP_GT_OQ);
        maxindices          = _mm256_blendv_epi8(maxindices, indices, _mm256_castps_si256(gt));
        maxvalues           = _mm256_max_ps   (value, maxvalues);
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
template<int length> int argmax(const float* values)
{
    const int simd  = sizeof(__m256) / sizeof(float);

    const __m256i increment  = _mm256_setr_epi32(8, 8, 8, 8, 8, 8, 8, 8);
          __m256i indices    = _mm256_setr_epi32(0, 1, 2, 3, 4, 5, 6, 7);
          __m256i maxindices = _mm256_setzero_si256();
          __m256  maxvalues  = _mm256_setzero_ps();

#if defined (__clang__)
    #pragma unroll
#endif
    for (int i = 0; i < length; i += simd)
    {
        const __m256 value  = _mm256_loadu_ps (values + i);
        const __m256 gt     = _mm256_cmp_ps   (value, maxvalues, _CMP_GT_OQ);
        maxindices          = _mm256_blendv_epi8(maxindices, indices, _mm256_castps_si256(gt));
        maxvalues           = _mm256_max_ps   (value, maxvalues);
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
    return  maxindex;
};
//
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//
#endif