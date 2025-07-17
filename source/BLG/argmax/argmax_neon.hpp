//!
//!     Copyright (c) 2020-2023, Bertrand LE GAL
//!     All rights reserved.
//!
//!     Redistribution and use in source and binary forms, with or without
//!     modification, are not permitted with written authorization.
//!
//!
#include "argmax.hpp"
#include <arm_neon.h>
//
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//
template<int size> void argmax(const float* values, int* position, float* valeur)
{
    const int32x4_t increment  = { 4, 4, 4, 4 };
          int32x4_t indices    = { 0, 1, 2, 3 };
          int32x4_t maxindices = indices;

    float32x4_t maxvalues  = vld1q_f32( values );

#pragma unroll
    for (int i = 4; i < size; i += 4)
    {
        indices = vaddq_s32(indices, increment);
        const float32x4_t value  = vld1q_f32( values + i);
        const uint32x4_t  gt     = vcgtq_f32(value, maxvalues);
        maxindices               = vbslq_s32(gt, indices, maxindices);
        maxvalues                = vmaxq_f32(value, maxvalues);
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
    *valeur   = maxvalue;
}
//
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//
template<int length>
int argmax(const float* values)
{
    const int32x4_t increment  = { 4, 4, 4, 4 };
    int32x4_t indices    = { 0, 1, 2, 3 };
    int32x4_t maxindices = indices;

    float32x4_t maxvalues  = vld1q_f32( values );

#pragma unroll
    for (int i = 4; i < length; i += 4)
    {
        indices = vaddq_s32(indices, increment);
        const float32x4_t value  = vld1q_f32( values + i);
        const uint32x4_t  gt     = vcgtq_f32(value, maxvalues);
        maxindices               = vbslq_s32(gt, indices, maxindices);
        maxvalues                = vmaxq_f32(value, maxvalues);
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
    return maxindex;
}
//
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//
