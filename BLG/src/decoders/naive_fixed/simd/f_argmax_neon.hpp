//!
//!     Copyright (c) 2020-2023, Bertrand LE GAL
//!     All rights reserved.
//!
//!     Redistribution and use in source and binary forms, with or without
//!     modification, are not permitted with written authorization.
//!
//!
#include "../../../features/argmax/argmax.hpp"
#include <arm_neon.h>
//
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//
#include <arm_neon.h>

template<int size>
inline void argmax_int32(const int32_t* values, int* position, int* valeur)
{
    const int32x4_t increment  = {4, 4, 4, 4};
    int32x4_t indices    = {0, 1, 2, 3};
    int32x4_t maxindices = indices;

    // Charger les 4 premières valeurs
    int32x4_t maxvalues = vld1q_s32(values);

#if defined (__clang__)
    #pragma unroll
#endif
    for (int i = 4; i < size; i += 4)
    {
        indices = vaddq_s32(indices, increment);
        const int32x4_t value = vld1q_s32(values + i);
        const uint32x4_t gt   = vcgtq_s32(value, maxvalues);
        maxindices             = vbslq_s32(gt, indices, maxindices);
        maxvalues              = vmaxq_s32(value, maxvalues);
    }

    // Extraction scalaire finale
    int32_t values_array [4];
    int32_t indices_array[4];

    vst1q_s32(values_array,  maxvalues);
    vst1q_s32(indices_array, maxindices);

    int maxindex = indices_array[0];
    int maxvalue = values_array[0];

#if defined (__clang__)
    #pragma unroll
#endif
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
inline int argmax(const int32_t* values)
{
    const int32x4_t increment  = {4, 4, 4, 4};
    int32x4_t indices    = {0, 1, 2, 3};
    int32x4_t maxindices = indices;

    int32x4_t maxvalues = vld1q_s32(values);

#if defined (__clang__)
#pragma unroll
#endif
    for (int i = 4; i < length; i += 4)
    {
        indices = vaddq_s32(indices, increment);
        const int32x4_t value = vld1q_s32(values + i);
        const uint32x4_t gt   = vcgtq_s32(value, maxvalues);
        maxindices             = vbslq_s32(gt, indices, maxindices);
        maxvalues              = vmaxq_s32(value, maxvalues);
    }

    int32_t values_array [4];
    int32_t indices_array[4];

    vst1q_s32(values_array,  maxvalues);
    vst1q_s32(indices_array, maxindices);

    int maxindex = indices_array[0];
    int maxvalue = values_array[0];

#if defined (__clang__)
#pragma unroll
#endif
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
