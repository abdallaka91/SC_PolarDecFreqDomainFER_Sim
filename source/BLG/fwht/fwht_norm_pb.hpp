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
#include <cassert>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>

#include "definitions/const_config_GF64_N64.hpp"

//
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
template <uint16_t GF> inline void fwht_2norm(value_type x[]) {
    assert(x != 0);
    assert(true);
    exit(x != NULL); // pour gerer le release mode
}
template <uint16_t GF> inline float fwht_2norm_internal(value_type x[], const float fact) {
    assert(x    != nullptr);
    assert(fact != 0.f);
    assert(true);
    exit(x != NULL); // pour gerer le release mode
    return 0.f;
}
//
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
inline float fwht_2norm_tuile(const value_type* inp, value_type* outp, const float fact) {
    value_type L1[8], L2[8];
    L1[0] = (inp[0] + inp[4]);
    L1[1] = (inp[1] + inp[5]);
    L1[2] = (inp[2] + inp[6]);
    L1[3] = (inp[3] + inp[7]);
    L1[4] = (inp[0] - inp[4]);
    L1[5] = (inp[1] - inp[5]);
    L1[6] = (inp[2] - inp[6]);
    L1[7] = (inp[3] - inp[7]);

    L2[0] = (L1[0] + L1[2]);
    L2[2] = (L1[0] - L1[2]);
    L2[1] = (L1[1] + L1[3]);
    L2[3] = (L1[1] - L1[3]);
    L2[4] = (L1[4] + L1[6]);
    L2[6] = (L1[4] - L1[6]);
    L2[5] = (L1[5] + L1[7]);
    L2[7] = (L1[5] - L1[7]);

    const float v0 = (L2[0] + L2[1]) * fact;
    const float v1 = (L2[0] - L2[1]) * fact;
    const float v2 = (L2[2] + L2[3]) * fact;
    const float v3 = (L2[2] - L2[3]) * fact;
    const float v4 = (L2[4] + L2[5]) * fact;
    const float v5 = (L2[4] - L2[5]) * fact;
    const float v6 = (L2[6] + L2[7]) * fact;
    const float v7 = (L2[6] - L2[7]) * fact;

    outp[0] = v0;
    outp[1] = v1;
    outp[2] = v2;
    outp[3] = v3;
    outp[4] = v4;
    outp[5] = v5;
    outp[6] = v6;
    outp[7] = v7;
    return v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7;
}
//
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
template <> inline float fwht_2norm_internal<8>(value_type* inp, const float fact) {
    value_type part_1[8];
    for (int i = 0; i < 4; i++)
        part_1[i] = inp[i] + inp[i + 4];
    return fwht_2norm_tuile(part_1, inp, fact);
}
//
template <> inline void fwht_2norm<8>(value_type* inp) {
    fwht_2norm_internal<8>(inp, 0.35355339059f);
}
//
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
template <>
inline float fwht_2norm_internal<16>(value_type* inp, const float fact) {
    value_type part_1[8];
    value_type part_2[8];

    for (int i = 0; i < 8; i++) part_1[i] = inp[i] + inp[i + 8];
    for (int i = 0; i < 8; i++) part_2[i] = inp[i] - inp[i + 8];
    const float v1 = fwht_2norm_tuile(part_1, inp + 0, fact);
    const float v2 = fwht_2norm_tuile(part_2, inp + 8, fact);
    return (v1 + v2);
}
//
template <> inline void fwht_2norm<16>(value_type* inp) {
    const float v  = fwht_2norm_internal<16>(inp, 0.25f);
    const float inv = 1.f / v;
    for (int i = 0; i < 16; i++)
        inp[i] *= inv;
}
//
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
template <>
inline float fwht_2norm_internal<32>(value_type* inp, const float fact) {
    value_type part_1[16];
    value_type part_2[16];

    for (int i = 0; i < 16; i++) {
        part_1[i] = inp[i] + inp[i + 16];
        part_2[i] = inp[i] - inp[i + 16];
    }

    const float v1  = fwht_2norm_internal<16>(part_1, fact);
    const float v2  = fwht_2norm_internal<16>(part_2, fact);
    const float inv = 1.f / (v1 + v2);

    for (int i = 0; i < 16; i++) {
        inp[     i] = part_1[i] * inv;
        inp[16 + i] = part_2[i] * inv;
    }
}
//
template <>
inline void fwht_2norm<32>(value_type* inp) {
    fwht_2norm_internal<32>(inp, 0.17677669529f);
}
//
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
template <>
inline void fwht_2norm_internal<64>(value_type* inp, const float fact) {
    value_type part_1[32];
    value_type part_2[32];

    for (int i = 0; i < 32; i++) {
        part_1[i] = inp[i] + inp[i + 32];
        part_2[i] = inp[i] - inp[i + 32];
    }

    const float v1 = fwht_2norm_internal<32>(part_1, fact);
    const float v2 = fwht_2norm_internal<32>(part_2, fact);
    const float inv = 1.f / (v1 + v2);
    for (int i = 0; i < 32; i++) {
        inp[i]      = part_1[i] * inv;
        inp[32 + i] = part_2[i] * inv;
    }
}
//
template <>
inline void fwht_2norm<64>(value_type* inp) {
    fwht_2norm_internal<64>(inp, 0.125f);
}
//
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
template <>
inline void fwht_2norm_internal<128>(value_type* inp, const float fact) {
    value_type part_1[64], part_2[64];

    for (int i = 0; i < 64; i++) {
        part_1[i] = inp[i] + inp[i + 64];
        part_2[i] = inp[i] - inp[i + 64];
    }

    const float v1 = fwht_2norm_internal<64>(part_1, fact);
    const float v2 = fwht_2norm_internal<64>(part_2, fact);
    const float inv = 1.f / (v1 + v2);
    for (int i = 0; i < 64; i++) {
        inp[i + 0]  = part_1[i] * inv;
        inp[i + 64] = part_2[i] * inv;
    }
}
//
template <>
inline void fwht_2norm<128>(value_type* inp) {
    fwht_2norm_internal<128>(inp, 0.08838834764f);
}
//
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
template <>
inline void fwht_2norm_internal<256>(value_type* inp, const float fact) {
    value_type part_1[128];
    value_type part_2[128];

    for (int i = 0; i < 128; i++) {
        part_1[i] = inp[i] + inp[i + 128];
        part_2[i] = inp[i] - inp[i + 128];
    }

    const float v1  = fwht_2norm_internal<128>(part_1, fact);
    const float v2  = fwht_2norm_internal<128>(part_2, fact);
    const float inv = 1.f / (v1 + v2);
    for (int i = 0; i < 128; i++) {
        inp[i +   0] = part_1[i] * inv;
        inp[i + 128] = part_2[i] * inv;
    }
}
//
template <>
inline void fwht_2norm<256>(value_type* inp) {
    fwht_2norm_internal<256>(inp, 0.0625f);
}
//
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
