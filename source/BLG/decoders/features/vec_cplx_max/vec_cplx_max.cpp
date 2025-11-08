//!
//!     Copyright (c) 2020-2023, Bertrand LE GAL
//!     All rights reserved.
//!
//!     Redistribution and use in source and binary forms, with or without
//!     modification, are not permitted with written authorization.
//!
//!
#include "vec_cplx_max.hpp"
#include <float.h>
//
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//
float vec_cplx_max(const float* real, const float* imag, const int length)
{
    float max_mod = 0.0f;
    for (int i = 0; i < length; i++)
    {
        const float R = real[i];
        const float I = imag[i];
        const float M = (R * R) + (I * I);
        max_mod = std::max(max_mod, M);
    }
    return sqrt(max_mod);
}
//
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//
float vec_cplx_max(const complex<float>* RI, const int length)
{
    float max_mod = 0.0f;
    for (int i = 0; i < length; i++)
    {
        const float R = RI[i].real();
        const float I = RI[i].imag();
        const float M = (R * R) + (I * I);
        max_mod = std::max(max_mod, M);
    }
    return sqrt(max_mod);
}
//
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//
