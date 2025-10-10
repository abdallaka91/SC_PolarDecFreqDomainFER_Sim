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
#include <cstring>

#include "const_config_GF64_N64.hpp"

//
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
template <uint16_t galois_size>
inline void fwht_norm(float* x) {
    assert(x != 0);
    assert(true);
    exit(x != NULL); // pour gerer le release mode
}
template <uint16_t galois_size>
inline void fwht_norm_internal(float* x, const float fact) {
    assert(x    != nullptr);
    assert(fact != 0.f);
    assert(true);
    exit( (x != nullptr) && (fact != 0.f)); // pour gerer le release mode
}
//
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
inline void fwht_norm_tuile(float* inp,  const float fact) {
    float L1[8], L2[8];
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

    inp[0] = (L2[0] + L2[1]) * fact;
    inp[1] = (L2[0] - L2[1]) * fact;
    inp[2] = (L2[2] + L2[3]) * fact;
    inp[3] = (L2[2] - L2[3]) * fact;
    inp[4] = (L2[4] + L2[5]) * fact;
    inp[5] = (L2[4] - L2[5]) * fact;
    inp[6] = (L2[6] + L2[7]) * fact;
    inp[7] = (L2[6] - L2[7]) * fact;
}
//
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
template <> inline void fwht_norm_internal<8>(float* inp, const float fact) {
//    for (int i = 0; i < 4; i++) {
//
//    }
//        part_1[i] = inp[i] + inp[i + 4];
    fwht_norm_tuile(inp, fact);
}
//
template <> inline void fwht_norm<8>(float* inp) {
    fwht_norm_internal<8>(inp, 0.35355339059f);
}
//
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
template <> inline void fwht_norm_internal<16>(float* inp, const float fact) {

    for (int i = 0; i < 8; i++) {
        const float A = inp[i] + inp[i + 8];
        const float B = inp[i] - inp[i + 8];
        inp[i    ] = A;
        inp[i + 8] = B;
    }

    fwht_norm_tuile(inp + 0, fact);
    fwht_norm_tuile(inp + 8, fact);
}
//
template <> inline void fwht_norm<16>(float* inp) {
    fwht_norm_internal<16>(inp, 0.25f);
}
//
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
template <> inline void fwht_norm_internal<32>(float* inp, const float fact) {

    for (int i = 0; i < 16; i++) {
        const float A = inp[i] + inp[i + 16];
        const float B = inp[i] - inp[i + 16];
        inp[i     ] = A;
        inp[i + 16] = B;
    }
    fwht_norm_internal<16>(inp,      fact);
    fwht_norm_internal<16>(inp + 16, fact);
}
//
template <>
inline void fwht_norm<32>(float* inp) {
    fwht_norm_internal<32>(inp, 0.17677669529f);
}
//
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
template <>
inline void fwht_norm_internal<64>(float* inp, const float fact) {
    for (int i = 0; i < 32; i++) {
        const float A = inp[i] + inp[i + 32];
        const float B = inp[i] - inp[i + 32];
        inp[i     ] = A;
        inp[i + 32] = B;
    }
    fwht_norm_internal<32>(inp,      fact);
    fwht_norm_internal<32>(inp + 32, fact);
}
//
template <>
inline void fwht_norm<64>(float* inp) {
    fwht_norm_internal<64>(inp, 0.125f);
}
//
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
template <>
inline void fwht_norm_internal<128>(float* inp, const float fact)
{
    for (int i = 0; i < 64; i++) {
        const float A = inp[i] + inp[i + 64];
        const float B = inp[i] - inp[i + 64];
        inp[i     ] = A;
        inp[i + 64] = B;
    }
    fwht_norm_internal<64>(inp +  0, fact);
    fwht_norm_internal<64>(inp + 64, fact);
}
//
template <>
inline void fwht_norm<128>(float* inp) {
    fwht_norm_internal<128>(inp, 0.08838834764f);
}
//
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
template <>
inline void fwht_norm_internal<256>(float* inp, const float fact) {
    for (int i = 0; i < 128; i++) {
        const float A = inp[i] + inp[i + 128];
        const float B = inp[i] - inp[i + 128];
        inp[i      ] = A;
        inp[i + 128] = B;
    }
    fwht_norm_internal<128>(inp,       fact);
    fwht_norm_internal<128>(inp + 128, fact);
}
//
template <>
inline void fwht_norm<256>(float* inp) {
    fwht_norm_internal<256>(inp, 0.0625f);
}
//
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
template <>
inline void fwht_norm_internal<512>(float* inp, const float fact) {
    for (int i = 0; i < 256; i++) {
        const float A = inp[i] + inp[i + 256];
        const float B = inp[i] - inp[i + 256];
        inp[i      ] = A;
        inp[i + 256] = B;
    }
    fwht_norm_internal<256>(inp +   0, fact);
    fwht_norm_internal<256>(inp + 256, fact);
}
//
template <>
inline void fwht_norm<512>(float* inp) {
    fwht_norm_internal<512>(inp, 0.04419417382f);
}
//
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
template <>
inline void fwht_norm_internal<1024>(float* inp, const float fact) {
    for (int i = 0; i < 512; i++) {
        const float A = inp[i] + inp[i + 512];
        const float B = inp[i] - inp[i + 512];
        inp[i      ] = A;
        inp[i + 512] = B;
    }
    fwht_norm_internal<512>(inp +   0, fact);
    fwht_norm_internal<512>(inp + 512, fact);
}
//
template <>
inline void fwht_norm<1024>(float* inp) {
    fwht_norm_internal<1024>(inp, 0.03125f);
}
//
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
template <>
inline void fwht_norm_internal<2048>(float* inp, const float fact) {
    for (int i = 0; i < 1024; i++) {
        const float A = inp[i] + inp[i + 1024];
        const float B = inp[i] - inp[i + 1024];
        inp[i       ] = A;
        inp[i + 1024] = B;
    }
    fwht_norm_internal<1024>(inp +    0, fact);
    fwht_norm_internal<1024>(inp + 1024, fact);
}
//
template <>
inline void fwht_norm<2048>(float* inp) {
    fwht_norm_internal<2048>(inp, 0.02209708691f);
}
//
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
template <>
inline void fwht_norm_internal<4096>(float* inp, const float fact) {
    for (int i = 0; i < 2048; i++) {
        const float A = inp[i] + inp[i + 2048];
        const float B = inp[i] - inp[i + 2048];
        inp[i       ] = A;
        inp[i + 2048] = B;
    }
    fwht_norm_internal<2048>(inp +    0, fact);
    fwht_norm_internal<2048>(inp + 2048, fact);
}
//
template <>
inline void fwht_norm<4096>(float* inp) {
    fwht_norm_internal<4096>(inp, 0.015625f);
}
//
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
