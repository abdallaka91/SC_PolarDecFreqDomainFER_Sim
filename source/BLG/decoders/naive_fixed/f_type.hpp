#pragma once
//
//
//
//
//
#include "include/ap_fixed.h"
//
#define NBITS   64
#define NFRAC   32
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
