#include <cstdio>

int main() {
    constexpr int Q      = 6;
    constexpr int K_data = 12;

    uint16_t syms[K_data + 8] = {     /* +8 : marge pour tous les CRC */
        0x3F,0x01,0x2A,0x15,0x00,0x3F,0x10,0x07,0x38,0x0C,0x21,0x3E
    };

    /* --- CRC-24 --- */
    int n24 = crc24_insert<Q>(syms, K_data);
    printf("CRC-24 (%d symboles) : ", n24);
    for (int i = 0; i < n24; i++) printf("0x%02X ", syms[K_data+i]);
    printf("\nVerify : %s\n", crc24_verify<Q>(syms, K_data) ? "OK" : "ERREUR");

    syms[2] ^= 1;
    printf("Après corruption : %s\n\n",
           crc24_verify<Q>(syms, K_data) ? "OK" : "ERREUR");
    syms[2] ^= 1; /* restaure */

    /* --- CRC-32 IEEE --- */
    int n32 = crc32_insert<Q>(syms, K_data);
    printf("CRC-32 (%d symboles) : ", n32);
    for (int i = 0; i < n32; i++) printf("0x%02X ", syms[K_data+i]);
    printf("\nVerify : %s\n", crc32_verify<Q>(syms, K_data) ? "OK" : "ERREUR");

    /* --- CRC-32C Castagnoli --- */
    int n32c = crc32_insert<Q, 0x1EDC6F41u>(syms, K_data);
    printf("CRC-32C (%d symboles) : ", n32c);
    for (int i = 0; i < n32c; i++) printf("0x%02X ", syms[K_data+i]);
    printf("\nVerify : %s\n",
           crc32_verify<Q, 0x1EDC6F41u>(syms, K_data) ? "OK" : "ERREUR");

    return 0;
}