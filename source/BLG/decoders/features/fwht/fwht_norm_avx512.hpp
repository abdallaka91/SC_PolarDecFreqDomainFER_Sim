/**
  Copyright (c) 2012-2015 "Bordeaux INP, Bertrand LE GAL"
  [http://legal.vvv.enseirb-matmeca.fr]

  This file is part of LDPC_C_Simulator.

  LDPC_C_Simulator is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#pragma once
#ifdef __AVX512F__

#include "definitions/const_config_GF64_N64.hpp"
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <immintrin.h>

template <uint16_t GF>
inline void fwht_norm_avx512(float x[]) {
    assert(x != 0);
    assert(true);
    exit(x != nullptr);
}

#if 0
template <uint16_t GF>
inline void fwht_norm_avx512(float x[], float y[]) {
    assert(x != 0);
    assert(y != 0);
    assert(true);
    exit((x != NULL) + (y != NULL));
}
#endif
//
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
inline void fwht16_norm_terminale(const __m256 X0, const __m256 X1, float y[], const __m256 factor) {
    //
    // ON LOAD LES COEFFICIENTS NECESSAIRE A LA TRANFORMATION DES TUILES BASSES
    //
    const __m256 M0 = _mm256_castsi256_ps(_mm256_setr_epi32(0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x80000000, 0x80000000, 0x80000000, 0x80000000));
    const __m256 M1 = _mm256_castsi256_ps(_mm256_setr_epi32(0x00000000, 0x00000000, 0x80000000, 0x80000000, 0x00000000, 0x00000000, 0x80000000, 0x80000000));
    const __m256 M2 = _mm256_castsi256_ps(_mm256_setr_epi32(0x00000000, 0x80000000, 0x00000000, 0x80000000, 0x00000000, 0x80000000, 0x00000000, 0x80000000));

    //////////////////////////////////////////////////////
    //	const __m256 B0 = fwht8_avx512 ( A0 );
    const __m256 HAUT = _mm256_add_ps(X0, X1);
    const __m256 N0   = _mm256_xor_ps(HAUT, M0);
    const __m256 N1   = _mm256_permute2f128_ps(HAUT, HAUT, 0x01);
    const __m256 N2   = _mm256_add_ps(N0, N1);
    const __m256 O0   = _mm256_xor_ps(N2, M1);
    const __m256 O1   = _mm256_shuffle_ps(N2, N2, 0x4E);
    const __m256 O2   = _mm256_add_ps(O0, O1);
    const __m256 P0   = _mm256_xor_ps(O2, M2);
    const __m256 P1   = _mm256_shuffle_ps(O2, O2, 0xB1);
    const __m256 P2   = _mm256_add_ps(P0, P1);
    const __m256 P3   = _mm256_mul_ps(P2, factor);
    _mm256_storeu_ps(y + 0, P3);
    //////////////////////////////////////////////////////
    //	const __m256 B0 = fwht8_avx512 ( A0 );
    const __m256 BAS = _mm256_sub_ps(X0, X1);
    const __m256 B0  = _mm256_xor_ps(BAS, M0);
    const __m256 B1  = _mm256_permute2f128_ps(BAS, BAS, 0x01);
    const __m256 B2  = _mm256_add_ps(B0, B1);
    const __m256 BO0 = _mm256_xor_ps(B2, M1);
    const __m256 BO1 = _mm256_shuffle_ps(B2, B2, 0x4E);
    const __m256 BO2 = _mm256_add_ps(BO0, BO1);
    const __m256 BP0 = _mm256_xor_ps(BO2, M2);
    const __m256 BP1 = _mm256_shuffle_ps(BO2, BO2, 0xB1);
    const __m256 BP2 = _mm256_add_ps(BP0, BP1);
    const __m256 BP3 = _mm256_mul_ps(BP2, factor);
    _mm256_storeu_ps(y + 8, BP3);
    //////////////////////////////////////////////////////
}
inline void fwht16_norm_terminale(const __m512 X, float y[], const __m512 factor) {
    const __m256 X0 = _mm512_extractf32x8_ps(X, 0);
    const __m256 X1 = _mm512_extractf32x8_ps(X, 1);
    const __m256 F0 = _mm512_extractf32x8_ps(factor, 0);
    fwht16_norm_terminale(X0, X1, y, F0);
}

inline void fwht16_norm_flat_avx512(float x[], float y[], const __m512 factor) {
    const __m256 X0 = _mm256_loadu_ps(x + 0);
    const __m256 X1 = _mm256_loadu_ps(x + 8);
    const __m256 F0 = _mm512_extractf32x8_ps(factor, 0);
    fwht16_norm_terminale(X0, X1, y, F0);
}

inline void fwht32_norm_terminale(const __m512 X0, const __m512 X1, float y[], const __m512 factor) {
    const __m512 A0 = X0 + X1;
    const __m512 B0 = X0 - X1;
    fwht16_norm_terminale(A0, y +  0, factor);
    fwht16_norm_terminale(B0, y + 16, factor);
}

inline void fwht32_norm_flat_avx512(float x[], float y[], const __m512 factor) {
    const __m512 X0 = _mm512_loadu_ps(x +  0);
    const __m512 X2 = _mm512_loadu_ps(x + 16);
    const __m512 m0 = _mm512_add_ps(X0, X2);
    fwht16_norm_terminale(m0, y +  0, factor);
    const __m512 M0 = _mm512_sub_ps(X0, X2);
    fwht16_norm_terminale(M0, y + 16, factor);
}

inline void fwht64_norm_terminale(const __m512 X0, const __m512 X1, const __m512 X2, const __m512 X3, float y[], const __m512 factor)
{
    const __m512 A0 = X0 + X2;
    const __m512 A1 = X1 + X3;
    const __m512 B0 = X0 - X2;
    const __m512 B1 = X1 - X3;

    fwht32_norm_terminale(A0, A1, y +  0, factor);
    fwht32_norm_terminale(B0, B1, y + 32, factor);
}

inline void fwht64_norm_flat_avx512(float x[], float y[], const __m512 factor) {
    const __m512 X0 = _mm512_loadu_ps(x + 0);
    const __m512 X1 = _mm512_loadu_ps(x + 16);
    const __m512 X2 = _mm512_loadu_ps(x + 32);
    const __m512 X3 = _mm512_loadu_ps(x + 48);

    const __m512 A0 = X0 + X2, A1 = X1 + X3;
    const __m512 B0 = X0 - X2, B1 = X1 - X3;

    fwht32_norm_terminale(A0, A1, y +  0, factor);
    fwht32_norm_terminale(B0, B1, y + 32, factor);
}

inline void fwht128_norm_terminale(__m512 X0, __m512 X1, __m512 X2, __m512 X3, __m512 X4, __m512 X5, __m512 X6, __m512 X7, float y[], const __m512 factor) {
    const __m512 A0 = X0 + X4, A1 = X1 + X5, A2 = X2 + X6, A3 = X3 + X7;
    const __m512 B0 = X0 - X4, B1 = X1 - X5, B2 = X2 - X6, B3 = X3 - X7;
    fwht64_norm_terminale(A0, A1, A2, A3, y +  0, factor);
    fwht64_norm_terminale(B0, B1, B2, B3, y + 64, factor);
}

inline void fwht128_norm_flat_avx512(float x[], float y[], const __m512 factor) {
    const __m512 X0 = _mm512_loadu_ps(x + 0);
    const __m512 X1 = _mm512_loadu_ps(x + 16);
    const __m512 X2 = _mm512_loadu_ps(x + 32);
    const __m512 X3 = _mm512_loadu_ps(x + 48);
    const __m512 X4 = _mm512_loadu_ps(x + 64);
    const __m512 X5 = _mm512_loadu_ps(x + 80);
    const __m512 X6 = _mm512_loadu_ps(x + 96);
    const __m512 X7 = _mm512_loadu_ps(x + 112);

    fwht128_norm_terminale(X0, X1, X2, X3, X4, X5, X6, X7, y, factor);
}

inline void fwht256_norm_flat_avx512(float x[], float y[], const __m512 factor) {
    const __m512 X0  = (_mm512_loadu_ps(x +   0) + _mm512_loadu_ps(x + 128));
    const __m512 X2  = (_mm512_loadu_ps(x +  16) + _mm512_loadu_ps(x + 144));
    const __m512 X4  = (_mm512_loadu_ps(x +  32) + _mm512_loadu_ps(x + 160));
    const __m512 X6  = (_mm512_loadu_ps(x +  48) + _mm512_loadu_ps(x + 176));
    const __m512 X8  = (_mm512_loadu_ps(x +  64) + _mm512_loadu_ps(x + 192));
    const __m512 X10 = (_mm512_loadu_ps(x +  80) + _mm512_loadu_ps(x + 208));
    const __m512 X12 = (_mm512_loadu_ps(x +  96) + _mm512_loadu_ps(x + 224));
    const __m512 X14 = (_mm512_loadu_ps(x + 112) + _mm512_loadu_ps(x + 240));
 
    const __m512 x0  = (_mm512_loadu_ps(x +   0) - _mm512_loadu_ps(x + 128));
    const __m512 x2  = (_mm512_loadu_ps(x +  16) - _mm512_loadu_ps(x + 144));
    const __m512 x4  = (_mm512_loadu_ps(x +  32) - _mm512_loadu_ps(x + 160));
    const __m512 x6  = (_mm512_loadu_ps(x +  48) - _mm512_loadu_ps(x + 176));
    const __m512 x8  = (_mm512_loadu_ps(x +  64) - _mm512_loadu_ps(x + 192));
    const __m512 x10 = (_mm512_loadu_ps(x +  80) - _mm512_loadu_ps(x + 208));
    const __m512 x12 = (_mm512_loadu_ps(x +  96) - _mm512_loadu_ps(x + 224));
    const __m512 x14 = (_mm512_loadu_ps(x + 112) - _mm512_loadu_ps(x + 240));

    fwht128_norm_terminale(X0, X2, X4, X6, X8, X10, X12, X14,  y,       factor);
    fwht128_norm_terminale(x0, x2, x4, x6, x8, x10, x12, x14,  y + 128, factor);
}
//
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
inline void fwht512_norm_flat_avx512(float* srcdst, const float factor_s = 0.04419417382f)
{
    const int simd = sizeof(__m512) / sizeof(float);
    for (int i = 0; i < 256; i+= simd) {
        const __m512 A = _mm512_loadu_ps(srcdst + i      );
        const __m512 B = _mm512_loadu_ps(srcdst + i + 256);
        const __m512 C = _mm512_add_ps (A, B);
        const __m512 D = _mm512_sub_ps (A, B);
        _mm512_storeu_ps(srcdst + i +   0, C);
        _mm512_storeu_ps(srcdst + i + 256, D);
    }
    const __m512 factor = _mm512_set1_ps(factor_s);
    fwht256_norm_flat_avx512(srcdst,       srcdst,       factor);
    fwht256_norm_flat_avx512(srcdst + 256, srcdst + 256, factor);
}
//
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
inline void fwht1024_norm_flat_avx512(float* srcdst)
{
    const int simd = sizeof(__m512) / sizeof(float);
    for (int i = 0; i < 512; i+= simd) {
        const __m512 A = _mm512_loadu_ps(srcdst + i      );
        const __m512 B = _mm512_loadu_ps(srcdst + i + 512);
        const __m512 C = _mm512_add_ps (A, B);
        const __m512 D = _mm512_sub_ps (A, B);
        _mm512_storeu_ps(srcdst + i +   0, C);
        _mm512_storeu_ps(srcdst + i + 512, D);
    }
    fwht512_norm_flat_avx512(srcdst,       0.03125f);
    fwht512_norm_flat_avx512(srcdst + 512, 0.03125f);
}
//
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
inline __m256 fwht8_norm_avx512(const __m256 x, const __m256 factor) {
    const __m256 M0 = _mm256_castsi256_ps(_mm256_setr_epi32(0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x80000000, 0x80000000, 0x80000000, 0x80000000));
    const __m256 N0 = _mm256_xor_ps(x, M0);
    const __m256 N1 = _mm256_permute2f128_ps(x, x, 0x01);
    const __m256 N2 = _mm256_add_ps(N0, N1);

    const __m256 M1 = _mm256_castsi256_ps(_mm256_setr_epi32(0x00000000, 0x00000000, 0x80000000, 0x80000000, 0x00000000, 0x00000000, 0x80000000, 0x80000000));
    const __m256 O0 = _mm256_xor_ps(N2, M1);
    const __m256 O1 = _mm256_shuffle_ps(N2, N2, 0x4E);
    const __m256 O2 = _mm256_add_ps(O0, O1);

    const __m256 M2 = _mm256_castsi256_ps(_mm256_setr_epi32(0x00000000, 0x80000000, 0x00000000, 0x80000000, 0x00000000, 0x80000000, 0x00000000, 0x80000000));
    const __m256 P0 = _mm256_xor_ps(O2, M2);
    const __m256 P1 = _mm256_shuffle_ps(O2, O2, 0xB1);
    const __m256 P2 = _mm256_add_ps(P0, P1);
    const __m256 P3 = _mm256_mul_ps(P2, factor);
    return P3;
}
//
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
inline void fwht16_norm_avx512(__m256 * B0, __m256 * B1, const __m256 factor) {
    const __m256 C0 = _mm256_add_ps(*B0, *B1);
    const __m256 C1 = _mm256_sub_ps(*B0, *B1);
    *B0             = fwht8_norm_avx512(C0, factor);
    *B1             = fwht8_norm_avx512(C1, factor);
}
//
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
inline void fwht32_norm_avx512(__m256 * B0, __m256 * B1, __m256 * B2, __m256 * B3, const __m256 factor) {
    __m256 C0 = _mm256_add_ps(*B0, *B2);
    __m256 C1 = _mm256_add_ps(*B1, *B3);

    __m256 C2 = _mm256_sub_ps(*B0, *B2);
    __m256 C3 = _mm256_sub_ps(*B1, *B3);

    fwht16_norm_avx512(&C0, &C1, factor);
    fwht16_norm_avx512(&C2, &C3, factor);

    *B0 = C0;
    *B1 = C1;
    *B2 = C2;
    *B3 = C3;
}
//
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
template <>
inline void fwht_norm_avx512<8>(float x[]) {
    const __m256 factor = _mm256_set1_ps(0.35355339059f);
    const __m256 C0     = _mm256_loadu_ps(x);
    const __m256 D0     = fwht8_norm_avx512(C0, factor);
    _mm256_storeu_ps(x, D0);
}
//
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
template <>
inline void fwht_norm_avx512<16>(float x[]) {
    const __m512 factor = _mm512_set1_ps(0.25f);
    fwht16_norm_flat_avx512(x, x, factor);
}
//
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
template <>
inline void fwht_norm_avx512<32>(float x[]) {
    const __m512 factor = _mm512_set1_ps(0.17677669529f);
    fwht32_norm_flat_avx512(x, x, factor);
}
//
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
template <>
inline void fwht_norm_avx512<64>(float x[]) {
    const __m512 factor = _mm512_set1_ps(0.125f);
    fwht64_norm_flat_avx512(x, x, factor);
}
//
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
template <>
inline void fwht_norm_avx512<128>(float x[]) {
    const __m512 factor = _mm512_set1_ps(0.08838834764f);
    fwht128_norm_flat_avx512(x, x, factor);
}
#if 0
template <>
inline void fwht_norm_avx512<128>(float x[], float y[]) {
    const __m256 factor = _mm256_set1_ps(0.08838834764f);
    fwht128_norm_flat_avx512(x, y, factor);
}
#endif
//
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
template <> inline void fwht_norm_avx512<256>(float x[]) {
    const __m512 factor = _mm512_set1_ps(0.0625f);
    fwht256_norm_flat_avx512(x, x, factor);
}
#if 0
template <> inline void fwht_norm_avx512<256>(float x[], float y[]) {
    const __m256 factor = _mm256_set1_ps(0.0625f);
    fwht256_norm_flat_avx512(x, y, factor);
}
#endif
//
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
template <> inline void fwht_norm_avx512<512>(float x[]) {
    fwht512_norm_flat_avx512(x);
}
//
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
template <> inline void fwht_norm_avx512<1024>(float x[]) {
    fwht1024_norm_flat_avx512(x);
}
//
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
#endif