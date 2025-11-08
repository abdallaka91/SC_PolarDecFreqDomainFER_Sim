#include "decoder_dedicated.hpp"

/**
 *
 * @param n
 * @param frozen_symb
 */
template <int gf_size>
decoder_dedicated<gf_size>::decoder_dedicated(const int n, const int* frozen_symb)
{
    internal = new symbols_t[n];
    symbols  = new uint16_t [n];

    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += (frozen_symb[i] == false);
    }

    if ( N_gen != n ) {
        printf("(EE) Error we should never be there...\n");
        printf("(EE) %s %d\n", __FILE__, __LINE__);
        printf("(EE) An issue was detected on N value between generated decoder and simulated one:\n");
        printf("(EE) - N value in generated decoder  = %d\n", N_gen);
        printf("(EE) - N value in simulated environ. = %d\n", n);
        exit( EXIT_FAILURE );
    }

    if ( K_gen != sum ) {
        printf("(EE) Error we should never be there...\n");
        printf("(EE) %s %d\n", __FILE__, __LINE__);
        printf("(EE) An issue was detected on K value between generated decoder and simulated one:\n");
        printf("(EE) - K value in generated decoder  = %d\n", K_gen);
        printf("(EE) - K value in simulated environ. = %d\n", sum);
        exit( EXIT_FAILURE );
    }
}

/**
 *
 */
template <int gf_size>
decoder_dedicated<gf_size>::decoder_dedicated()
{
    internal = nullptr;
    symbols  = nullptr;
    printf("(EE) Error we should never be there...\n");
    printf("(EE) %s %d\n", __FILE__, __LINE__);
    exit(EXIT_FAILURE);
}

template <int gf_size>
decoder_dedicated<gf_size>::~decoder_dedicated()
{
    delete[]internal;
    delete[]symbols;
}

#include "dedicated_execute.hpp"

//
//
//
#if _GF_ == 8
    template class decoder_dedicated< 8>;
#elif _GF_ == 16
    template class decoder_dedicated< 16>;
#elif _GF_ == 32
    template class decoder_dedicated< 32>;
#elif _GF_ == 64
    template class decoder_dedicated< 64>;
#elif _GF_ == 128
    template class decoder_dedicated<128>;
#elif _GF_ == 256
    template class decoder_dedicated<256>;
#elif _GF_ == 512
    template class decoder_dedicated<512>;
#elif _GF_ == 1024
    template class decoder_dedicated<1024>;
#endif