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
#if defined(__ARM_NEON__) || defined(__ARM_NEON)
#include <arm_neon.h>
#include <string>

template <uint16_t galois_size> inline void fwht_neon(float x[]) {
    assert(x != 0);
    assert(true);
    exit(x != nullptr);
}

#if 0
template <uint16_t galois_size> inline void fwht_neon(float x[], float y[]) {
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
#ifndef  neon_functions
#define  neon_functions

#define ABCD_to_BADC(a) vrev64q_f32(a)
#define ABCD_to_CDAB(a) vextq_f32(a)

inline void show_vector(const float32x4_t reg, const std::string str) {
    float tab[4];
    vst1q_f32(tab, reg);
    printf("[%s] : float32x4_t[%6.3f, %6.3f, %6.3f, %6.3f]\n", str.c_str(), tab[0], tab[1], tab[2], tab[3]);
}

inline float32x4x2_t vld1q_x2_f32(const float * ptr) {
    float32x4x2_t A;
    A.val[0] = vld1q_f32(ptr);
    A.val[1] = vld1q_f32(ptr + 4);
    return A;
}

inline void vst1q_x2_f32(float * ptr, const float32x4x2_t A) {
    vst1q_f32(ptr + 0, A.val[0]);
    vst1q_f32(ptr + 4, A.val[1]);
}

//inline void vst1q_x2_f32(float * ptr, const float32x4x2_t A) {
//    vst1q_f32(ptr + 0, A.val[0]);
//    vst1q_f32(ptr + 4, A.val[1]);
//}

inline float32x4x2_t vaddq_x2_f32(const float32x4x2_t A, const float32x4x2_t B) {
    float32x4x2_t C;
    C.val[0] = vaddq_f32(A.val[0], B.val[0]);
    C.val[1] = vaddq_f32(A.val[1], B.val[1]);
    return C;
}

inline float32x4x2_t vsubq_x2_f32(const float32x4x2_t A, const float32x4x2_t B) {
    float32x4x2_t C;
    C.val[0] = vsubq_f32(A.val[0], B.val[0]);
    C.val[1] = vsubq_f32(A.val[1], B.val[1]);
    return C;
}
#endif
//
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
inline float32x4x2_t fwht8_neon(const float32x4_t X0, const float32x4_t X1) {
    float32x4x2_t    resu;
    const uint32x4_t m0 = {0x00000000, 0x00000000, 0x80000000, 0x80000000};
    const uint32x4_t m1 = {0x00000000, 0x80000000, 0x00000000, 0x80000000};
    //
    //////////////////////////////////////////////////////
    //
    const float32x4_t HH = vaddq_f32(X0, X1);
    const float32x4_t N0 = vextq_f32(HH, HH, 2);
    const float32x4_t N1 = vreinterpretq_f32_u32(veorq_u32(vreinterpretq_u32_f32(HH), m0));
    const float32x4_t N2 = vaddq_f32(N0, N1);
    const float32x4_t V0 = vreinterpretq_f32_u32(veorq_u32(vreinterpretq_u32_f32(N2), m1));
    const float32x4_t V1 = vrev64q_f32(N2);
    const float32x4_t V2 = vaddq_f32(V0, V1);
    resu.val[0]          = V2;
    //
    //////////////////////////////////////////////////////
    //
    const float32x4_t BB = vsubq_f32(X0, X1);
    const float32x4_t O0 = vextq_f32(BB, BB, 2);
    const float32x4_t O1 = vreinterpretq_f32_u32(veorq_u32(vreinterpretq_u32_f32(BB), m0));
    const float32x4_t O2 = vaddq_f32(O0, O1);
    const float32x4_t Q0 = vreinterpretq_f32_u32(veorq_u32(vreinterpretq_u32_f32(O2), m1));
    const float32x4_t Q1 = vrev64q_f32(O2);
    const float32x4_t Q2 = vaddq_f32(Q0, Q1);
    resu.val[1]          = Q2;
    return resu;
}
//
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
inline void fwht16_neon(const float32x4x2_t A, const float32x4x2_t B, float y[]) {
    {
        const float32x4x2_t T0 = vaddq_x2_f32(A, B);
        const float32x4x2_t C  = fwht8_neon(T0.val[0], T0.val[1]);
        vst1q_x2_f32(y, C);
    }
    {
        const float32x4x2_t R0 = vsubq_x2_f32(A, B);
        const float32x4x2_t D  = fwht8_neon(R0.val[0], R0.val[1]);
        vst1q_x2_f32(y + 8, D);
    }
}
//
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
inline void fwht16_flat_neon(float x[], float y[]) {
    const float32x4x2_t A = vld1q_x2_f32(x);
    const float32x4x2_t B = vld1q_x2_f32(x + 8);
    fwht16_neon(A, B, y);
}
//
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
inline void fwht32_neon(
    float32x4x2_t X0,
    float32x4x2_t X1,
    float32x4x2_t X2,
    float32x4x2_t X3, float y[]) {
    const float32x4x2_t A0 = vaddq_x2_f32(X0, X2), A1 = vaddq_x2_f32(X1, X3);
    const float32x4x2_t B0 = vsubq_x2_f32(X0, X2), B1 = vsubq_x2_f32(X1, X3);
    fwht16_neon(A0, A1, y + 0);
    fwht16_neon(B0, B1, y + 16);
}

inline void fwht32_flat_neon(float x[], float y[]) {
    const float32x4x2_t X0 = vld1q_x2_f32(x + 0);
    const float32x4x2_t X1 = vld1q_x2_f32(x + 8);
    const float32x4x2_t X2 = vld1q_x2_f32(x + 16);
    const float32x4x2_t X3 = vld1q_x2_f32(x + 24);
    const float32x4x2_t m0 = vaddq_x2_f32(X0, X2), m1 = vaddq_x2_f32(X1, X3);
    fwht16_neon(m0, m1, y + 0);
    const float32x4x2_t M0 = vsubq_x2_f32(X0, X2), M1 = vsubq_x2_f32(X1, X3);
    fwht16_neon(M0, M1, y + 16);
}

inline void fwht64_neon(float32x4x2_t X0, float32x4x2_t X1, float32x4x2_t X2, float32x4x2_t X3, float32x4x2_t X4, float32x4x2_t X5, float32x4x2_t X6, float32x4x2_t X7, float y[]) {
    const float32x4x2_t A0 = vaddq_x2_f32(X0, X4), A1 = vaddq_x2_f32(X1, X5);
    const float32x4x2_t A2 = vaddq_x2_f32(X2, X6), A3 = vaddq_x2_f32(X3, X7);
    fwht32_neon(A0, A1, A2, A3, y + 0);
    const float32x4x2_t B0 = vsubq_x2_f32(X0, X4), B1 = vsubq_x2_f32(X1, X5);
    const float32x4x2_t B2 = vsubq_x2_f32(X2, X6), B3 = vsubq_x2_f32(X3, X7);
    fwht32_neon(B0, B1, B2, B3, y + 32);
}

inline void fwht64_flat_neon(float x[], float y[]) {
    const float32x4x2_t X0 = vld1q_x2_f32(x + 0);
    const float32x4x2_t X1 = vld1q_x2_f32(x + 8);
    const float32x4x2_t X2 = vld1q_x2_f32(x + 16);
    const float32x4x2_t X3 = vld1q_x2_f32(x + 24);
    const float32x4x2_t X4 = vld1q_x2_f32(x + 32);
    const float32x4x2_t X5 = vld1q_x2_f32(x + 40);
    const float32x4x2_t X6 = vld1q_x2_f32(x + 48);
    const float32x4x2_t X7 = vld1q_x2_f32(x + 56);

    const float32x4x2_t A0 = vaddq_x2_f32(X0, X4), A1 = vaddq_x2_f32(X1, X5);
    const float32x4x2_t A2 = vaddq_x2_f32(X2, X6), A3 = vaddq_x2_f32(X3, X7);
    fwht32_neon(A0, A1, A2, A3, y + 0);

    const float32x4x2_t B0 = vsubq_x2_f32(X0, X4), B1 = vsubq_x2_f32(X1, X5);
    const float32x4x2_t B2 = vsubq_x2_f32(X2, X6), B3 = vsubq_x2_f32(X3, X7);
    fwht32_neon(B0, B1, B2, B3, y + 32);
}

inline void fwht128_neon(float32x4x2_t X0, float32x4x2_t X1, float32x4x2_t X2, float32x4x2_t X3, float32x4x2_t X4, float32x4x2_t X5, float32x4x2_t X6, float32x4x2_t X7, float32x4x2_t X8, float32x4x2_t X9, float32x4x2_t X10, float32x4x2_t X11, float32x4x2_t X12, float32x4x2_t X13, float32x4x2_t X14, float32x4x2_t X15, float y[]) {
    const float32x4x2_t A0 = vaddq_x2_f32(X0, X8), A1 = vaddq_x2_f32(X1, X9), A2 = vaddq_x2_f32(X2, X10), A3 = vaddq_x2_f32(X3, X11);
    const float32x4x2_t A4 = vaddq_x2_f32(X4, X12), A5 = vaddq_x2_f32(X5, X13), A6 = vaddq_x2_f32(X6, X14), A7 = vaddq_x2_f32(X7, X15);
    fwht64_neon(A0, A1, A2, A3, A4, A5, A6, A7, y + 0);
    const float32x4x2_t B0 = vsubq_x2_f32(X0, X8), B1 = vsubq_x2_f32(X1, X9), B2 = vsubq_x2_f32(X2, X10), B3 = vsubq_x2_f32(X3, X11);
    const float32x4x2_t B4 = vsubq_x2_f32(X4, X12), B5 = vsubq_x2_f32(X5, X13), B6 = vsubq_x2_f32(X6, X14), B7 = vsubq_x2_f32(X7, X15);
    fwht64_neon(B0, B1, B2, B3, B4, B5, B6, B7, y + 64);
}

inline void fwht128_flat_neon(float x[], float y[]) {
    const float32x4x2_t X0  = vld1q_x2_f32(x + 0);
    const float32x4x2_t X1  = vld1q_x2_f32(x + 8);
    const float32x4x2_t X2  = vld1q_x2_f32(x + 16);
    const float32x4x2_t X3  = vld1q_x2_f32(x + 24);
    const float32x4x2_t X4  = vld1q_x2_f32(x + 32);
    const float32x4x2_t X5  = vld1q_x2_f32(x + 40);
    const float32x4x2_t X6  = vld1q_x2_f32(x + 48);
    const float32x4x2_t X7  = vld1q_x2_f32(x + 56);
    const float32x4x2_t X8  = vld1q_x2_f32(x + 64);
    const float32x4x2_t X9  = vld1q_x2_f32(x + 72);
    const float32x4x2_t X10 = vld1q_x2_f32(x + 80);
    const float32x4x2_t X11 = vld1q_x2_f32(x + 88);
    const float32x4x2_t X12 = vld1q_x2_f32(x + 96);
    const float32x4x2_t X13 = vld1q_x2_f32(x + 104);
    const float32x4x2_t X14 = vld1q_x2_f32(x + 112);
    const float32x4x2_t X15 = vld1q_x2_f32(x + 120);

    fwht128_neon(X0, X1, X2, X3, X4, X5, X6, X7, X8, X9, X10, X11, X12, X13, X14, X15, y);
}

inline void fwht256_flat_neon(float x[], float y[])
{
    const float32x4x2_t X0  = vaddq_x2_f32(vld1q_x2_f32(x + 0), vld1q_x2_f32(x + 128));
    const float32x4x2_t X1  = vaddq_x2_f32(vld1q_x2_f32(x + 8), vld1q_x2_f32(x + 136));
    const float32x4x2_t X2  = vaddq_x2_f32(vld1q_x2_f32(x + 16), vld1q_x2_f32(x + 144));
    const float32x4x2_t X3  = vaddq_x2_f32(vld1q_x2_f32(x + 24), vld1q_x2_f32(x + 152));
    const float32x4x2_t X4  = vaddq_x2_f32(vld1q_x2_f32(x + 32), vld1q_x2_f32(x + 160));
    const float32x4x2_t X5  = vaddq_x2_f32(vld1q_x2_f32(x + 40), vld1q_x2_f32(x + 168));
    const float32x4x2_t X6  = vaddq_x2_f32(vld1q_x2_f32(x + 48), vld1q_x2_f32(x + 176));
    const float32x4x2_t X7  = vaddq_x2_f32(vld1q_x2_f32(x + 56), vld1q_x2_f32(x + 184));
    const float32x4x2_t X8  = vaddq_x2_f32(vld1q_x2_f32(x + 64), vld1q_x2_f32(x + 192));
    const float32x4x2_t X9  = vaddq_x2_f32(vld1q_x2_f32(x + 72), vld1q_x2_f32(x + 200));
    const float32x4x2_t X10 = vaddq_x2_f32(vld1q_x2_f32(x + 80), vld1q_x2_f32(x + 208));
    const float32x4x2_t X11 = vaddq_x2_f32(vld1q_x2_f32(x + 88), vld1q_x2_f32(x + 216));
    const float32x4x2_t X12 = vaddq_x2_f32(vld1q_x2_f32(x + 96), vld1q_x2_f32(x + 224));
    const float32x4x2_t X13 = vaddq_x2_f32(vld1q_x2_f32(x + 104), vld1q_x2_f32(x + 232));
    const float32x4x2_t X14 = vaddq_x2_f32(vld1q_x2_f32(x + 112), vld1q_x2_f32(x + 240));
    const float32x4x2_t X15 = vaddq_x2_f32(vld1q_x2_f32(x + 120), vld1q_x2_f32(x + 248));

    const float32x4x2_t x0  = vsubq_x2_f32(vld1q_x2_f32(x + 0), vld1q_x2_f32(x + 128));
    const float32x4x2_t x1  = vsubq_x2_f32(vld1q_x2_f32(x + 8), vld1q_x2_f32(x + 136));
    const float32x4x2_t x2  = vsubq_x2_f32(vld1q_x2_f32(x + 16), vld1q_x2_f32(x + 144));
    const float32x4x2_t x3  = vsubq_x2_f32(vld1q_x2_f32(x + 24), vld1q_x2_f32(x + 152));
    const float32x4x2_t x4  = vsubq_x2_f32(vld1q_x2_f32(x + 32), vld1q_x2_f32(x + 160));
    const float32x4x2_t x5  = vsubq_x2_f32(vld1q_x2_f32(x + 40), vld1q_x2_f32(x + 168));
    const float32x4x2_t x6  = vsubq_x2_f32(vld1q_x2_f32(x + 48), vld1q_x2_f32(x + 176));
    const float32x4x2_t x7  = vsubq_x2_f32(vld1q_x2_f32(x + 56), vld1q_x2_f32(x + 184));
    const float32x4x2_t x8  = vsubq_x2_f32(vld1q_x2_f32(x + 64), vld1q_x2_f32(x + 192));
    const float32x4x2_t x9  = vsubq_x2_f32(vld1q_x2_f32(x + 72), vld1q_x2_f32(x + 200));
    const float32x4x2_t x10 = vsubq_x2_f32(vld1q_x2_f32(x + 80), vld1q_x2_f32(x + 208));
    const float32x4x2_t x11 = vsubq_x2_f32(vld1q_x2_f32(x + 88), vld1q_x2_f32(x + 216));
    const float32x4x2_t x12 = vsubq_x2_f32(vld1q_x2_f32(x + 96), vld1q_x2_f32(x + 224));
    const float32x4x2_t x13 = vsubq_x2_f32(vld1q_x2_f32(x + 104), vld1q_x2_f32(x + 232));
    const float32x4x2_t x14 = vsubq_x2_f32(vld1q_x2_f32(x + 112), vld1q_x2_f32(x + 240));
    const float32x4x2_t x15 = vsubq_x2_f32(vld1q_x2_f32(x + 120), vld1q_x2_f32(x + 248));

    fwht128_neon(X0, X1, X2, X3, X4, X5, X6, X7, X8, X9, X10, X11, X12, X13, X14, X15, y);
    fwht128_neon(x0, x1, x2, x3, x4, x5, x6, x7, x8, x9, x10, x11, x12, x13, x14, x15, y + 128);
}
//
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
#if 0
template <> inline void fwht_neon<8>(float x[], float y[]) {
    const float32x4x2_t C0 = vld1q_x2_f32(x);
    const float32x4x2_t D0 = fwht8_neon(C0.val[0], C0.val[1]);
    vst1q_x2_f32(y, D0);
}
#endif
//
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
#if 0
template <> inline void fwht_neon<8>(float x[]) {
    fwht_neon<8>(x, x);
}
#endif
template <> inline void fwht_neon<16>(float x[]) {
    fwht16_flat_neon(x, x);
}
#if 0
template <> inline void fwht_neon<16>(float x[], float y[]) {
    fwht16_flat_neon(x, y);
}
#endif
//
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
template <> inline void fwht_neon<32>(float x[]) {
    fwht32_flat_neon(x, x);
}
#if 0
template <> inline void fwht_neon<32>(float x[], float y[]) {
    fwht32_flat_neon(x, y);
}
#endif
//
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
template <> inline void fwht_neon<64>(float x[]) {
    fwht64_flat_neon(x, x);
}
#if 0
template <> inline void fwht_neon<64>(float x[], float y[]) {
    fwht64_flat_neon(x, y);
}
#endif
//
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
template <> inline void fwht_neon<128>(float x[]) {
    fwht128_flat_neon(x, x);
}
#if 0
template <> inline void fwht_neon<128>(float x[], float y[]) {
    fwht128_flat_neon(x, y);
}
#endif
//
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
template <> inline void fwht_neon<256>(float x[]) {
    fwht256_flat_neon(x, x);
}
#if 0
template <> inline void fwht_neon<256>(float x[], float y[]) {
    fwht256_flat_neon(x, y);
}
#endif
//
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
template <> inline void fwht_neon<512>(float x[])
{
    for (int i = 0; i < 256; i+= sizeof(float32x4_t)) {
        const float32x4_t A = vld1q_f32(x + i      );
        const float32x4_t B = vld1q_f32(x + i + 256);
        const float32x4_t C = vaddq_f32 (A, B);
        const float32x4_t D = vsubq_f32 (A, B);
        vst1q_f32(x + i +   0, C);
        vst1q_f32(x + i + 256, D);
    }
    fwht256_flat_neon(x +   0, x +   0);
    fwht256_flat_neon(x + 256, x + 256);
}
#if 0
template <> inline void fwht_neon<512>(float x[], float y[]) {
    for (int i = 0; i < 256; i+= sizeof(float32x4_t)) {
        const float32x4_t A = vld1q_f32(x + i      );
        const float32x4_t B = vld1q_f32(x + i + 256);
        const float32x4_t C = vaddq_f32 (A, B);
        const float32x4_t D = vsubq_f32 (A, B);
        vst1q_f32(x + i +   0, C);
        vst1q_f32(x + i + 256, D);
    }
    fwht256_flat_neon(x +   0, y +   0);
    fwht256_flat_neon(x + 256, y + 256);
}
#endif
//
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
template <> inline void fwht_neon<1024>(float x[]) {
    for (int i = 0; i < 512; i+= sizeof(float32x4_t)) {
        const float32x4_t A = vld1q_f32(x + i      );
        const float32x4_t B = vld1q_f32(x + i + 512);
        const float32x4_t C = vaddq_f32 (A, B);
        const float32x4_t D = vsubq_f32 (A, B);
        vst1q_f32(x + i +   0, C);
        vst1q_f32(x + i + 512, D);
    }
    fwht256_flat_neon(x +   0, x +   0);
    fwht256_flat_neon(x + 512, x + 512);
}
#if 0
template <> inline void fwht_neon<1024>(float x[], float y[]) {
    for (int i = 0; i < 512; i+= sizeof(float32x4_t)) {
        const float32x4_t A = vld1q_f32(x + i      );
        const float32x4_t B = vld1q_f32(x + i + 512);
        const float32x4_t C = vaddq_f32 (A, B);
        const float32x4_t D = vsubq_f32 (A, B);
        vst1q_f32(x + i +   0, C);
        vst1q_f32(x + i + 512, D);
    }
    fwht256_flat_neon(x +   0, y +   0);
    fwht256_flat_neon(x + 512, y + 512);
}
#endif
//
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
#endif
