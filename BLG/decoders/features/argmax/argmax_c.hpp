//!
//!     Copyright (c) 2020-2023, Bertrand LE GAL
//!     All rights reserved.
//!
//!     Redistribution and use in source and binary forms, with or without
//!     modification, are not permitted with written authorization.
//!
//!
#pragma once
//
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//
#include <cstdint>
//
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//
#if defined(__ARM_NEON__) || defined(__ARM_NEON)
    #include "argmax_neon.hpp"
#elif defined(__AVX2__)
    #include "argmax_avx2.hpp"
#else
    #include "argmax_x86.hpp"
#endif
