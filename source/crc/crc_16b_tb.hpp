#include <cstdio>

int main() {
    /* Q=6, K_data=12 symboles de données + 3 symboles CRC */
    constexpr int Q      = 6;
    constexpr int K_data = 12;
    constexpr int N_CRC  = (15/Q)+1;   /* = 3 */
    constexpr int K_tot  = K_data + N_CRC;

    uint16_t syms[K_tot] = {
        0x3F, 0x01, 0x2A, 0x15, 0x00, 0x3F,
        0x10, 0x07, 0x38, 0x0C, 0x21, 0x3E,
        0, 0, 0   /* réservé CRC */
    };

    /* Calcul et insertion du CRC */
    crc16_insert<Q>(syms, K_data);
    printf("CRC stocké dans syms[%d..%d] : ", K_data, K_tot-1);
    for (int i = K_data; i < K_tot; i++)
        printf("0x%02X ", syms[i]);
    printf("\n");

    /* Vérification */
    printf("Verify OK  : %s\n", crc16_verify<Q>(syms, K_data) ? "OUI" : "NON");

    /* Corruption */
    syms[3] ^= 0x01;
    printf("Verify KO  : %s\n", crc16_verify<Q>(syms, K_data) ? "OUI" : "NON");

    return 0;
}
