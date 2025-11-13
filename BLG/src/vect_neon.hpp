/**
  Copyright (c) 2012-2020 "Bordeaux INP, Bertrand LE GAL"
  bertrand.legal@ims-bordeaux.fr
  [http://legal.vvv.enseirb-matmeca.fr]

  This file is part of a LDPC library for realtime LDPC decoding
  on processor cores.
*/

#ifndef _vect_fixed_intra_
#define _vect_fixed_intra_

/* Paste this on the file you want to debug. */
#ifndef _print_trace_
#define _print_trace_
#include "sse2neon.h"
#include <execinfo.h>
#include <stdio.h>

inline void print_trace() {
    char ** strings;
    size_t  i, size;
    enum Constexpr { MAX_SIZE = 1024 };
    void * array[MAX_SIZE];
    size    = backtrace(array, MAX_SIZE);
    strings = backtrace_symbols(array, size);
    for (i = 0; i < size; i++)
        printf("%s\n", strings[i]);
    puts("");
    free(strings);
}
#endif
/* Paste this on the file you want to debug. */

#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string.h>

using namespace std;

#if defined(__SSE4_2__) || defined(__AVX2__) || defined(__AVX512F__)
#include <immintrin.h>
#elif defined(__ARM_NEON__) || defined(__ARM_NEON)
#include <arm_neon.h>
#endif

#define perror                                                            \
    {                                                                     \
        printf("(EE) Error in file %s :: line %d\n", __FILE__, __LINE__); \
        exit(EXIT_FAILURE);                                               \
    };

