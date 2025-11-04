#pragma once
//
//
//
//
//
#include "include/ap_fixed.h"
//
#define NBITS   32
#define NFRAC   22
//
struct symbols_f {
    ap_fixed<NBITS, NFRAC> value[_GF_];
    bool is_freq;
};
//
//
//
//
//
