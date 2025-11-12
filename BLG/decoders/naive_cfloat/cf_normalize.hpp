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
void f_normalize(flx::floatx<N_EXPO, N_MANT> *tab)
{
    flx::floatx<N_EXPO, N_MANT> sum = 1e-24f;
    for (int i = 0; i < gf_size; i += 1)
    {
        sum += tab[i];
    }
    const flx::floatx<N_EXPO, N_MANT> zero = 0.f;
    if (zero == sum)
    {
        sum = 1.f;
    }
    for (int i = 0; i < gf_size; i++)
    {
        tab[i] /= sum;
    }
}
#if 0
template <int gf_size>
void f_normalize(flx::floatx<N_EXPO, N_MANT> * tab) {
    double sum = 0.0;
    for (int i = 0; i < gf_size; i += 1) {
        sum += tab[i].to_double();
    }
    sum = ( sum == 0.0 ) ? 1.f : sum;

    for (int i = 0; i < gf_size; i++) {
        flx::floatx<N_EXPO, N_MANT> vv = tab[i].to_double() / sum;
        tab[i] = vv;
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
template <int gf_size>
void f_normalize(flx::floatx<N_EXPO, N_MANT> * tab, const float norm) {
    const flx::floatx<N_EXPO, N_MANT> factor = norm;
    for (int i = 0; i < gf_size; i++)
    {
        const flx::floatx<N_EXPO, N_MANT> ff = norm * double( tab[i] );
        tab[i] = ff;
    }
}
//
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//
