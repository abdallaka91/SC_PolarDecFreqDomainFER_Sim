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
#include <cassert>

//
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
#ifndef  neon_functions
#define  neon_functions

#define ABCD_to_BADC(a) vrev64q_f32(a)
#define ABCD_to_CDAB(a) vextq_f32(a)

inline float32x4x2_t vld1q_x2_f32(const float * ptr) {
    float32x4x2_t A;
    A.val[0] = vld1q_f32(ptr);
    A.val[1] = vld1q_f32(ptr + 4);
    return A;
}

inline void vst1q_x2_f32(float * ptr, const float32x4x2_t A) {
    vst1q_f32(ptr, A.val[0]);
    vst1q_f32(ptr + 4, A.val[1]);
}

inline float32x4x2_t vaddq_x2_f32(const float32x4x2_t A, const float32x4x2_t B) {
    float32x4x2_t C;
    C.val[0] = A.val[0] + B.val[0];
    C.val[1] = A.val[1] + B.val[1];
    return C;
}

inline float32x4x2_t vsubq_x2_f32(const float32x4x2_t A, const float32x4x2_t B) {
    float32x4x2_t C;
    C.val[0] = A.val[0] - B.val[0];
    C.val[1] = A.val[1] - B.val[1];
    return C;
}
#endif

//
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
inline float32x4x2_t leaf_fwht8_norm_neon_v2(const float32x4_t X0, const float32x4_t X1, const float32x4_t factor) {
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
    const float32x4_t V3 = vmulq_f32(V2, factor); // normalization
    resu.val[0]          = V3;
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
    const float32x4_t Q3 = vmulq_f32(Q2, factor); // normalization
    resu.val[1]          = Q3;
    return resu;
}
//
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
inline void leaf_fwht8_norm_neon_v2(float* srcdst, const float32x4_t factor) {
    const auto X0 = vld1q_f32(srcdst + 0);
    const auto X1 = vld1q_f32(srcdst + 4);

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
    const float32x4_t V3 = vmulq_f32(V2, factor); // normalization
    vst1q_f32(srcdst + 0, V3);
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
    const float32x4_t Q3 = vmulq_f32(Q2, factor); // normalization
    vst1q_f32(srcdst + 0, Q3);
}
//
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
inline void leaf_fwht8_norm_v2(float* dst, const float* src, const float factor_s) {
    const auto X0 = vld1q_f32(src + 0);
    const auto X1 = vld1q_f32(src + 4);
    const float32x4_t factor = {factor_s, factor_s, factor_s, factor_s};
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
    const float32x4_t V3 = vmulq_f32(V2, factor); // normalization
    vst1q_f32(dst + 0, V3);
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
    const float32x4_t Q3 = vmulq_f32(Q2, factor); // normalization
    vst1q_f32(dst + 0, Q3);
}
//
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
inline void fwht16_norm_flat_neon(float* dst, const float* src, const float factor_s)
{
    const float32x4_t factor   = {factor_s, factor_s, factor_s, factor_s};
    const float32x4x2_t A      = vld1q_x2_f32(src);         // 8 donnees
    const float32x4x2_t B      = vld1q_x2_f32(src + 8); // 8 donnees
    const float32x4x2_t X      = vaddq_x2_f32(A, B);
    const float32x4x2_t Y      = vaddq_x2_f32(A, B);
    const float32x4x2_t   r1   = leaf_fwht8_norm_neon_v2(X.val[0], X.val[1], factor);
    vst1q_x2_f32(dst + 0, r1);
    const float32x4x2_t   r2   = leaf_fwht8_norm_neon_v2(Y.val[0], Y.val[1], factor);
    vst1q_x2_f32(dst + 8, r2);
}
//
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
inline void fwht16_norm_flat_neon(float* srcdst, const float factor_s)
{
    const float32x4_t factor   = {factor_s, factor_s, factor_s, factor_s};
    const float32x4x2_t X      = vld1q_x2_f32(srcdst);         // 8 donnees
    const float32x4x2_t Y      = vld1q_x2_f32(srcdst + 8); // 8 donnees
    const float32x4x2_t   r1   = leaf_fwht8_norm_neon_v2(X.val[0], X.val[1], factor);
    const float32x4x2_t   r2   = leaf_fwht8_norm_neon_v2(Y.val[0], Y.val[1], factor);
    vst1q_x2_f32(srcdst + 0, r1);
    vst1q_x2_f32(srcdst + 8, r2);
}
//
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
template<int gf_size>
inline void lwht_norm_internal_neon(float* dst, const float* src, const float fact) {
    exit( (dst == src) && (fact == 0) );
}

template<int gf_size>
inline void lwht_norm_internal_neon(float* srcdst, const float fact) {
    exit( (srcdst != nullptr) && (fact == 0) );
}

