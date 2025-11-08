//!
//!     Copyright (c) 2020-2023, Bertrand LE GAL
//!     All rights reserved.
//!
//!     Redistribution and use in source and binary forms, with or without
//!     modification, are not permitted with written authorization.
//!
//!
#include "vec_cplx_max_indexes.hpp"
#include "../vec_cplx_max_index/vec_cplx_max_index.hpp"
//
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//
void vec_cplx_max_indexes(float* real, float* imag, const int length, int& pmax1, int& pmax2)
{
#if defined (__ARM_NEON__) || defined(__ARM_NEON)
#else
#endif
    const int p1 = vec_cplx_max_index(real, imag, length);
    const int save_real = real[p1];
    const int save_imag = imag[p1];
    real[p1] = 0;
    imag[p1] = 0;
    const int p2 = vec_cplx_max_index(real, imag, length);
    real[p1] = save_real;
    imag[p1] = save_imag;
    pmax1 = p1;
    pmax2 = p2;
}

void vec_cplx_max_indexes(complex<float>* RI, const int length, int& pmax1, int& pmax2)
{
#if defined (__ARM_NEON__) || defined(__ARM_NEON)
#else
#endif
    const int p1 = vec_cplx_max_index(RI, length);
    const complex<float> save = RI[p1];
    RI[p1] = 0;
    const int p2 = vec_cplx_max_index(RI, length);
    RI[p1] = save;
    pmax1 = p1;
    pmax2 = p2;
}
