#pragma once

#include "f_argmax.hpp"
#include "f_fwht_norm.hpp"
#include "f_normalize.hpp"

//#define _TEST_

template <int gf_size>
void g_function(
    symbols_f * __restrict dst,   // the data to be computed for the left side of the graph
    symbols_f * __restrict src_a, // the upper value set from the right side of the graph
    symbols_f * __restrict src_b, // the lower value set from the right side of the graph
    uint32_t    src_c)    // the computed symbols coming from the left side of the graph
{
#ifdef _TEST_
    if ( is_issue(src_a->value) > 1.f ) {
        printf("(EE) Error in %s %d\n", __FILE__, __LINE__);
        printf("(EE) value = %f\n", is_issue(src_a->value));
        exit(EXIT_FAILURE);
    }
#endif

    if (src_a->is_freq == true)
    {
        //
        // Pour eviter que la dynamique explose en interne dans la FWHT, il
        // faut appliquer la normalisation que l'on n'a pas fait en sortie
        // de la FWHT (fonction F)
        //
        const ap_fixed<NBITS, NFRAC> factor = norm_factor_lwht<gf_size>();
        for (size_t i = 0; i < gf_size; i++) {
            src_a->value[i] = src_a->value[i] * factor;
        }
        // traitement normal en suivant
        fwht_norm<gf_size>(src_a->value);
        src_a->is_freq = false;
    }

#ifdef _TEST_
    if ( is_issue(src_a->value) > 1.f ) {
        printf("(EE) Error in %s %d\n", __FILE__, __LINE__);
        printf("(EE) value = %f\n", is_issue(src_a->value));
        exit(EXIT_FAILURE);
    }
#endif

    if (src_b->is_freq == true) {
        //
        // Pour eviter que la dynamique explose en interne dans la FWHT, il
        // faut appliquer la normalisation que l'on n'a pas fait en sortie
        // de la FWHT (fonction F)
        //
        const ap_fixed<NBITS, NFRAC> factor = norm_factor_lwht<gf_size>();
        for (size_t i = 0; i < gf_size; i++) {
            src_b->value[i] = src_b->value[i] * factor;
        }
        // traitement normal en suivant
        fwht_norm<gf_size>(src_b->value);
        src_b->is_freq = false;
    }

#ifdef _TEST_
    if ( is_issue(src_b->value) > 1.f ) {
        printf("(EE) Error in %s %d\n", __FILE__, __LINE__);
        exit(EXIT_FAILURE);
    }
#endif

    for (size_t i = 0; i < gf_size; i++) {
        const int idx   = src_c ^ i;
        dst->value[idx] = src_a->value[i] * src_b->value[idx];
    }

#ifdef _TEST_
    if ( is_issue(dst->value) > 1.f ) {
        printf("(EE) Error in %s %d\n", __FILE__, __LINE__);
        exit(EXIT_FAILURE);
    }
#endif

    f_normalize<gf_size>(dst->value); // temporal

#ifdef _TEST_
    if ( is_issue(dst->value) > 1.f ) {
        printf("(EE) Error in %s %d\n", __FILE__, __LINE__);
        exit(EXIT_FAILURE);
    }
#endif

    dst->is_freq = false;
}
//
//
//
//
//
