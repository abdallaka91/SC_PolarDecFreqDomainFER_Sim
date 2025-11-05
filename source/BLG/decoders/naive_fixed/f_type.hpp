#pragma once
//
//
//
//
//
#include "include/ap_fixed.h"
//
#define NBITS   32
#define NFRAC   24
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
inline float is_issue(const ap_fixed<NBITS, NFRAC>* value)
{
    float maxv = 0.f;
    for (size_t i = 0; i < _GF_; i++)
    {
        const float v = value[i].to_float();
        maxv = std::fmaxf( maxv, std::fabs( v ) );
    }
    return maxv;
}
//
//
//
//
//
