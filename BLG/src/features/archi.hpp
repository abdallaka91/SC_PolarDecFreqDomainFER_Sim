#pragma once

#if defined(__ARM_NEON__) || defined(__ARM_NEON)

#elif defined(__AVX2__)

#else

#endif

#if defined(__ARM_NEON__) || defined(__ARM_NEON)
#include "fwht/fwht_neon.hpp"
#endif

#if defined(__AVX512F__)
#include "fwht/fwht_avx512.hpp"
#endif

#if defined(__AVX2__)
#include "fwht/fwht_avx2.hpp"
#endif

#if defined(__ARM_NEON__) || defined(__ARM_NEON)
#define FUNCTION_SUFFIX _neon
#elif defined(__AVX512F__)
#define FUNCTION_SUFFIX _avx2
#elif defined(__AVX2__)
#define FUNCTION_SUFFIX _avx2
#else
#define FUNCTION_SUFFIX
#endif

#define PASTER(x, y)    x##y
#define EVALUATOR(x, y) PASTER(x, y)

#define FWHT EVALUATOR(fwht, FUNCTION_SUFFIX)

#include "argmax/argmax.hpp"

#include "normalize/normalize.hpp"