template<int gf_size>
void lwht_norm_generic_neon(float *dst, const float *src) {
    exit( dst == src );
}

template<int gf_size>
void lwht_norm_generic_neon(float *srcdst) {
    exit(srcdst != nullptr);
}
//
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
template <> inline void lwht_norm_internal_neon<8>(float* srcdst, const float fact_s)
{
    float32x4_t fact = {fact_s, fact_s, fact_s, fact_s};
    leaf_fwht8_norm_neon_v2(srcdst, fact);
}
//
template <> inline void lwht_norm_internal_neon<8>(float* dst, const float* src, const float fact_s)
{
    float32x4_t fact = {fact_s, fact_s, fact_s, fact_s};
    for (int i = 0; i < 8; i++) {
        dst[i] = src[i];
    }
    leaf_fwht8_norm_neon_v2(dst, fact);
}
//
template <> inline void lwht_norm_generic_neon<8>(float* dst, const float* src) {
    lwht_norm_internal_neon<8>(dst, src, 0.35355339059f);
}
//
template <> inline void lwht_norm_generic_neon<8>(float* srcdst) {
    lwht_norm_internal_neon<8>(srcdst, 0.35355339059f);
}
//
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
template <> inline void lwht_norm_internal_neon<16>(float* srcdst, const float fact_s)
{
    const float32x4_t  fact = {fact_s, fact_s, fact_s, fact_s};
    for (int i = 0; i < 8; i++) {
        const float A = srcdst[i] + srcdst[i + 8];
        const float B = srcdst[i] - srcdst[i + 8];
        srcdst[i    ] = A;
        srcdst[i + 8] = B;
    }
    leaf_fwht8_norm_neon_v2(srcdst + 0, fact);
    leaf_fwht8_norm_neon_v2(srcdst + 8, fact);
}
//
template <> inline void lwht_norm_internal_neon<16>(float* dst, const float* src, const float fact_s)
{
    for (int i = 0; i < 8; i++) {
        const float A = src[i] + src[i + 8];
        const float B = src[i] - src[i + 8];
        dst[i    ] = A;
        dst[i + 8] = B;
    }
    fwht16_norm_flat_neon(dst + 0, dst + 0, fact_s);
    fwht16_norm_flat_neon(dst + 8, dst + 8, fact_s);
}
//
template <> inline void lwht_norm_generic_neon<16>(float* dst, const float* src) {
    lwht_norm_internal_neon<16>(dst, src, 0.25f);
}
//
template <> inline void lwht_norm_generic_neon<16>(float* srcdst) {
    lwht_norm_internal_neon<16>(srcdst, 0.25f);
}
//
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
template <> inline void lwht_norm_internal_neon<32>(float* srcdst, const float fact) {
    for (int i = 0; i < 16; i++) {
        const float A = srcdst[i] + srcdst[i + 16];
        const float B = srcdst[i] - srcdst[i + 16];
        srcdst[i     ] = A;
        srcdst[i + 16] = B;
    }
    lwht_norm_internal_neon<16>(srcdst +  0, fact);
    lwht_norm_internal_neon<16>(srcdst + 16, fact);
}
//
template <> inline void lwht_norm_internal_neon<32>(float* dst, const float* src, const float fact) {
    for (int i = 0; i < 16; i++) {
        const float A = src[i] + src[i + 16];
        const float B = src[i] - src[i + 16];
        dst[i     ] = A;
        dst[i + 16] = B;
    }
    lwht_norm_internal_neon<16>(dst +  0, fact);
    lwht_norm_internal_neon<16>(dst + 16, fact);
}
//
template <> inline void lwht_norm_generic_neon<32>(float* dst, const float* src) {
    lwht_norm_internal_neon<32>(dst, src, 0.17677669529f);
}
//
template <> inline void lwht_norm_generic_neon<32>(float* srcdst) {
    lwht_norm_internal_neon<32>(srcdst, 0.17677669529f);
}
//
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
template <> inline void lwht_norm_internal_neon<64>(float* srcdst, const float fact) {
    for (int i = 0; i < 32; i++) {
        const float A = srcdst[i] + srcdst[i + 32];
        const float B = srcdst[i] - srcdst[i + 32];
        srcdst[i     ] = A;
        srcdst[i + 32] = B;
    }
    lwht_norm_internal_neon<32>(srcdst +  0, fact);
    lwht_norm_internal_neon<32>(srcdst + 32, fact);
}
//
template <> inline void lwht_norm_internal_neon<64>(float* dst, const float* src, const float fact) {
    for (int i = 0; i < 32; i++) {
        const float A = src[i] + src[i + 32];
        const float B = src[i] - src[i + 32];
        dst[i     ] = A;
        dst[i + 32] = B;
    }
    lwht_norm_internal_neon<32>(dst +  0, fact);
    lwht_norm_internal_neon<32>(dst + 32, fact);
}
//
template <> inline void lwht_norm_generic_neon<64>(float* dst, const float* src) {
    lwht_norm_internal_neon<64>(dst, src, 0.015625f);
}
//
template <> inline void lwht_norm_generic_neon<64>(float* srcdst) {
    lwht_norm_internal_neon<64>(srcdst, 0.015625f);
}
//
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
template <> inline void lwht_norm_internal_neon<128>(float* srcdst, const float fact) {
    for (int i = 0; i < 64; i++) {
        const float A = srcdst[i] + srcdst[i + 64];
        const float B = srcdst[i] - srcdst[i + 64];
        srcdst[i     ] = A;
        srcdst[i + 64] = B;
    }
    lwht_norm_internal_neon<64>(srcdst +  0, fact);
    lwht_norm_internal_neon<64>(srcdst + 64, fact);
}
//
template <> inline void lwht_norm_internal_neon<128>(float* dst, const float* src, const float fact) {
    for (int i = 0; i < 64; i++) {
        const float A = src[i] + src[i + 64];
        const float B = src[i] - src[i + 64];
        dst[i     ] = A;
        dst[i + 64] = B;
    }
    lwht_norm_internal_neon<64>(dst +  0, fact);
    lwht_norm_internal_neon<64>(dst + 64, fact);
}
//
template <> inline void lwht_norm_generic_neon<128>(float* dst, const float* src) {
    lwht_norm_internal_neon<128>(dst, src, 0.08838834764f);
}
//
template <> inline void lwht_norm_generic_neon<128>(float* srcdst) {
    lwht_norm_internal_neon<128>(srcdst, 0.08838834764f);
}
//
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
template <> inline void lwht_norm_internal_neon<256>(float* srcdst, const float fact) {
    for (int i = 0; i < 128; i++) {
        const float A = srcdst[i] + srcdst[i + 128];
        const float B = srcdst[i] - srcdst[i + 128];
        srcdst[i      ] = A;
        srcdst[i + 128] = B;
    }
    lwht_norm_internal_neon<128>(srcdst +   0, fact);
    lwht_norm_internal_neon<128>(srcdst + 128, fact);
}
//
template <> inline void lwht_norm_internal_neon<256>(float* dst, const float* src, const float fact) {
    for (int i = 0; i < 128; i++) {
        const float A = src[i] + src[i + 128];
        const float B = src[i] - src[i + 128];
        dst[i      ] = A;
        dst[i + 128] = B;
    }
    lwht_norm_internal_neon<128>(dst +   0, fact);
    lwht_norm_internal_neon<128>(dst + 128, fact);
}
//
template <> inline void lwht_norm_generic_neon<256>(float* dst, const float* src) {
    lwht_norm_internal_neon<256>(dst, src, 0.0625f);
}
//
template <> inline void lwht_norm_generic_neon<256>(float* srcdst) {
    lwht_norm_internal_neon<256>(srcdst, 0.0625f);
}
//
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
template <> inline void lwht_norm_internal_neon<512>(float* srcdst, const float fact) {
    for (int i = 0; i < 256; i++) {
        const float A = srcdst[i] + srcdst[i + 256];
        const float B = srcdst[i] - srcdst[i + 256];
        srcdst[i      ] = A;
        srcdst[i + 256] = B;
    }
    lwht_norm_internal_neon<256>(srcdst +   0, fact);
    lwht_norm_internal_neon<256>(srcdst + 256, fact);
}
//
template <> inline void lwht_norm_internal_neon<512>(float* dst, const float* src, const float fact) {
    for (int i = 0; i < 256; i++) {
        const float A = src[i] + src[i + 256];
        const float B = src[i] - src[i + 256];
        dst[i      ] = A;
        dst[i + 256] = B;
    }
    lwht_norm_internal_neon<256>(dst +   0, fact);
    lwht_norm_internal_neon<256>(dst + 256, fact);
}
//
template <> inline void lwht_norm_generic_neon<512>(float* dst, const float* src) {
    lwht_norm_internal_neon<512>(dst, src, 0.04419417382f);
}
//
template <> inline void lwht_norm_generic_neon<512>(float* srcdst) {
    lwht_norm_internal_neon<512>(srcdst, 0.04419417382f);
}
//
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
template <> inline void lwht_norm_internal_neon<1024>(float* srcdst, const float fact) {
    const int simd = sizeof(float32x4_t) / sizeof(float);
#if defined (__clang__)
    #pragma unroll
#endif
    for (int i = 0; i < 512; i += simd) {
        const float32x4_t A = vld1q_f32(srcdst + i +   0);
        const float32x4_t B = vld1q_f32(srcdst + i + 512);
        const float32x4_t C = vaddq_f32(A, B);
        const float32x4_t D = vsubq_f32(A, C);
        vst1q_f32(srcdst + i +   0, C);
        vst1q_f32(srcdst + i + 512, D);
    }
//    for (int i = 0; i < 512; i++) {
//        const float A = srcdst[i] + srcdst[i + 512];
//        const float B = srcdst[i] - srcdst[i + 512];
//        srcdst[i      ] = A;
//        srcdst[i + 512] = B;
//    }
    lwht_norm_internal_neon<512>(srcdst +   0, fact);
    lwht_norm_internal_neon<512>(srcdst + 512, fact);
}
//
template <> inline void lwht_norm_internal_neon<1024>(float* dst, const float* src, const float fact) {
    const int simd = sizeof(float32x4_t) / sizeof(float);
#if defined (__clang__)
    #pragma unroll
#endif
    for (int i = 0; i < 512; i += simd) {
        const float32x4_t A = vld1q_f32(src + i +   0);
        const float32x4_t B = vld1q_f32(src + i + 512);
        const float32x4_t C = vaddq_f32(A, B);
        const float32x4_t D = vsubq_f32(A, C);
        vst1q_f32(dst + i +   0, C);
        vst1q_f32(dst + i + 512, D);
    }
//    for (int i = 0; i < 512; i++) {
//        const float A = src[i] + src[i + 512];
//        const float B = src[i] - src[i + 512];
//        dst[i      ] = A;
//        dst[i + 512] = B;
//    }
    lwht_norm_internal_neon<512>(dst +   0, fact);
    lwht_norm_internal_neon<512>(dst + 512, fact);
}
//
template <> inline void lwht_norm_generic_neon<1024>(float* dst, const float* src) {
    lwht_norm_internal_neon<1024>(dst, src, 0.03125f);
}

