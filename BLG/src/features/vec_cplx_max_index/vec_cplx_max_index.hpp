//!
//!     Copyright (c) 2020-2023, Bertrand LE GAL
//!     All rights reserved.
//!
//!     Redistribution and use in source and binary forms, with or without
//!     modification, are not permitted with written authorization.
//!
//!
#ifndef _vec_cplx_max_index_
#define _vec_cplx_max_index_
//
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//
#include <cstdint>
#include <cmath>
#include <string>
#include <complex>
#include <vector>
#include <iostream>
//
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//
using namespace std;
//
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//
extern int vec_cplx_max_index(const float* real, const float* imag, const int length);
extern int vec_cplx_max_index(const complex<float>* RI,             const int length);

extern void vec_cplx_max_index(const float* real, const float* imag, int* position, float* valeur, const int length);
extern void vec_cplx_max_index(const complex<float>* RI,             int* position, float* valeur, const int length);

//
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//
#endif