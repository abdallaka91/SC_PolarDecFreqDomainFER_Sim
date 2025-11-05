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
void normalize1(ap_fixed<NBITS, NINTG> *tab)
{
    ap_fixed<NBITS, NINTG> fact = 0.015625f;
    for (int i = 0; i < gf_size; i += 1)
    {
        tab[i]*=fact;
    }
}