namespace vect_fixed {

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// LOAD FUNTIONS FOR MEMORY ALIGNED ACCESSES
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

__inline__ int8x16_t __attribute__((__always_inline__)) load(const int8x16_t * ptr) {
    return vld1q_s8((const int8_t *) ptr);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// STORE FUNCTIONS FOR MEMORY ALIGNED ACCESSES
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template <class T>
__inline void __attribute__((__always_inline__)) store(T * ptr, const T v) {
    print_trace();
    perror;
}

template <>
__inline void __attribute__((__always_inline__)) store(int8x16_t * ptr, const int8x16_t v) {
    vst1q_s8((int8_t *) ptr, v);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// ADD
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template <class T>
__inline T __attribute__((__always_inline__)) add_s8(const T a, const T b) {
    print_trace();
    perror;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// ADD (8b unsigned, saturated)
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template <class T>
__inline T __attribute__((__always_inline__)) adds_u8(const T a, const T b) {
    print_trace();
    perror;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// ADD (8b signed, saturated)
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template <class T>
__inline T __attribute__((__always_inline__)) adds_s8(const T a, const T b) {
    print_trace();
    perror;
}

template <>
__inline int8x16_t __attribute__((__always_inline__)) adds_s8(const int8x16_t a, const int8x16_t b) {
    return vqaddq_s8(a, b);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// SUB (saturation, unsigned)
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template <class T>
__inline T __attribute__((__always_inline__)) subs_u8(const T a, const T b) {
    print_trace();
    perror;
}

template <>
__inline int8x16_t __attribute__((__always_inline__)) subs_u8(const int8x16_t a, const int8x16_t b) {
    const uint8x16_t _a = vreinterpretq_u8_s8(a);
    const uint8x16_t _b = vreinterpretq_u8_s8(b);
    const uint8x16_t _c = vqsubq_u8(_a, _b);
    return vreinterpretq_s8_u8(_c);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// SUB (saturation, signed)
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template <class T>
__inline T __attribute__((__always_inline__)) subs_s8(const T a, const T b) {
    print_trace();
    perror;
}

template <>
__inline int8x16_t __attribute__((__always_inline__)) subs_s8(const int8x16_t a, const int8x16_t b) {
    return vqsubq_s8(a, b);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// SIGNED MAXIMUM VALUE
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template <class T>
__inline T __attribute__((__always_inline__)) max_s8(const T a, const T b) {
    print_trace();
    perror;
}

template <>
__inline int8x16_t __attribute__((__always_inline__)) max_s8(const int8x16_t a, const int8x16_t b) {
    return vmaxq_s8(a, b);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// UNSIGNED MAXIMUM VALUE
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template <class T>
__inline T __attribute__((__always_inline__)) max_u8(const T a, const T b) {
    print_trace();
    perror;
}

template <>
__inline int8x16_t __attribute__((__always_inline__)) max_u8(const int8x16_t a, const int8x16_t b) {
    return vmaxq_s8(a, b);
}

template <>
__inline uint8x16_t __attribute__((__always_inline__)) max_u8(const uint8x16_t a, const uint8x16_t b) {
    return vmaxq_u8(a, b);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// SIGNED MINIMUM VALUE
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template <class T>
__inline T __attribute__((__always_inline__)) min_s8(const T a, const T b) {
    print_trace();
    perror;
}

template <>
__inline int8x16_t __attribute__((__always_inline__)) min_s8(const int8x16_t a, const int8x16_t b) {
    return vminq_s8(a, b);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// UNSIGNED MINIMUM VALUE
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template <class T>
__inline T __attribute__((__always_inline__)) min_u8(const T a, const T b) {
    print_trace();
    perror;
}

template <>
__inline uint8x16_t __attribute__((__always_inline__)) min_u8(uint8x16_t a, uint8x16_t b) {
    return vminq_u8(a, b);
}
template <>
__inline int8x16_t __attribute__((__always_inline__)) min_u8(int8x16_t a, int8x16_t b) {
    return vminq_u8(a, b);
}

//    template < >
//    __inline int8x16_t __attribute__((__always_inline__)) min_u8(int8x16_t a, int8x16_t b) {
//        const uint8x16_t _a = vreinterpretq_u8_s8(a);
//        const uint8x16_t _b = vreinterpretq_u8_s8(b);
//        const uint8x16_t _c = vminq_u8(_a, _b);
//        return vreinterpretq_s8_u8(_c);
//    }

//    template < >
//    __inline int8x8_t __attribute__((__always_inline__)) min_u8(int8x8_t a, int8x8_t b) {
//        const uint8x8_t _a = vreinterpret_u8_s8(a);
//        const uint8x8_t _b = vreinterpret_u8_s8(b);
//        const uint8x8_t _c = vmin_u8(_a, _b);
//        return vreinterpret_s8_u8(_c);
//    }

#ifdef __AVX2__ // INTEL 256 bits
template <>
__inline __m256i __attribute__((__always_inline__)) min_u8(const __m256i a, const __m256i b) {
    return _mm256_min_epu8(a, b);
}
#endif

#ifdef __AVX512F__ // INTEL 512 bits
template <>
__inline __m512i __attribute__((__always_inline__)) min_u8(const __m512i a, const __m512i b) {
    return _mm512_min_epu8(a, b);
}
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// MIN_2(a, b)
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template <class T>
__inline T __attribute__((__always_inline__)) min_u8(const T a, const T min1, const T min2) {
    const auto x = vect_fixed::max_u8<T>(a, min1);
    return vect_fixed::min_u8<T>(min2, x);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// SET1 (a, b) - int8_t
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template <class T, class Q>
__inline Q __attribute__((__always_inline__)) set1s(const T a) {
    print_trace();
    perror;
}

template <>
__inline int8x16_t __attribute__((__always_inline__)) set1s(const int8_t a) {
    return vdupq_n_s8(a);
}

template <class T, class Q>
__inline Q __attribute__((__always_inline__)) set1u(const T a) {
    return a;
}

template <>
__inline int8x16_t __attribute__((__always_inline__)) set1u(const uint8_t a) {
    return vdupq_n_u8((int8_t) a);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// ABS(a, b)
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template <class T>
__inline T __attribute__((__always_inline__)) abs(const T a) {
    print_trace();
    perror;
}

template <>
__inline int8x16_t __attribute__((__always_inline__)) abs(const int8x16_t a) {
    return vabsq_s8(a);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// OR(a, b)
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template <class T>
__inline T __attribute__((__always_inline__)) orb(const T a, const T b) {
    print_trace();
    perror;
}

template <>
__inline int8x16_t __attribute__((__always_inline__)) orb(const int8x16_t a, const int8x16_t b) {
    return vorrq_s8(a, b);
}

///////////////////////////////////////////////////////////////////////////////////////////
//
// ZERO value( )
//
///////////////////////////////////////////////////////////////////////////////////////////

template <class T>
__inline T __attribute__((__always_inline__)) zero() {
    print_trace();
    perror;
}

template <>
__inline int8x16_t __attribute__((__always_inline__)) zero() {
    return vdupq_n_s8(0);
}

///////////////////////////////////////////////////////////////////////////////////////////
//
// AND binary function
//
///////////////////////////////////////////////////////////////////////////////////////////

template <class T>
__inline T __attribute__((__always_inline__)) andb(const T a, const T b) {
    print_trace();
    perror;
}

template <>
__inline int8x16_t __attribute__((__always_inline__)) andb(const int8x16_t a, const int8x16_t b) {
    return vandq_s8(a, b);
}

///////////////////////////////////////////////////////////////////////////////////////////
//
// XOR binary function
//
///////////////////////////////////////////////////////////////////////////////////////////

template <class T>
__inline T __attribute__((__always_inline__)) xorb(const T a, const T b) {
    print_trace();
    perror;
}

template <>
__inline int8x16_t __attribute__((__always_inline__)) xorb(const int8x16_t a, const int8x16_t b) {
    return veorq_s8(a, b);
}

///////////////////////////////////////////////////////////////////////////////////////////
//
// AND NOT binary function
//
///////////////////////////////////////////////////////////////////////////////////////////

template <class T>
__inline T __attribute__((__always_inline__)) and_notb(const T a, const T b) {
    print_trace();
    perror;
}

template <>
__inline int8x16_t __attribute__((__always_inline__)) and_notb(const int8x16_t a, const int8x16_t b) {
    return vbicq_s8(b, a); // *NOTE* argument swap
}

///////////////////////////////////////////////////////////////////////////////////////////
//
// EQUAL function
//
///////////////////////////////////////////////////////////////////////////////////////////

template <class T>
__inline T __attribute__((__always_inline__)) equal(const T a, const T b) {
    print_trace();
    perror;
}

template <>
__inline int8x16_t __attribute__((__always_inline__)) equal(const int8x16_t a, const int8x16_t b) {
    return vceqq_s8(a, b);
}

///////////////////////////////////////////////////////////////////////////////////////////
//
// XXXXX function
//

template <class T>
__inline T __attribute__((__always_inline__)) unpackhi_epi8(const T a) {
    print_trace();
    perror;
}

///////////////////////////////////////////////////////////////////////////////////////////
//
// XXXXX function
//

template <class T>
__inline T __attribute__((__always_inline__)) unpacklo_epi8(const T a) {
    print_trace();
    perror;
}

///////////////////////////////////////////////////////////////////////////////////////////
//
// XXXXX function
//

template <class T>
__inline T __attribute__((__always_inline__)) div32_epi16(const T a) {
    print_trace();
    perror;
}

///////////////////////////////////////////////////////////////////////////////////////////
//
// XXXXX function
//

template <class T>
__inline T __attribute__((__always_inline__)) pack_epi16(const T a, const T b) {
    print_trace();
    perror;
}

///////////////////////////////////////////////////////////////////////////////////////////
//
// XXXXX function
//

template <class T>
__inline T __attribute__((__always_inline__)) mullo_epi16(const T a, const T b) {
    print_trace();
    perror;
}

///////////////////////////////////////////////////////////////////////////////////////////
//
// CONDITIONNAL MOVE function
//

template <class T>
__inline T __attribute__((__always_inline__)) cmov(const T a, const T b, const T vrai, const T faux) {
    print_trace();
    perror;
}

template <>
__inline int8x16_t __attribute__((__always_inline__)) cmov(int8x16_t a, int8x16_t b, int8x16_t v1, int8x16_t v2) {
    const uint8x16_t z = vceqq_s8(a, b);
    return vbslq_s8(z, v1, v2);
}

///////////////////////////////////////////////////////////////////////////////////////////
//
// SIGN function
//

template <class T>
__inline T __attribute__((__always_inline__)) sign(const T a) {
    print_trace();
    perror;
}

template <>
__inline int8x16_t __attribute__((__always_inline__)) sign(int8x16_t a) {
    const int8x16_t  zero = veorq_s8(a, a); // vdupq_n_s8(0x00);
    const uint8x16_t resu = vcgeq_s8(a, zero);
    return vreinterpretq_s8_u8(resu);
}

///////////////////////////////////////////////////////////////////////////////////////////
//
// INVERSE SIGN function
//

template <class T, class Q>
__inline T __attribute__((__always_inline__)) copysign(const T a, const Q b) {
    print_trace();
    perror;
}

template <>
__inline int8x16_t __attribute__((__always_inline__)) copysign(const int8x16_t a, const int8x16_t b) {
    const int8x16_t minus_a = vnegq_s8(a);
    const int8x16_t zero    = vdupq_n_s8(0);
    const int8x16_t nmask   = vcltq_s8(b, zero);
    return vbslq_s8(nmask, minus_a, a);
}

///////////////////////////////////////////////////////////////////////////////////////////
//
// XOR reduction function
//
template <class T>
__inline bool __attribute__((__always_inline__)) xor_reduce(const T a) {
    print_trace();
    perror;
}

//    #define vreinterpretq_u8_m128i(x) vreinterpretq_u8_s32(x)

template <>
__inline bool __attribute__((__always_inline__)) xor_reduce(const int8x16_t a) {
    const uint8x16_t                                 input      = vreinterpretq_u8_s8(a);
    static const int8_t __attribute__((aligned(16))) xr[8]      = {-7, -6, -5, -4, -3, -2, -1, 0};
    const uint8x8_t                                  mask_and   = vdup_n_u8(0x80);
    const int8x8_t                                   mask_shift = vld1_s8(xr);

    uint8x8_t lo = vget_low_u8(input);
    uint8x8_t hi = vget_high_u8(input);

    lo = vand_u8(lo, mask_and);
    lo = vshl_u8(lo, mask_shift);

    hi = vand_u8(hi, mask_and);
    hi = vshl_u8(hi, mask_shift);

    lo = vpadd_u8(lo, lo);
    lo = vpadd_u8(lo, lo);
    lo = vpadd_u8(lo, lo);

    hi = vpadd_u8(hi, hi);
    hi = vpadd_u8(hi, hi);
    hi = vpadd_u8(hi, hi);

    return ((hi[0] << 8) | (lo[0] & 0xFF));
}

///////////////////////////////////////////////////////////////////////////////////////////
//
// LES FONCTIONS D'ENTRELACEMENT POUR LES CODES LDPCs
//
__inline uint64_t cycles() {
#if defined(__SSE4_2__)
    uint32_t hi, lo;
    __asm__ __volatile__("rdtsc" : "=a"(lo), "=d"(hi));
    return ((uint64_t) lo) | (((uint64_t) hi) << 32);
#elif defined(__ARM_NEON__) || defined(__ARM_NEON) // ARM - 128 bits
    int64_t virtual_timer_value;
    asm volatile("mrs %0, cntvct_el0" : "=r"(virtual_timer_value));
    return virtual_timer_value;
#else
#error "No CPU time counter function..."
    return 0;
#endif
}

///////////////////////////////////////////////////////////////////////////////////////////
//
// LES FONCTIONS D'ENTRELACEMENT POUR LES CODES LDPCs
//
template <class T, class Q>
__inline void __attribute__((__always_inline__)) reorder(T * dest, const Q * src, int length) // a = sign, b = value
{
    for (int i = 0; i < length; i++) {
        dest[i] = src[i]; // OK
    }
    perror;
}

#ifdef __SSE4_2__ // INTEL - 64 bits
template <>
__inline void __attribute__((__always_inline__)) reorder(__m64 * dest, const char * src, int N) // a = sign, b = value
{
    char * ptr = (char *) dest;
    for (int i = 0; i < N; i++) {
        for (int32_t z = 0; z < 8; z += 1) {
            ptr[8 * i + z] = src[z * N + i];
        }
    }
}
#endif

#ifdef __SSE4_2__ // INTEL - 64 bits
template <>
__inline void __attribute__((__always_inline__)) reorder(__m128i * dest, const char * src, int N) // a = sign, b = value
{
    if (N % 16 == 0) {
        uchar_transpose_sse((__m128i *) src, (__m128i *) dest, N);
    } else {
        char * ptr = (char *) dest;
        for (int i = 0; i < N; i++) {
            for (int z = 0; z < 16; z++) {
                ptr[16 * i + z] = src[z * N + i];
            }
        }
    }
}
#endif

#ifdef __AVX2__ // INTEL 256 bits
template <>
__inline void __attribute__((__always_inline__)) reorder(__m256i * dest, const char * src, int N) // a = sign, b = value
{
    if (N % 32 == 0) {
        uchar_transpose_avx2((__m256i *) src, (__m256i *) dest, N);
    } else {
        char * ptr = (char *) dest;
        for (int i = 0; i < N; i++) {
            for (int z = 0; z < 32; z++) {
                ptr[32 * i + z] = src[z * N + i];
            }
        }
    }
}
#endif

#if defined(__ARM_NEON__) || defined(__ARM_NEON)
template <>
__inline void __attribute__((__always_inline__)) reorder(int8x8_t * dest, const char * src, int N) // a = sign, b = value
{
    char * ptr = (char *) dest;
    for (int i = 0; i < N; i++) {
        for (int z = 0; z < 8; z++) {
            ptr[8 * i + z] = src[z * N + i];
        }
    }
}
#endif

#if defined(__ARM_NEON__) || defined(__ARM_NEON)
template <>
__inline void __attribute__((__always_inline__)) reorder(int8x16_t * dest, const char * src, int N) // a = sign, b = value
{
    char * ptr = (char *) dest;
    for (int i = 0; i < N; i++) {
        for (int z = 0; z < 16; z++) {
            ptr[16 * i + z] = src[z * N + i];
        }
    }
}
#endif

#ifdef __AVX512F__ // INTEL 512 bits
template <>
__inline void __attribute__((__always_inline__)) reorder(__m512i * dest, const char * src, int N) // a = sign, b = value
{

    char * ptr = (char *) dest;
    for (int i = 0; i < N; i++) {
        for (int z = 0; z < 64; z++) {
            ptr[64 * i + z] = src[z * N + i];
        }
    }
}
#endif

///////////////////////////////////////////////////////////////////////////////////////////
//
// LES FONCTIONS D'ENTRELACEMENT POUR LES CODES LDPCs
//
template <class T>
__inline void __attribute__((__always_inline__)) clear(T * src, int N) // a = sign, b = value
{
    memset((void *) src, 0, N * sizeof(T));
}

///////////////////////////////////////////////////////////////////////////////////////////
//
// LES FONCTIONS D'ENTRELACEMENT POUR LES CODES LDPCs
//
template <class T>
__inline void __attribute__((__always_inline__)) ireorder(char * dest, const T * src, int N) // a = sign, b = value
{
    for (int i = 0; i < N; i++) {
        dest[i] = (src[i] >= (T) 0); // OK
    }
}

#ifdef __SSE4_2__ // INTEL - 64 bits
template <>
__inline void __attribute__((__always_inline__)) ireorder(char * dest, const __m64 * src, int N) // a = sign, b = value
{
    char * ptr = (char *) src;
    for (int32_t i = 0; i < N; i += 1) {
        for (int32_t j = 0; j < 8; j += 1) {
            dest[j * N + i] = ptr[8 * i + j] > 0;
        }
    }
}
#endif

#ifdef __SSE4_2__ // INTEL - 64 bits
template <>
__inline void __attribute__((__always_inline__)) ireorder(char * dest, const __m128i * src, int N) // a = sign, b = value
{
    if (N % 16 == 0) {
        uchar_itranspose_sse((__m128i *) src, (__m128i *) dest, N);
    } else {
        char * ptr = (char *) src;
        for (int i = 0; i < N; i += 1) {
            for (int j = 0; j < 16; j += 1) {
                dest[j * N + i] = ptr[16 * i + j] > 0;
            }
        }
    }
}
#endif

#ifdef __AVX2__                                                                                    // INTEL 256 bits
__inline void __attribute__((__always_inline__)) ireorder(char * dest, const __m256i * src, int N) // a = sign, b = value
{
    uchar_itranspose_2_avx2((__m256i *) src, (__m256i *) dest, N);
    /*
            if( N%32 == 0  ){
                uchar_itranspose_avx((__m256i*)src, (__m256i*)dest, N);
            }else{
                char* ptr = (char*)src;
                for (int i=0; i<N; i+=1){
                    for (int j=0; j<32; j+=1){
                        dest[j * N + i] = ptr[32 * i + j] > 0;
                    }
                }
            }
     */
}
#endif

#if defined(__ARM_NEON__) || defined(__ARM_NEON)
template <>
__inline void __attribute__((__always_inline__)) ireorder(char * dest, const int8x8_t * src, int N) {
    char * ptr = (char *) src;
    for (int i = 0; i < N; i += 1) {
        for (int j = 0; j < 8; j += 1) {
            dest[j * N + i] = ptr[8 * i + j] >= 0;
        }
    }
}
#endif

#if defined(__ARM_NEON__) || defined(__ARM_NEON)
template <>
__inline void __attribute__((__always_inline__)) ireorder(char * dest, const int8x16_t * src, int N) {
    char * ptr = (char *) src;
    for (int i = 0; i < N; i += 1) {
        for (int j = 0; j < 16; j += 1) {
            dest[j * N + i] = ptr[16 * i + j] >= 0;
        }
    }
}
#endif

#ifdef __AVX512F__ // INTEL 512 bits
template <>
__inline void __attribute__((__always_inline__)) ireorder(char * dest, const __m512i * src, int N) {
    char * ptr = (char *) src;
    for (int i = 0; i < N; i += 1) {
        for (int j = 0; j < 64; j += 1) {
            dest[j * N + i] = ptr[64 * i + j] >= 0;
        }
    }
}
#endif

} // namespace vect_fixed

#if defined(__ARM_NEON__) || defined(__ARM_NEON)
inline void int8_t_show(const int8x16_t reg) {
    int8_t tmp[16];
    vst1q_s32((int32_t *) tmp, reg);
    for (int i = 0; i < 16; i++) {
        printf("%4d ", (int32_t) tmp[i]);
    }
    printf("\n");
}
#endif

// #ifdef __SSE4__
// #else
//   #warning "__SSE4__ not activated"
// #endif

// #ifdef __NEON__
// #include "vect_neon.hpp"
// #warning "__NEON__ activated"
// #endif

// #ifdef __AVX2__  // INTEL 256 bits
// #include "vect_m256i.hpp"
// #else
// #warning "__AVX2__ not activated"
// #endif

// #ifdef __AVX512__
// #include "vect_m512i.hpp"
// #endif

#endif // CLASS_VectLib
