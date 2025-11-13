/*
 *	Optimized bit-packing and bit-unpacking functions - Copyright (c) 2021 Bertrand LE GAL
 *
 *  This software is provided 'as-is', without any express or
 *  implied warranty. In no event will the authors be held
 *  liable for any damages arising from the use of this software.
 *
 *  Permission is granted to anyone to use this software for any purpose,
 *  including commercial applications, and to alter it and redistribute
 *  it freely, subject to the following restrictions:
 *
 *  1. The origin of this software must not be misrepresented;
 *  you must not claim that you wrote the original software.
 *  If you use this software in a product, an acknowledgment
 *  in the product documentation would be appreciated but
 *  is not required.
 *
 *  2. Altered source versions must be plainly marked as such,
 *  and must not be misrepresented as being the original software.
 *
 *  3. This notice may not be removed or altered from any
 *  source distribution.
 *
 */

#ifndef _Tools_
#define _Tools_

#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <iostream>

#ifdef __SSE4_2__
#include <immintrin.h>
#endif

using namespace std;

#ifdef __AVX2__
inline void uint8_t_show(const __m256i reg) {
    uint8_t tmp[sizeof(__m256i)];
    _mm256_storeu_si256((__m256i *) tmp, reg);
    for (uint32_t i = 0; i < sizeof(__m256i); i++)
        printf("%3d ", tmp[sizeof(__m256i) - 1 - i]);
    printf("\n");
}
#endif

#ifdef __SSE4_2__
inline void uint8_t_show(const __m128i reg) {
    uint8_t tmp[sizeof(__m128i)];
    _mm_storeu_si128((__m128i *) tmp, reg);
    for (uint32_t i = 0; i < sizeof(__m128i); i++) {
        if ((i % 4 == 0) && (i != 0))
            printf("| ");
        printf("%3d ", tmp[sizeof(__m128i) - 1 - i]);
    }
    printf("\n");
}
#endif

inline void int8_t_show(const int8_t * ptr) {
    assert(ptr != nullptr);
    for (uint32_t i = 0; i < 32; i++)
        printf("%4d ", ptr[i]);
    printf("\n");
}

inline void uint8_t_show(const uint8_t * ptr) {
    assert(ptr != nullptr);
    for (uint32_t i = 0; i < 32; i++) {
        if ((i % 16 == 0) && (i != 0))
            printf("\n");
        printf("%4d ", ptr[i]);
    }
    printf("\n");
}

inline void int8_t_show(const int8_t * ptr, const int32_t length) {
    assert(ptr != nullptr);
    assert(length != 0);
    for (int32_t i = 0; i < length; i += 32)
        int8_t_show(ptr + i);
    printf("\n");
}

inline void uint8_t_show(const uint8_t * ptr, const int32_t length) {
    assert(ptr != nullptr);
    assert(length != 0);
    for (int32_t i = 0; i < length; i += 32)
        uint8_t_show(ptr + i);
    printf("\n");
}

#endif