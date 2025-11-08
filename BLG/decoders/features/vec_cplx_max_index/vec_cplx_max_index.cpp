//!
//!     Copyright (c) 2020-2023, Bertrand LE GAL
//!     All rights reserved.
//!
//!     Redistribution and use in source and binary forms, with or without
//!     modification, are not permitted with written authorization.
//!
//!
#include "vec_cplx_max_index.hpp"
#if defined (__ARM_NEON__) || defined(__ARM_NEON)
    #include "vec_cplx_max_index_neon.hpp"
#else
    #include "vec_cplx_max_index_x86.hpp"
#endif
//
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//
int vec_cplx_max_index(const float* real, const float* imag, const int length)
{
#if defined (__ARM_NEON__) || defined(__ARM_NEON)
    return vec_cplx_max_index_neon(real, imag, length);
#else
    return vec_cplx_max_index_x86(real, imag, length);
#endif
}
//
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//
int vec_cplx_max_index(const complex<float>* RI, const int length)
{
#if defined (__ARM_NEON__) || defined(__ARM_NEON)
    return vec_cplx_max_index_neon(RI, length);
#else
    return vec_cplx_max_index_x86(RI, length);
#endif
}
//
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//
void vec_cplx_max_index(const float* real, const float* imag, int* position, float* valeur, const int length)
{
#if defined (__ARM_NEON__) || defined(__ARM_NEON)
    vec_cplx_max_index_neon(real, imag, position, valeur, length);
#else
    vec_cplx_max_index_x86(real, imag, length, position, valeur);
#endif
}
//
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//
void vec_cplx_max_index(const complex<float>* RI, int* position, float* valeur, const int length)
{
#if defined (__ARM_NEON__) || defined(__ARM_NEON)
    vec_cplx_max_index_neon(RI, position, valeur, length);
#else
    vec_cplx_max_index_x86(RI, length, position, valeur);
#endif
}
//
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//
