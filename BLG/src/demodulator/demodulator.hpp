#pragma once

#include <cstdint>
#include <cstdio>
#include <vector>

#include "definitions/custom_types.hpp"

template<int gf_size>
class demodulator {
public:
    demodulator(const int n) : N(n)
    {
    }

    void demodulate(symbols_t* dst, const uint16_t* src) const {
        for (int i = 0; i < N; i++)
        {
            dst[i].is_freq = false;
            for (int j = 0; j < gf_size; j++)    // toutes les probabilités à zero
                dst[i].value[j] = 0.f;      // pour l'initialisation
            dst[i].value[ src[i] ] = 1.f;   // on met le bon symbole à 100%
            //printf("Processing symbol %2d\n", i);
            //show_symbols<gf_size>( dst[i].value );
        }
    }

private:
    const int N;
};
