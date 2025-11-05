#pragma once

#include "custom_types.hpp"
#include "archi.hpp"

template <uint32_t gf_size>
void f_function(symbols_f * __restrict dst, symbols_f * __restrict src_a, symbols_f * __restrict src_b)
{
#ifdef _TEST_
    printf("(DD) f_function(--)\n");
#endif

    if (src_a->is_freq == false) // Switch from time to frequency domain
    {
        //
        // On ne normalise pas durant la FWHT CAR cela permet d'avoir des l'energie
        // du signal normalisée à 1 en sortie de la transformation
        //
        // fwht_norm<gf_size>(src_a->value);
        fwht<gf_size>(src_a->value);
        // printf("%f\n", float(src_a->value[0]));
        src_a->is_freq = true;
    }
#ifdef _TEST_
    if ( is_issue(src_a->value) > 1.f ) {
        printf("(EE) Error in %s %d\n", __FILE__, __LINE__);
        printf("(EE) value = %f\n", is_issue(src_a->value));
        exit(EXIT_FAILURE);
    }
#endif

    if (src_b->is_freq == false) // Switch from time to frequency domain
    {
        //
        // On ne normalise pas durant la FWHT CAR cela permet d'avoir des l'energie
        // du signal normalisée à 1 en sortie de la transformation
        //
        fwht<gf_size>(src_b->value);
        // printf("%f\n", float(src_b->value[0]));
        src_b->is_freq = true;
    }

#ifdef _TEST_
    if ( is_issue(src_b->value) > 1.f ) {
        printf("(EE) Error in %s %d\n", __FILE__, __LINE__);
        printf("(EE) value = %f\n", is_issue(src_b->value));
        exit(EXIT_FAILURE);
    }
#endif

    //
    // Element-wise multiplication of the two input symbols because we are in frequency domain !
    //
#if 0
    const ap_fixed<NBITS, NINTG> fact = 8.f; // TODO : attention au facteur 10x qui est magique !!!
    for (size_t i = 0; i < gf_size; i++) {
        dst->value[i] =  fact * src_a->value[i] * src_b->value[i];
    }
#else
    for (size_t i = 0; i < gf_size; i++) {
        dst->value[i] =  src_a->value[i] * src_b->value[i];
    }
    // printf("%f\n", float(dst->value[0]));

    
    // Plus besoin de normaliser car l'energie qui est
    // egale à 1 est mulitpliée par 1 => 1
    // on est normalisé par construction !
//    const float fact = 1.f / dst->value[0].to_float();
//    const ap_fixed<16, 16> ffact = fact;    // should be drastically reduced !
//    for (size_t i = 0; i < gf_size; i++) {
//        dst->value[i] = ffact * dst->value[i];
//    }
#endif
     dst->is_freq = true; // a.a we do CN in FD

#ifdef _TEST_
    if ( is_issue(dst->value) > 1.f ) {
        printf("(EE) Error in %s %d\n", __FILE__, __LINE__);
        printf("(EE) value = %f\n", is_issue(dst->value));
        exit(EXIT_FAILURE);
    }
#endif

}
//
//
//
//
//
