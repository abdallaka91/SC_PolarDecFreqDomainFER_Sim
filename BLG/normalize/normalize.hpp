//
// Created by legal on 23/07/2025.
//
#pragma once

#if defined(__AVX512F__)
    #include "normalize/normalize_avx512.hpp"
#elif defined(__AVX2__)
    #include "normalize/normalize_avx2.hpp"
#elif defined(__ARM_NEON__) || defined(__ARM_NEON)
    #include "normalize/normalize_neon.hpp"
#else
    #include "normalize/normalize_c.hpp"
#endif
