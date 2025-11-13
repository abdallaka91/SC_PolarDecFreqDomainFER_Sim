#pragma once
//
//
//
//
//
#include "floatx.hpp"
//
#define N_EXPO 8
#define N_MANT 23
//
struct symbols_cf
{
    flx::floatx<N_EXPO, N_MANT> value[_GF_];
    bool                        is_freq;
};
//
//
//
//
//
inline float is_issue(const flx::floatx<N_EXPO, N_MANT> * value)
{
    float maxv = 0.f;
    for (size_t i = 0; i < _GF_; i++)
    {
        const float v = float(value[i]);
        maxv          = std::fmaxf(maxv, std::fabs(v));
    }
    return maxv;
}
//
//
//
//
//
#include "cf_argmax.hpp"
#include "cf_fwht.hpp"
#include "cf_fwht_norm.hpp"
#include "cf_normalize.hpp"
