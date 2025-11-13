#include "polar_encoder.hpp"
#include "encoder_1.hpp"

polar_encoder::polar_encoder(const int *vec, const int k, const int n) : K(k), N(n) {
    liste.resize(N);
    for (int i = 0; i < N; i++)
        liste[i] = vec[i];
}

polar_encoder::~polar_encoder() {
}

void polar_encoder::encode(uint16_t *dst, const uint16_t *src) {
    for (int i = 0; i < K; i++)
        dst[liste[i]] = src[i];
    for (int i = K; i < N; i++)
        dst[liste[i]] = 0;

         if (N ==    8) polar_encode<   8>(dst);
    else if (N ==   16) polar_encode<  16>(dst);
    else if (N ==   32) polar_encode<  32>(dst);
    else if (N ==   64) polar_encode<  64>(dst);
    else if (N ==  128) polar_encode< 128>(dst);
    else if (N ==  256) polar_encode< 256>(dst);
    else if (N ==  512) polar_encode< 512>(dst);
    else if (N == 1024) polar_encode<1024>(dst);
    else if (N == 2048) polar_encode<2048>(dst);
    else if (N == 4096) polar_encode<4096>(dst);
    else {
        printf("(EE) The encoder does not support N = %d value...\n", N);
        printf("(EE) %s %d\n", __FILE__, __LINE__);
        exit(EXIT_FAILURE);
    }
}

void polar_encoder::decode(uint16_t *dst, const uint16_t *src) {
    for (int i = 0; i < K; i++)
        dst[i] = src[liste[i]];
}
