/**
  Copyright (c) "Camille Monière"

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

#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#if defined(__ARM_NEON__) || defined(__ARM_NEON)
#include <arm_neon.h>
inline void fwht_size_8(float * __restrict outp, const float * __restrict inp) {
    const float32x4_t inp1 = vld1q_f32(inp    );
    const float32x4_t inp2 = vld1q_f32(inp + 4);

    const uint32x4_t m0 = {0x00000000, 0x00000000, 0x80000000, 0x80000000};
    const uint32x4_t m1 = {0x00000000, 0x80000000, 0x00000000, 0x80000000};
    //
    //////////////////////////////////////////////////////
    //
    const float32x4_t HH = vaddq_f32(inp1, inp2);
    const float32x4_t N0 = vextq_f32(HH, HH, 2);
    const float32x4_t N1 = vreinterpretq_f32_u32(veorq_u32(vreinterpretq_u32_f32(HH), m0));
    const float32x4_t N2 = vaddq_f32(N0, N1);
    const float32x4_t V0 = vreinterpretq_f32_u32(veorq_u32(vreinterpretq_u32_f32(N2), m1));
    const float32x4_t V1 = vrev64q_f32(N2);
    const float32x4_t V2 = vaddq_f32(V0, V1);
    vst1q_f32( outp, V2 );
    //
    //////////////////////////////////////////////////////
    //
    const float32x4_t BB = vsubq_f32(inp1, inp2);
    const float32x4_t O0 = vextq_f32(BB, BB, 2);
    const float32x4_t O1 = vreinterpretq_f32_u32(veorq_u32(vreinterpretq_u32_f32(BB), m0));
    const float32x4_t O2 = vaddq_f32(O0, O1);
    const float32x4_t Q0 = vreinterpretq_f32_u32(veorq_u32(vreinterpretq_u32_f32(O2), m1));
    const float32x4_t Q1 = vrev64q_f32(O2);
    const float32x4_t Q2 = vaddq_f32(Q0, Q1);
    vst1q_f32( outp + 4, Q2 );
}
#elif defined(__AVX2__)
#include <immintrin.h>

const __m256 M0 = _mm256_castsi256_ps(_mm256_setr_epi32(0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x80000000, 0x80000000, 0x80000000, 0x80000000));
const __m256 M1 = _mm256_castsi256_ps(_mm256_setr_epi32(0x00000000, 0x00000000, 0x80000000, 0x80000000, 0x00000000, 0x00000000, 0x80000000, 0x80000000));
const __m256 M2 = _mm256_castsi256_ps(_mm256_setr_epi32(0x00000000, 0x80000000, 0x00000000, 0x80000000, 0x00000000, 0x80000000, 0x00000000, 0x80000000));

inline void fwht_size_8(float * __restrict outp, const float * __restrict inp){
    const __m256 HAUT = _mm256_loadu_ps(inp);
    const __m256 N0   = _mm256_xor_ps(HAUT, M0);
    const __m256 N1   = _mm256_permute2f128_ps(HAUT, HAUT, 0x01);
    const __m256 N2   = _mm256_add_ps(N0, N1);
    const __m256 O0   = _mm256_xor_ps(N2, M1);
    const __m256 O1   = _mm256_shuffle_ps(N2, N2, 0x4E);
    const __m256 O2   = _mm256_add_ps(O0, O1);
    const __m256 P0   = _mm256_xor_ps(O2, M2);
    const __m256 P1   = _mm256_shuffle_ps(O2, O2, 0xB1);
    const __m256 P2   = _mm256_add_ps(P0, P1);
    _mm256_storeu_ps(outp + 0, P2);
}
#else
inline void fwht_size_8(float * __restrict outp, const float * __restrict inp) {
    float L1[8], L2[8];
    L1[0] = inp[0] + inp[4];
    L1[1] = inp[1] + inp[5];
    L1[2] = inp[2] + inp[6];
    L1[3] = inp[3] + inp[7];
    L1[4] = inp[0] - inp[4];
    L1[5] = inp[1] - inp[5];
    L1[6] = inp[2] - inp[6];
    L1[7] = inp[3] - inp[7];

    L2[0] = L1[0] + L1[2];
    L2[1] = L1[1] + L1[3];
    L2[2] = L1[0] - L1[2];
    L2[3] = L1[1] - L1[3];
    L2[4] = L1[4] + L1[6];
    L2[5] = L1[5] + L1[7];
    L2[6] = L1[4] - L1[6];
    L2[7] = L1[5] - L1[7];

    outp[0] = L2[0] + L2[1];
    outp[1] = L2[0] - L2[1];
    outp[2] = L2[2] + L2[3];
    outp[3] = L2[2] - L2[3];
    outp[4] = L2[4] + L2[5];
    outp[5] = L2[4] - L2[5];
    outp[6] = L2[6] + L2[7];
    outp[7] = L2[6] - L2[7];
}
#endif

template <size_t galois_size>
inline void fwht_template_spec8(float * dst, const float * src) {
    static_assert(galois_size > 8, "galois_size must be over 8.");
    constexpr size_t half_gf = galois_size >> 1;
    static_assert((half_gf << 1) == galois_size, "galois_size must be a power of 2.");
    float intermediary[galois_size];
    for (size_t j = 0; j < half_gf; j++) {
        intermediary[j] = src[j] + src[j + half_gf];
        intermediary[j + half_gf] = src[j] - src[j + half_gf];
    }
    fwht_template_spec8<half_gf>(dst, intermediary);
    fwht_template_spec8<half_gf>(dst + half_gf, intermediary + half_gf);
}

template <>
inline void fwht_template_spec8<8>(float * dst, const float * src) {
    fwht_size_8(dst, src);
}
