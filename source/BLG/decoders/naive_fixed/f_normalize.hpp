//
// Created by legal on 03/07/2025.
//
#pragma once
//
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//
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

    float sum2 =0.0f;
    for (int i = 0; i < gf_size; i += 1)
    {
        sum2 += float(tab[i]);
    }
    // printf("%f, %f\n", sum1, sum2);
}
//
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//
