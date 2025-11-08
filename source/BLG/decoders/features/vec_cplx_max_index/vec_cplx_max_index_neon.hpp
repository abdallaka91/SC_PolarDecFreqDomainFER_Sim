//!
//!     Copyright (c) 2020-2023, Bertrand LE GAL
//!     All rights reserved.
//!
//!     Redistribution and use in source and binary forms, with or without
//!     modification, are not permitted with written authorization.
//!
//!
#include "vec_cplx_max_index.hpp"
#include <arm_neon.h>
//
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//
void vec_cplx_max_index_neon(const float* real, const float* imag, int* position, float* valeur, const int size)
{
    const int32x4_t increment  = { 4, 4, 4, 4 };
          int32x4_t indices    = { 0, 1, 2, 3 };
          int32x4_t maxindices = indices;

    float32x4_t r0         = vld1q_f32( real );
    float32x4_t i0         = vld1q_f32( imag );
    float32x4_t maxvalues  = (r0 * r0) + (i0 * i0);

#if defined (__clang__)
    #pragma unroll(16)
#endif
    for (int i = 4; i < size; i += 4)
    {
        indices = vaddq_s32(indices, increment);
        const float32x4_t rn     = vld1q_f32( real + i);
        const float32x4_t in     = vld1q_f32( imag + i);
        const float32x4_t values = (rn * rn) + (in * in);
        const uint32x4_t  gt     = vcgtq_f32(values, maxvalues);
        maxindices               = vbslq_s32(gt, indices, maxindices);
        maxvalues                = vmaxq_f32  (values, maxvalues);
    }

    float   values_array [4];
    int32_t indices_array[4];

    vst1q_f32(values_array,  maxvalues);
    vst1q_s32(indices_array, maxindices);

    int   maxindex = indices_array[0];
    float maxvalue =  values_array[0];
#pragma unroll
    for (int i = 1; i < 4; i++)
    {
        if (values_array[i] > maxvalue)
        {
            maxvalue = values_array[i];
            maxindex = indices_array[i];
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
void vec_cplx_max_index_neon(const complex<float>* RI, int* position, float* valeur, const int length)
{
    const int32x4_t increment = { 4, 4, 4, 4 };
    int32x4_t indices         = { 0, 1, 2, 3 };
    int32x4_t maxindices      = indices;

    const float* ptr = (float*)RI;

    const float32x4x2_t ri0 = vld2q_f32( ptr );
    const float32x4_t   r0  = ri0.val[0];
    const float32x4_t   i0  = ri0.val[1];
    float32x4_t maxvalues  = (r0 * r0) + (i0 * i0);

    const int size = 2 * length;
    for (int i = 8; i < size; i += 8)
    {
        indices = vaddq_s32(indices, increment);
        const float32x4x2_t ri   = vld2q_f32( ptr + i );
        const float32x4_t   rn   = ri.val[0];
        const float32x4_t   in   = ri.val[1];
        const float32x4_t values = (rn * rn) + (in * in);
        const uint32x4_t  gt     = vcgtq_f32(values, maxvalues);
        maxindices               = vbslq_s32(gt, indices, maxindices);
        maxvalues                = vmaxq_f32(values, maxvalues);
    }

    float   values_array [4];
    int32_t indices_array[4];

    vst1q_f32(values_array,  maxvalues);
    vst1q_s32(indices_array, maxindices);

    int   maxindex = indices_array[0];
    float maxvalue =  values_array[0];
#pragma unroll
    for (int i = 1; i < 4; i++)
    {
        if (values_array[i] > maxvalue)
        {
            maxvalue = values_array[i];
            maxindex = indices_array[i];
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
int vec_cplx_max_index_neon(const complex<float>* RI, const int length)
{
    int   position;
    float valeur;
    vec_cplx_max_index_neon(RI, &position, &valeur, length);
    return position;
}
//
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//
int vec_cplx_max_index_neon(const float* real, const float* imag, const int size)
{
    int   position;
    float valeur;
    vec_cplx_max_index_neon(real, imag, &position, &valeur, size);
    return position;
}
//
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//
