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
int f_argmax(const ap_fixed<NBITS, NINTG, AP_TRN, AP_SAT> * value) {
    int                                    max_index = 0;
    ap_fixed<NBITS, NINTG, AP_TRN, AP_SAT> max_value = value[0];

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
