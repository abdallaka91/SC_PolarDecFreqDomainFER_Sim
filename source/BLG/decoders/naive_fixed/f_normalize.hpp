//
// Created by legal on 03/07/2025.

#define _meth_ 2
#pragma once
//
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//

#if _meth_ == 0
template <int gf_size>
void f_normalize(ap_fixed<NBITS, NINTG> *tab)
{
    ap_fixed<NBITS, NINTG> sum = 1e-24f;
    for (int i = 0; i < gf_size; i += 1)
    {
        sum += tab[i];
    }
    const ap_fixed<NBITS, NINTG> zero = 0.f;
    if (zero == sum)
    {
        sum = 1.f;
    }
    float sum1 = (float)sum;

    for (int i = 0; i < gf_size; i++)
    {
        tab[i] /= sum;
    }

    float sum2 = 0.0f;
    for (int i = 0; i < gf_size; i += 1)
    {
        sum2 += float(tab[i]);
    }
    // printf("%f, %f\n", sum1, sum2);
}

#else
template <int gf_size>
void f_normalize(ap_fixed<NBITS, NINTG> *tab)
{
    ap_fixed<NBITS, NINTG> sum = 0.0f;
    ap_fixed<NBITS, NINTG> one = 1.0f;
    ap_fixed<NBITS, NINTG> zero = 0.0f;
    for (int i = 0; i < gf_size; i++)
        sum += tab[i];

    if ((sum < one) && (sum != zero))
    {
        ap_ufixed<NBITS, 0> u_sum = sum;
        ap_ufixed<FRA_BITS, 0> frac = u_sum;
        ap_uint<FRA_BITS> raw = frac.range(FRA_BITS - 1, 0);
        int lzc;
#if FRA_BITS <= 32
        lzc = __builtin_clz((uint32_t)raw) - (32 - FRA_BITS);
#else
        lzc = __builtin_clzll((uint64_t)raw) - (64 - FRA_BITS);
#endif
        ap_fixed<NBITS, NINTG> sum_norm = sum << lzc;
        ap_ufixed<NBITS, 0> t = sum_norm;
        bool isPow2 = (t & (t - 1)) == 0;

        ap_fixed<NBITS, NINTG> inv_norm;
        if (isPow2)
            inv_norm = one;
        else
            inv_norm = (one / sum_norm);

        for (int i = 0; i < gf_size; i++)
            tab[i] = (tab[i] * inv_norm) << lzc;
    }
}

#endif
//
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//
