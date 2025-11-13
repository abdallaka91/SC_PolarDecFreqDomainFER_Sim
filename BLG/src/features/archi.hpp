#pragma once

#if defined(__ARM_NEON__) || defined(__ARM_NEON)
#elif defined(__AVX2__)
#else
#endif

#include "fwht/fwht.hpp"
#include "fwht/fwht_norm.hpp"

#if defined(__ARM_NEON__) || defined(__ARM_NEON)
    #include "fwht/fwht_neon.hpp"
    #include "fwht/fwht_norm_neon.hpp"
    #include "fwht/fwht_norm_neon_v2.hpp"
#endif
#if defined(__AVX512F__)
    #include "fwht/fwht_avx512.hpp"
    #include "fwht/fwht_norm_avx512.hpp"
#endif
#if defined(__AVX2__)
    #include "fwht/fwht_avx2.hpp"
    #include "fwht/fwht_norm_avx2.hpp"
#endif

#if defined(__ARM_NEON__) || defined(__ARM_NEON)

//    #define FWHT_NORM lwht_norm_generic
    #define FWHT_NORM fwht_norm_neon
//    #define FWHT_NORM lwht_norm_generic

#elif defined(__AVX512F__)
    #define FWHT_NORM fwht_norm_avx2 // fwht_norm_avx512 (it performs better !!!)
#elif defined(__AVX2__)
    #define FWHT_NORM fwht_norm_avx2
#else
    #define FWHT_NORM fwht_norm
#endif

#include "argmax/argmax.hpp"

#include "normalize/normalize.hpp"