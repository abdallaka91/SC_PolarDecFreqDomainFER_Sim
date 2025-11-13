#pragma once
#include <algorithm>
#include <cstdio>

#include "definitions/custom_types.hpp"
#include "features/archi.hpp"

void local_remove_xors(uint16_t * values, int size);

template <int gf_size>
void show_symbols(const symbols_t * symbols) {
    if (symbols->is_freq) {
        printf("(DD) Symbols [freq]:");
    } else {
        printf("(DD) Symbols [prob]:");
    }

    float sum = 0.f;
    for (int i = 0; i < gf_size; i += 1) {
        if ((i % 16) == 0)
            printf("\n(DD)  %2d : ", i);
        if (symbols->value[i] >= 0.2f)
            printf("\e[1;32m%+6.4f\e[0m ", symbols->value[i]);
        else
            printf("%+6.4f ", symbols->value[i]);
        sum += symbols->value[i];
    }
    const int symb = argmax<gf_size>(symbols->value);
    printf("\n(DD)     : sum = %f | argmax = %d\n", sum, symb);
}

template <int gf_size> inline __attribute__((always_inline)) int position(const float* vector, float value)
{
    for (int i = 0; i < gf_size; i++) {
        if (vector[i] == value) return i;
    }
    return -1;
}

template <int gf_size>
void show_symbols(const float* symbols) {
    //
    // On trie les probabilites pour extraire les 3 mins
    //
    float tmp[gf_size];
    for (int i = 0; i < gf_size; i++) tmp[i] = symbols[i];
    std::sort(tmp, tmp + gf_size);

    float sum = 0.f;
    for (int i = 0; i < gf_size; i += 1) {
        if ((i % 16) == 0)
            printf("\n(DD)  %2d : ", i);
        if (symbols[i] >= tmp[gf_size-1])
            printf("\e[1;32m%+6.4f\e[0m ", symbols[i]);
        else if (symbols[i] >= tmp[gf_size-2])
            printf("\e[1;33m%+6.4f\e[0m ", symbols[i]);
        else if (symbols[i] >= tmp[gf_size-3])
            printf("\e[1;31m%+6.4f\e[0m ", symbols[i]);
        else
            printf("%+6.4f ", symbols[i]);
        sum += symbols[i];
    }
    const int symb = argmax  <gf_size>(symbols);
    const int arg2 = position<gf_size>(symbols, tmp[gf_size-2]);
    const int arg3 = position<gf_size>(symbols, tmp[gf_size-3]);
    printf("\n(DD)     : sum = %f | argmax = %d (%f) [2: %d (%f)] [3: %d (%f)]\n", sum, symb, symbols[symb], arg2, tmp[gf_size-2], arg3, tmp[gf_size-3]);
}
/*
void show_symbols(const float* symbols, int gf_size) {
    //
    // On trie les probabilites pour extraire les 3 mins
    //
    float sum = 0.f;
    for (int i = 0; i < gf_size; i += 1) {
        if ((i % 16) == 0)
            printf("\n(DD)  %2d : ", i);
        printf("%+6.4f ", symbols[i]);
        sum += symbols[i];
    }
}
*/

template <int gf_size>
void show_symbols(const double * symbols) {
    double sum = 0.f;
    for (int i = 0; i < gf_size; i += 1) {
        if ((i % 16) == 0)
            printf("\n(DD)  %2d : ", i);
        if (symbols[i] >= 0.2f)
            printf("\e[1;32m%+6.4lf\e[0m ", symbols[i]);
        else
            printf("%+6.4f ", symbols[i]);
        sum += symbols[i];
    }
    const int symb = argmax<gf_size>(symbols);
    printf("\n(DD)     : sum = %lf | argmax = %d\n", sum, symb);
}
