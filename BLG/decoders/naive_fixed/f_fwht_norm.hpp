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
#include "f_type.hpp"

//
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
template <uint16_t galois_size>
inline float norm_factor_lwht()
{
    if (galois_size == 8) {
        return 0.35355339059f;
    }else if (galois_size == 16) {
        return 0.25f;
    }else if (galois_size == 32) {
        return 0.17677669529f;
    }else if (galois_size == 64) {
        return 0.125f;
    }else if (galois_size == 128) {
        return 0.08838834764f;
    }else if (galois_size == 256) {
        return 0.0625f;
    }else if (galois_size == 512) {
        return 0.04419417382f;
    }else if (galois_size == 1024) {
        return 0.03125f;
    }else if (galois_size == 2048) {
        return 0.02209708691f;
    }else if (galois_size == 4096) {
        return 0.015625f;
    }else {
        printf("Error !\n");
        return -1;
    }
}
//
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
template <uint16_t galois_size>
inline void fwht_norm(ap_fixed<NBITS, NINTG>* x) {
    assert(x != 0);
    assert(true);
    exit(x != NULL); // pour gerer le release mode
}
template <uint16_t galois_size>
inline void fwht_norm_internal(ap_fixed<NBITS, NINTG>* x, const ap_fixed<NBITS, NINTG> fact) {
    assert(x    != nullptr);
    assert(fact != (ap_fixed<NBITS, NINTG>)0.f);
    assert(true);
    const ap_fixed<NBITS, NINTG> zero = 0.f;
    exit( (x != nullptr) && (fact != zero)); // pour gerer le release mode
}
//
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
inline void fwht_norm_tuile(ap_fixed<NBITS, NINTG>* inp,  const ap_fixed<NBITS, NINTG> fact) {
    ap_fixed<NBITS, NINTG> L1[8], L2[8];
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
template <> inline void fwht_norm_internal<8>(ap_fixed<NBITS, NINTG>* inp, const ap_fixed<NBITS, NINTG> fact) {
//    for (int i = 0; i < 4; i++) {
//
//    }
//        part_1[i] = inp[i] + inp[i + 4];
    fwht_norm_tuile(inp, fact);
}
//
template <> inline void fwht_norm<8>(ap_fixed<NBITS, NINTG>* inp) {
    fwht_norm_internal<8>(inp, 0.35355339059f);
}
//
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
template <> inline void fwht_norm_internal<16>(ap_fixed<NBITS, NINTG>* inp, const ap_fixed<NBITS, NINTG> fact) {

    for (int i = 0; i < 8; i++) {
        const ap_fixed<NBITS, NINTG> A = inp[i] + inp[i + 8];
        const ap_fixed<NBITS, NINTG> B = inp[i] - inp[i + 8];
        inp[i    ] = A;
        inp[i + 8] = B;
    }

    fwht_norm_tuile(inp + 0, fact);
    fwht_norm_tuile(inp + 8, fact);
}
//
template <> inline void fwht_norm<16>(ap_fixed<NBITS, NINTG>* inp) {
    fwht_norm_internal<16>(inp, 0.25f);
}
//
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
template <> inline void fwht_norm_internal<32>(ap_fixed<NBITS, NINTG>* inp, const ap_fixed<NBITS, NINTG> fact) {

    for (int i = 0; i < 16; i++) {
        const ap_fixed<NBITS, NINTG> A = inp[i] + inp[i + 16];
        const ap_fixed<NBITS, NINTG> B = inp[i] - inp[i + 16];
        inp[i     ] = A;
        inp[i + 16] = B;
    }
    fwht_norm_internal<16>(inp,      fact);
    fwht_norm_internal<16>(inp + 16, fact);
}
//
template <>
inline void fwht_norm<32>(ap_fixed<NBITS, NINTG>* inp) {
    fwht_norm_internal<32>(inp, 0.17677669529f);
}
//
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
template <>
inline void fwht_norm_internal<64>(ap_fixed<NBITS, NINTG>* inp, const ap_fixed<NBITS, NINTG> fact) {
    for (int i = 0; i < 32; i++) {
        const ap_fixed<NBITS, NINTG> A = inp[i] + inp[i + 32];
        const ap_fixed<NBITS, NINTG> B = inp[i] - inp[i + 32];
        inp[i     ] = A;
        inp[i + 32] = B;
    }
    fwht_norm_internal<32>(inp,      fact);
    fwht_norm_internal<32>(inp + 32, fact);
}
//
template <>
inline void fwht_norm<64>(ap_fixed<NBITS, NINTG>* inp) {
    fwht_norm_internal<64>(inp, 0.125f);
}
//
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
template <>
inline void fwht_norm_internal<128>(ap_fixed<NBITS, NINTG>* inp, const ap_fixed<NBITS, NINTG> fact)
{
    for (int i = 0; i < 64; i++) {
        const ap_fixed<NBITS, NINTG> A = inp[i] + inp[i + 64];
        const ap_fixed<NBITS, NINTG> B = inp[i] - inp[i + 64];
        inp[i     ] = A;
        inp[i + 64] = B;
    }
    fwht_norm_internal<64>(inp +  0, fact);
    fwht_norm_internal<64>(inp + 64, fact);
}
//
template <>
inline void fwht_norm<128>(ap_fixed<NBITS, NINTG>* inp) {
    fwht_norm_internal<128>(inp, 0.08838834764f);
}
//
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
template <>
inline void fwht_norm_internal<256>(ap_fixed<NBITS, NINTG>* inp, const ap_fixed<NBITS, NINTG> fact) {
    for (int i = 0; i < 128; i++) {
        const ap_fixed<NBITS, NINTG> A = inp[i] + inp[i + 128];
        const ap_fixed<NBITS, NINTG> B = inp[i] - inp[i + 128];
        inp[i      ] = A;
        inp[i + 128] = B;
    }
    fwht_norm_internal<128>(inp,       fact);
    fwht_norm_internal<128>(inp + 128, fact);
}
//
template <>
inline void fwht_norm<256>(ap_fixed<NBITS, NINTG>* inp) {
    fwht_norm_internal<256>(inp, 0.0625f);
}
//
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
template <>
inline void fwht_norm_internal<512>(ap_fixed<NBITS, NINTG>* inp, const ap_fixed<NBITS, NINTG> fact) {
    for (int i = 0; i < 256; i++) {
        const ap_fixed<NBITS, NINTG> A = inp[i] + inp[i + 256];
        const ap_fixed<NBITS, NINTG> B = inp[i] - inp[i + 256];
        inp[i      ] = A;
        inp[i + 256] = B;
    }
    fwht_norm_internal<256>(inp +   0, fact);
    fwht_norm_internal<256>(inp + 256, fact);
}
//
template <>
inline void fwht_norm<512>(ap_fixed<NBITS, NINTG>* inp) {
    fwht_norm_internal<512>(inp, 0.04419417382f);
}
//
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
template <>
inline void fwht_norm_internal<1024>(ap_fixed<NBITS, NINTG>* inp, const ap_fixed<NBITS, NINTG> fact) {
    for (int i = 0; i < 512; i++) {
        const ap_fixed<NBITS, NINTG> A = inp[i] + inp[i + 512];
        const ap_fixed<NBITS, NINTG> B = inp[i] - inp[i + 512];
        inp[i      ] = A;
        inp[i + 512] = B;
    }
    fwht_norm_internal<512>(inp +   0, fact);
    fwht_norm_internal<512>(inp + 512, fact);
}
//
template <>
inline void fwht_norm<1024>(ap_fixed<NBITS, NINTG>* inp) {
    fwht_norm_internal<1024>(inp, 0.03125f);
}
//
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
template <>
inline void fwht_norm_internal<2048>(ap_fixed<NBITS, NINTG>* inp, const ap_fixed<NBITS, NINTG> fact) {
    for (int i = 0; i < 1024; i++) {
        const ap_fixed<NBITS, NINTG> A = inp[i] + inp[i + 1024];
        const ap_fixed<NBITS, NINTG> B = inp[i] - inp[i + 1024];
        inp[i       ] = A;
        inp[i + 1024] = B;
    }
    fwht_norm_internal<1024>(inp +    0, fact);
    fwht_norm_internal<1024>(inp + 1024, fact);
}
//
template <>
inline void fwht_norm<2048>(ap_fixed<NBITS, NINTG>* inp) {
    fwht_norm_internal<2048>(inp, 0.02209708691f);
}
//
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
template <>
inline void fwht_norm_internal<4096>(ap_fixed<NBITS, NINTG>* inp, const ap_fixed<NBITS, NINTG> fact) {
    for (int i = 0; i < 2048; i++) {
        const ap_fixed<NBITS, NINTG> A = inp[i] + inp[i + 2048];
        const ap_fixed<NBITS, NINTG> B = inp[i] - inp[i + 2048];
        inp[i       ] = A;
        inp[i + 2048] = B;
    }
    fwht_norm_internal<2048>(inp +    0, fact);
    fwht_norm_internal<2048>(inp + 2048, fact);
}
//
template <>
inline void fwht_norm<4096>(ap_fixed<NBITS, NINTG>* inp) {
    fwht_norm_internal<4096>(inp, 0.015625f);
}
//
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
