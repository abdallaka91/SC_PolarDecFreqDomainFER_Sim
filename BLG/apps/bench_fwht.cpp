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

#include "features/archi.hpp"

#if defined(__AVX512F__)
    #include "features/fwht/fwht_avx512.hpp"
    #include "features/fwht/fwht_norm_avx512.hpp"
#endif

#include <chrono>
#include <cstring>

#include "utilities/utility_functions.hpp"

bool are_equivalent(float * a, float * b, float epsilon, int size) {
    for (int i = 0; i < size; i++) {
        float diff = abs(a[i] - b[i]);
        if (diff > epsilon) {
            printf("   -> maximum absolute error is : %f : a[%d] = %f and b[%d] = %f\n", diff, i, a[i], i, b[i]);
/*
            show_symbols(a, size);
            normalize(b, size);
            show_symbols(b, size);
*/
            return false;
        }
    }
    return true;
}

int main(int argc, char *[]) {

    if (argc == 2)
        srand(time(NULL));

#if defined(__APPLE__)
    printf("(II) Benchmarking the FWHT functions on MacOS\n");
#elif defined(__linux__)
    printf("(II) Benchmarking the FWHT functions on Linux\n");
#else
    printf("(II) Benchmarking the FWHT functions on a undefined platform\n");
#endif

#if defined(__clang__)
    printf("(II) Code compiled with LLVM (%d.%d.%d)\n", __clang_major__, __clang_minor__, __clang_patchlevel__);
#elif defined(__GNUC__)
    printf("(II) Code compiled with GCC (%d.%d.%d)\n", __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__);
#else
    printf("(II) Code compiled with UNKWON compiler\n");
#endif

    const int32_t nTest = (64 * 1024 * 1024);

    for (int size = 16; size <= 4096; size *= 2) {

        float * tab_i = new float[size];
        float * tab_a = new float[size];
        float * tab_z = new float[size];

        for (int i = 0; i < size; i++) {
            tab_i[i] = ((float) rand()) / ((float) RAND_MAX);
        }
        tab_i[ rand()%size ] = .5f;

        float sum = 1e-32f;
        for (int i = 0; i < size; i += 1) { sum += tab_i[i]; }
        const float factor = 1.f / sum;
        for (int i = 0; i < size; i++) { tab_i[i] *= factor; }

#if 0
        fwht< 64>( tab_a );
        normalize< 64>( tab_a, 0.125f );
        show_symbols( tab_a );
        fwht< 64>( tab_a );
        normalize< 64>( tab_a, 0.125f );
        show_symbols( tab_a );


        fwht_avx2< 64>( tab_d );
        normalize< 64>( tab_d, 0.125f );
        show_symbols( tab_d );
        fwht_avx2< 64>( tab_d );
        normalize< 64>( tab_d, 0.125f );
        show_symbols( tab_d );

        fwht_norm_avx2< 64>( tab_e );
        show_symbols( tab_e );
        fwht_norm_avx2< 64>( tab_e );
        show_symbols( tab_e );

        exit( 0 );
#endif

        //
        //
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //
        //

        printf("+> testing functions [ll = %4d]\n", size);
        memcpy(tab_a, tab_i, size * sizeof(float));

        auto start_x86 = std::chrono::system_clock::now();
        for (int32_t loop = 0; loop < nTest; loop += 1) {
            if (size ==    8) { fwht<   8>(tab_a); normalize<   8>(tab_a, 0.35355339059f); fwht<   8>(tab_a); normalize<   8>(tab_a, 0.35355339059f); }
            if (size ==   16) { fwht<  16>(tab_a); normalize<  16>(tab_a, 0.25f         ); fwht<  16>(tab_a); normalize<  16>(tab_a, 0.25f         ); }
            if (size ==   32) { fwht<  32>(tab_a); normalize<  32>(tab_a, 0.17677669529f); fwht<  32>(tab_a); normalize<  32>(tab_a, 0.17677669529f); }
            if (size ==   64) { fwht<  64>(tab_a); normalize<  64>(tab_a, 0.125f        ); fwht<  64>(tab_a); normalize<  64>(tab_a, 0.125f        ); }
            if (size ==  128) { fwht< 128>(tab_a); normalize< 128>(tab_a, 0.08838834764f); fwht< 128>(tab_a); normalize< 128>(tab_a, 0.08838834764f); }
            if (size ==  256) { fwht< 256>(tab_a); normalize< 256>(tab_a, 0.0625f       ); fwht< 256>(tab_a); normalize< 256>(tab_a, 0.0625f       ); }
            if (size ==  512) { fwht< 512>(tab_a); normalize< 512>(tab_a, 0.04419417382f); fwht< 512>(tab_a); normalize< 512>(tab_a, 0.04419417382f); }
            if (size == 1024) { fwht<1024>(tab_a); normalize<1024>(tab_a, 0.03125f      ); fwht<1024>(tab_a); normalize<1024>(tab_a, 0.03125f      ); }
        }
        auto     stop_x86 = std::chrono::system_clock::now();
        bool     ok_x86   = are_equivalent(tab_i, tab_a, 0.00001, size);
        uint64_t time_x86 = std::chrono::duration_cast<std::chrono::nanoseconds>(stop_x86 - start_x86).count() / nTest;
        if (ok_x86) {
            printf(" - [GCCV] fwht           \033[32mOK\033[0m [%5d ns]\n", (int32_t) time_x86);
        } else {
            printf(" - [GCCV] fwht           \033[31mKO\033[0m [%5d ns]\n", (int32_t) time_x86);
        }

        //
        //
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //
        //
/*
        start_x86 = std::chrono::system_clock::now();
        memcpy(tab_a, tab_i, size * sizeof(float));
        for (int32_t loop = 0; loop < nTest; loop += 1) {
            if (size ==    8) { fwht<   8>(tab_z, tab_a); normalize<   8>(tab_z, 0.35355339059f); fwht<   8>(tab_a, tab_z); normalize<   8>(tab_a, 0.35355339059f); }
            if (size ==   16) { fwht<  16>(tab_z, tab_a); normalize<  16>(tab_z, 0.25f         ); fwht<  16>(tab_a, tab_z); normalize<  16>(tab_a, 0.25f         ); }
            if (size ==   32) { fwht<  32>(tab_z, tab_a); normalize<  32>(tab_z, 0.17677669529f); fwht<  32>(tab_a, tab_z); normalize<  32>(tab_a, 0.17677669529f); }
            if (size ==   64) { fwht<  64>(tab_z, tab_a); normalize<  64>(tab_z, 0.125f        ); fwht<  64>(tab_a, tab_z); normalize<  64>(tab_a, 0.125f        ); }
            if (size ==  128) { fwht< 128>(tab_z, tab_a); normalize< 128>(tab_z, 0.08838834764f); fwht< 128>(tab_a, tab_z); normalize< 128>(tab_a, 0.08838834764f); }
            if (size ==  256) { fwht< 256>(tab_z, tab_a); normalize< 256>(tab_z, 0.0625f       ); fwht< 256>(tab_a, tab_z); normalize< 256>(tab_a, 0.0625f       ); }
            if (size ==  512) { fwht< 512>(tab_z, tab_a); normalize< 512>(tab_z, 0.04419417382f); fwht< 512>(tab_a, tab_z); normalize< 512>(tab_a, 0.04419417382f); }
            if (size == 1024) { fwht<1024>(tab_z, tab_a); normalize<1024>(tab_z, 0.03125f      ); fwht<1024>(tab_a, tab_z); normalize<1024>(tab_a, 0.03125f      ); }
        }
        stop_x86 = std::chrono::system_clock::now();
        ok_x86   = are_equivalent(tab_i, tab_a, 0.002, size);
        time_x86 = std::chrono::duration_cast<std::chrono::nanoseconds>(stop_x86 - start_x86).count() / nTest;
        if (ok_x86) {
            printf(" - [GCCV] fwht (2x)      \033[32mOK\033[0m [%5d ns]\n", (int32_t) time_x86);
        } else {
            printf(" - [GCCV] fwht (2x)      \033[31mKO\033[0m [%5d ns]\n", (int32_t) time_x86);
        }
*/
        //
        //
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //
        //

        auto start_x86_n = std::chrono::system_clock::now();
        memcpy(tab_a, tab_i, size * sizeof(float));
        for (int32_t loop = 0; loop < nTest; loop += 1) {
            if (size ==    8) { fwht_norm<   8>(tab_a); fwht_norm<   8>(tab_a); }
            if (size ==   16) { fwht_norm<  16>(tab_a); fwht_norm<  16>(tab_a); }
            if (size ==   32) { fwht_norm<  32>(tab_a); fwht_norm<  32>(tab_a); }
            if (size ==   64) { fwht_norm<  64>(tab_a); fwht_norm<  64>(tab_a); }
            if (size ==  128) { fwht_norm< 128>(tab_a); fwht_norm< 128>(tab_a); }
            if (size ==  256) { fwht_norm< 256>(tab_a); fwht_norm< 256>(tab_a); }
            if (size ==  512) { fwht_norm< 512>(tab_a); fwht_norm< 512>(tab_a); }
            if (size == 1024) { fwht_norm<1024>(tab_a); fwht_norm<1024>(tab_a); }
        }
        auto           stop_x86_n = std::chrono::system_clock::now();
        const bool     ok_x86_n   = are_equivalent(tab_i, tab_a, 0.00001, size);
        const uint64_t time_x86_n = std::chrono::duration_cast<std::chrono::nanoseconds>(stop_x86_n - start_x86_n).count() / nTest;
        if (ok_x86_n) {
            printf(" - [GCCV] fwht_norm      \033[32mOK\033[0m [%5d ns]\n", (int32_t) time_x86_n);
        } else {
            printf(" - [GCCV] fwht_norm      \033[31mKO\033[0m [%5d ns]\n", (int32_t) time_x86_n);
        }

#if defined(__ARM_NEON__)
        auto start_i_neon = std::chrono::system_clock::now();
        memcpy(tab_a, tab_i, size * sizeof(float));
        for (int32_t loop = 0; loop < nTest; loop += 1) {
            if (size ==    8) { fwht_neon<   8>(tab_a); normalize<   8>(tab_a, 0.35355339059f); fwht_neon<   8>(tab_a); normalize<   8>(tab_a, 0.35355339059f); }
            if (size ==   16) { fwht_neon<  16>(tab_a); normalize<  16>(tab_a, 0.25f         ); fwht_neon<  16>(tab_a); normalize<  16>(tab_a, 0.25f         ); }
            if (size ==   32) { fwht_neon<  32>(tab_a); normalize<  32>(tab_a, 0.17677669529f); fwht_neon<  32>(tab_a); normalize<  32>(tab_a, 0.17677669529f); }
            if (size ==   64) { fwht_neon<  64>(tab_a); normalize<  64>(tab_a, 0.125f        ); fwht_neon<  64>(tab_a); normalize<  64>(tab_a, 0.125f        ); }
            if (size ==  128) { fwht_neon< 128>(tab_a); normalize< 128>(tab_a, 0.08838834764f); fwht_neon< 128>(tab_a); normalize< 128>(tab_a, 0.08838834764f); }
            if (size ==  256) { fwht_neon< 256>(tab_a); normalize< 256>(tab_a, 0.0625f       ); fwht_neon< 256>(tab_a); normalize< 256>(tab_a, 0.0625f       ); }
            if (size ==  512) { fwht_neon< 512>(tab_a); normalize< 512>(tab_a, 0.04419417382f); fwht_neon< 512>(tab_a); normalize< 512>(tab_a, 0.04419417382f); }
            if (size == 1024) { fwht_neon<1024>(tab_a); normalize<1024>(tab_a, 0.03125f      ); fwht_neon<1024>(tab_a); normalize<1024>(tab_a, 0.03125f      ); }
        }
        auto           stop_i_neon = std::chrono::system_clock::now();
        const bool     ok_neon     = are_equivalent(tab_i, tab_a, 0.00001, size);
        const uint64_t time_neon   = std::chrono::duration_cast<std::chrono::nanoseconds>(stop_i_neon - start_i_neon).count() / nTest;
        if (ok_neon) {
            printf(" - [NEON] fwht_neon      \033[32mOK\033[0m [%5d ns]\n", (int32_t) time_neon);
        } else {
            printf(" - [NEON] fwht_neon      \033[31mKO\033[0m [%5d ns]\n", (int32_t) time_neon);
        }
#endif

#if defined(__ARM_NEON__)
        auto start_i_neon_norm = std::chrono::system_clock::now();
        memcpy(tab_a, tab_i, size * sizeof(float));
        for (int32_t loop = 0; loop < nTest; loop += 1) {
            if (size ==    8) { fwht_norm_neon<   8>(tab_a); fwht_norm_neon<   8>(tab_a); }
            if (size ==   16) { fwht_norm_neon<  16>(tab_a); fwht_norm_neon<  16>(tab_a); }
            if (size ==   32) { fwht_norm_neon<  32>(tab_a); fwht_norm_neon<  32>(tab_a); }
            if (size ==   64) { fwht_norm_neon<  64>(tab_a); fwht_norm_neon<  64>(tab_a); }
            if (size ==  128) { fwht_norm_neon< 128>(tab_a); fwht_norm_neon< 128>(tab_a); }
            if (size ==  256) { fwht_norm_neon< 256>(tab_a); fwht_norm_neon< 256>(tab_a); }
            if (size ==  512) { fwht_norm_neon< 512>(tab_a); fwht_norm_neon< 512>(tab_a); }
            if (size == 1024) { fwht_norm_neon<1024>(tab_a); fwht_norm_neon<1024>(tab_a); }
        }
        auto           stop_i_neon_norm = std::chrono::system_clock::now();
        const bool     ok_neon_norm     = are_equivalent(tab_i, tab_a, 0.00001, size);
        const uint64_t time_neon_norm   = std::chrono::duration_cast<std::chrono::nanoseconds>(stop_i_neon_norm - start_i_neon_norm).count() / nTest;
        if (ok_neon_norm) {
            printf(" - [NEON] fwht_norm_neon \033[32mOK\033[0m [%5d ns]\n", (int32_t) time_neon_norm);
        } else {
            printf(" - [NEON] fwht_norm_neon \033[31mKO\033[0m [%5d ns]\n", (int32_t) time_neon_norm);
        }
#endif


#if 0 // THERE IS A BUG INSIDE ! defined(__ARM_NEON__)
        auto start_i_neon_norm_v2 = std::chrono::system_clock::now();
        memcpy(tab_a, tab_i, size * sizeof(float));
        for (int32_t loop = 0; loop < nTest; loop += 1) {
            if (size ==    8) { lwht_norm_generic<   8>(tab_a); lwht_norm_generic<   8>(tab_a); }
            if (size ==   16) { lwht_norm_generic<  16>(tab_a); lwht_norm_generic<  16>(tab_a); }
            if (size ==   32) { lwht_norm_generic<  32>(tab_a); lwht_norm_generic<  32>(tab_a); }
            if (size ==   64) { lwht_norm_generic<  64>(tab_a); lwht_norm_generic<  64>(tab_a); }
            if (size ==  128) { lwht_norm_generic< 128>(tab_a); lwht_norm_generic< 128>(tab_a); }
            if (size ==  256) { lwht_norm_generic< 256>(tab_a); lwht_norm_generic< 256>(tab_a); }
            if (size ==  512) { lwht_norm_generic< 512>(tab_a); lwht_norm_generic< 512>(tab_a); }
            if (size == 1024) { lwht_norm_generic<1024>(tab_a); lwht_norm_generic<1024>(tab_a); }
        }
        auto           stop_i_neon_norm_v2 = std::chrono::system_clock::now();
        const bool     ok_neon_norm_v2     = are_equivalent(tab_i, tab_a, 0.002, size);
        const uint64_t time_neon_norm_v2   = std::chrono::duration_cast<std::chrono::nanoseconds>(stop_i_neon_norm_v2 - start_i_neon_norm_v2).count() / nTest;
        if (ok_neon_norm_v2) {
            printf(" - [NEON] fwht_norm_neo2 \033[32mOK\033[0m [%5d ns]\n", (int32_t) time_neon_norm_v2);
        } else {
            printf(" - [NEON] fwht_norm_neo2 \033[31mKO\033[0m [%5d ns]\n", (int32_t) time_neon_norm_v2);
        }
#endif

#if defined(__AVX2__)
    {
        auto start_i_avx2 = std::chrono::system_clock::now();
        memcpy(tab_a, tab_i, size * sizeof(float));
        for (int32_t loop = 0; loop < nTest; loop += 1) {
            if (size ==    8) { fwht_avx2<   8>(tab_a); normalize<   8>(tab_a, 0.35355339059f); fwht_avx2<   8>(tab_a); normalize<   8>(tab_a, 0.35355339059f); }
            if (size ==   16) { fwht_avx2<  16>(tab_a); normalize<  16>(tab_a, 0.25f         ); fwht_avx2<  16>(tab_a); normalize<  16>(tab_a, 0.25f         ); }
            if (size ==   32) { fwht_avx2<  32>(tab_a); normalize<  32>(tab_a, 0.17677669529f); fwht_avx2<  32>(tab_a); normalize<  32>(tab_a, 0.17677669529f); }
            if (size ==   64) { fwht_avx2<  64>(tab_a); normalize<  64>(tab_a, 0.125f        ); fwht_avx2<  64>(tab_a); normalize<  64>(tab_a, 0.125f        ); }
            if (size ==  128) { fwht_avx2< 128>(tab_a); normalize< 128>(tab_a, 0.08838834764f); fwht_avx2< 128>(tab_a); normalize< 128>(tab_a, 0.08838834764f); }
            if (size ==  256) { fwht_avx2< 256>(tab_a); normalize< 256>(tab_a, 0.0625f       ); fwht_avx2< 256>(tab_a); normalize< 256>(tab_a, 0.0625f       ); }
            if (size ==  512) { fwht_avx2< 512>(tab_a); normalize< 512>(tab_a, 0.04419417382f); fwht_avx2< 512>(tab_a); normalize< 512>(tab_a, 0.04419417382f); }
            if (size == 1024) { fwht_avx2<1024>(tab_a); normalize<1024>(tab_a, 0.03125f      ); fwht_avx2<1024>(tab_a); normalize<1024>(tab_a, 0.03125f      ); }
        }
        auto           stop_i_avx2 = std::chrono::system_clock::now();
        const bool     ok_avx2     = are_equivalent(tab_i, tab_a, 0.002, size);
        const uint64_t time_avx2   = std::chrono::duration_cast<std::chrono::nanoseconds>(stop_i_avx2 - start_i_avx2).count() / nTest;
        if (ok_avx2) {
            printf(" - [AVX2] fwht_avx2      \033[32mOK\033[0m [%5d ns]\n", (int32_t) time_avx2);
        } else {
            printf(" - [AVX2] fwht_avx2      \033[31mKO\033[0m [%5d ns]\n", (int32_t) time_avx2);
        }
    }
#endif

#if defined(__AVX2__)
    {
        auto start_i_norm_avx2 = std::chrono::system_clock::now();
        memcpy(tab_a, tab_i, size * sizeof(float));
        for (int32_t loop = 0; loop < nTest; loop += 1) {
            if (size ==    8) { fwht_norm_avx2<   8>(tab_a); fwht_norm_avx2<   8>(tab_a); }
            if (size ==   16) { fwht_norm_avx2<  16>(tab_a); fwht_norm_avx2<  16>(tab_a); }
            if (size ==   32) { fwht_norm_avx2<  32>(tab_a); fwht_norm_avx2<  32>(tab_a); }
            if (size ==   64) { fwht_norm_avx2<  64>(tab_a); fwht_norm_avx2<  64>(tab_a); }
            if (size ==  128) { fwht_norm_avx2< 128>(tab_a); fwht_norm_avx2< 128>(tab_a); }
            if (size ==  256) { fwht_norm_avx2< 256>(tab_a); fwht_norm_avx2< 256>(tab_a); }
            if (size ==  512) { fwht_norm_avx2< 512>(tab_a); fwht_norm_avx2< 512>(tab_a); }
            if (size == 1024) { fwht_norm_avx2<1024>(tab_a); fwht_norm_avx2<1024>(tab_a); }
        }
        auto           stop_i_norm_avx2 = std::chrono::system_clock::now();
        const bool     ok_norm_avx2     = are_equivalent(tab_i, tab_a, 0.002, size);
        const uint64_t time_norm_avx2   = std::chrono::duration_cast<std::chrono::nanoseconds>(stop_i_norm_avx2 - start_i_norm_avx2).count() / nTest;
        if (ok_norm_avx2) {
            printf(" - [AVX2] fwht_norm_avx2 \033[32mOK\033[0m [%5d ns]\n", (int32_t) time_norm_avx2);
        } else {
            printf(" - [AVX2] fwht_norm_avx2 \033[31mKO\033[0m [%5d ns]\n", (int32_t) time_norm_avx2);
        }
    }
#endif

#if defined(__AVX512F__)
    {
        auto start_i_avx2 = std::chrono::system_clock::now();
        memcpy(tab_a, tab_i, size * sizeof(float));
        for (int32_t loop = 0; loop < nTest; loop += 1) {
            if (size ==    8) { fwht_avx512<   8>(tab_a); normalize<   8>(tab_a, 0.35355339059f); fwht_avx512<   8>(tab_a); normalize<   8>(tab_a, 0.35355339059f); }
            if (size ==   16) { fwht_avx512<  16>(tab_a); normalize<  16>(tab_a, 0.25f         ); fwht_avx512<  16>(tab_a); normalize<  16>(tab_a, 0.25f         ); }
            if (size ==   32) { fwht_avx512<  32>(tab_a); normalize<  32>(tab_a, 0.17677669529f); fwht_avx512<  32>(tab_a); normalize<  32>(tab_a, 0.17677669529f); }
            if (size ==   64) { fwht_avx512<  64>(tab_a); normalize<  64>(tab_a, 0.125f        ); fwht_avx512<  64>(tab_a); normalize<  64>(tab_a, 0.125f        ); }
            if (size ==  128) { fwht_avx512< 128>(tab_a); normalize< 128>(tab_a, 0.08838834764f); fwht_avx512< 128>(tab_a); normalize< 128>(tab_a, 0.08838834764f); }
            if (size ==  256) { fwht_avx512< 256>(tab_a); normalize< 256>(tab_a, 0.0625f       ); fwht_avx512< 256>(tab_a); normalize< 256>(tab_a, 0.0625f       ); }
            if (size ==  512) { fwht_avx512< 512>(tab_a); normalize< 512>(tab_a, 0.04419417382f); fwht_avx512< 512>(tab_a); normalize< 512>(tab_a, 0.04419417382f); }
            if (size == 1024) { fwht_avx512<1024>(tab_a); normalize<1024>(tab_a, 0.03125f      ); fwht_avx512<1024>(tab_a); normalize<1024>(tab_a, 0.03125f      ); }
        }
        auto           stop_i_avx2 = std::chrono::system_clock::now();
        const bool     ok_avx2     = are_equivalent(tab_i, tab_a, 0.002, size);
        const uint64_t time_avx2   = std::chrono::duration_cast<std::chrono::nanoseconds>(stop_i_avx2 - start_i_avx2).count() / nTest;
        if (ok_avx2) {
            printf(" - [AVX2] fwht_avx512    \033[32mOK\033[0m [%5d ns]\n", (int32_t) time_avx2);
        } else {
            printf(" - [AVX2] fwht_avx512    \033[31mKO\033[0m [%5d ns]\n", (int32_t) time_avx2);
        }
    }
#endif

#if defined(__AVX512F__)
    {
        auto start_i_norm_avx2 = std::chrono::system_clock::now();
        memcpy(tab_a, tab_i, size * sizeof(float));
        for (int32_t loop = 0; loop < nTest; loop += 1) {
            if (size ==    8) { fwht_norm_avx512<   8>(tab_a); fwht_norm_avx512<   8>(tab_a); }
            if (size ==   16) { fwht_norm_avx512<  16>(tab_a); fwht_norm_avx512<  16>(tab_a); }
            if (size ==   32) { fwht_norm_avx512<  32>(tab_a); fwht_norm_avx512<  32>(tab_a); }
            if (size ==   64) { fwht_norm_avx512<  64>(tab_a); fwht_norm_avx512<  64>(tab_a); }
            if (size ==  128) { fwht_norm_avx512< 128>(tab_a); fwht_norm_avx512< 128>(tab_a); }
            if (size ==  256) { fwht_norm_avx512< 256>(tab_a); fwht_norm_avx512< 256>(tab_a); }
            if (size ==  512) { fwht_norm_avx512< 512>(tab_a); fwht_norm_avx512< 512>(tab_a); }
            if (size == 1024) { fwht_norm_avx512<1024>(tab_a); fwht_norm_avx512<1024>(tab_a); }
        }
        auto           stop_i_norm_avx2 = std::chrono::system_clock::now();
        const bool     ok_norm_avx2     = are_equivalent(tab_i, tab_a, 0.002, size);
        const uint64_t time_norm_avx2   = std::chrono::duration_cast<std::chrono::nanoseconds>(stop_i_norm_avx2 - start_i_norm_avx2).count() / nTest;
        if (ok_norm_avx2) {
            printf(" - [AVX2] fwht_norm_avx5x\033[32mOK\033[0m [%5d ns]\n", (int32_t) time_norm_avx2);
        } else {
            printf(" - [AVX2] fwht_norm_avx5x\033[31mKO\033[0m [%5d ns]\n", (int32_t) time_norm_avx2);
        }
    }
#endif


        delete[] tab_i;
        delete[] tab_a;
        delete[] tab_z;
    }

    return EXIT_SUCCESS;
}