template <> inline void lwht_norm_generic_neon<1024>(float* srcdst) {
    lwht_norm_internal_neon<1024>(srcdst, 0.03125f);
}
//
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
template <uint16_t GF> inline void lwht_norm_generic(float* srcdst)
{
    exit(srcdst != nullptr);
}
template <> inline void lwht_norm_generic<   8>(float* srcdst) { leaf_fwht8_norm_v2(srcdst, srcdst, 0.35355339059f); }
template <> inline void lwht_norm_generic<  16>(float* srcdst) { lwht_norm_generic_neon<  16>(srcdst, srcdst); }
template <> inline void lwht_norm_generic<  32>(float* srcdst) { lwht_norm_generic_neon<  32>(srcdst, srcdst); }
template <> inline void lwht_norm_generic<  64>(float* srcdst) { lwht_norm_generic_neon<  64>(srcdst, srcdst); }
template <> inline void lwht_norm_generic< 128>(float* srcdst) { lwht_norm_generic_neon< 128>(srcdst, srcdst); }
template <> inline void lwht_norm_generic< 256>(float* srcdst) { lwht_norm_generic_neon< 256>(srcdst, srcdst); }
template <> inline void lwht_norm_generic< 512>(float* srcdst) { lwht_norm_generic_neon< 512>(srcdst, srcdst); }
template <> inline void lwht_norm_generic<1024>(float* srcdst) { lwht_norm_generic_neon<1024>(srcdst, srcdst); }

