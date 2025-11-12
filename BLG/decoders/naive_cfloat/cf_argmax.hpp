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
template <int gf_size>
int f_argmax(const flx::floatx<N_EXPO, N_MANT>* value)
{
    int               max_index = 0;
    flx::floatx<N_EXPO, N_MANT>  max_value = value[0];

    for (int i = 1; i < gf_size; i++) {
        if (value[i] > max_value) {
            max_value = value[i];
            max_index = i;
        }
    }
    return max_index;
}
//
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//
