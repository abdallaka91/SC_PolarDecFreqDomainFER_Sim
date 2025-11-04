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
void f_normalize(ap_fixed<NBITS, NFRAC> * tab) {
    ap_fixed<NBITS, NFRAC> sum = 1e-24f;
    for (int i = 0; i < gf_size; i += 1) {
        sum += tab[i];
    }

    const ap_fixed<NBITS, NFRAC> zero = 0.f;
    if ( zero == sum ) {
        sum = 1.f;
    }

    const ap_fixed<NBITS, NFRAC> one    = 1.f;
    const ap_fixed<NBITS, NFRAC> factor = one / sum;
    for (int i = 0; i < gf_size; i++) {
        tab[i] *= factor;
    }
}
//
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//