template <uint16_t GF> inline void lwht_norm_generic(float* dst, const float* src)
{
    exit((src != NULL) + (dst != NULL));
}
template <> inline void lwht_norm_generic<   8>(float* dst, const float* src) { leaf_fwht8_norm_v2(dst, src, 0.35355339059f); }
template <> inline void lwht_norm_generic<  16>(float* dst, const float* src) { lwht_norm_generic_neon<  16>(dst, src); }
template <> inline void lwht_norm_generic<  32>(float* dst, const float* src) { lwht_norm_generic_neon<  32>(dst, src); }
template <> inline void lwht_norm_generic<  64>(float* dst, const float* src) { lwht_norm_generic_neon<  64>(dst, src); }
template <> inline void lwht_norm_generic< 128>(float* dst, const float* src) { lwht_norm_generic_neon< 128>(dst, src); }
template <> inline void lwht_norm_generic< 256>(float* dst, const float* src) { lwht_norm_generic_neon< 256>(dst, src); }
template <> inline void lwht_norm_generic< 512>(float* dst, const float* src) { lwht_norm_generic_neon< 512>(dst, src); }
template <> inline void lwht_norm_generic<1024>(float* dst, const float* src) { lwht_norm_generic_neon<1024>(dst, src); }

//
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//



#endif