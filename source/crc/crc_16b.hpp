#pragma once
#include <cstdint>
#include <cstddef>
#include <cassert>
#include <array>

/* ================================================================
 * Polynôme CRC-16/CCITT (0x1021), traitement MSB-first
 * Table de 2^Q entrées générée à la compilation (constexpr)
 * ================================================================ */

/* --- Génération d'une entrée de table pour Q bits --- */
constexpr uint16_t crc16_entry(uint16_t sym, int Q, uint16_t poly) {
    /* Place le symbole dans les Q bits de poids fort */
    uint16_t crc = (uint16_t)(sym << (16 - Q));
    for (int i = 0; i < Q; i++)
        crc = (crc & 0x8000) ? (uint16_t)((crc << 1) ^ poly)
                             : (uint16_t) (crc << 1);
    return crc;
}

/* --- Table complète 2^Q entrées --- */
template<int Q>
struct CRC16Table {
    static_assert(Q >= 4 && Q <= 10);
    static constexpr int    SIZE = 1 << Q;
    static constexpr uint16_t POLY = 0x1021;

    uint16_t t[SIZE];

    constexpr CRC16Table() : t{} {
        for (int i = 0; i < SIZE; i++)
            t[i] = crc16_entry((uint16_t)i, Q, POLY);
    }

    /* Mise à jour du CRC par un symbole Q bits */
    constexpr uint16_t update(uint16_t crc, uint16_t sym) const {
        /* XOR le symbole avec les Q bits de poids fort du CRC */
        uint16_t idx = ((crc >> (16 - Q)) ^ sym) & (SIZE - 1);
        return (uint16_t)((crc << Q) ^ t[idx]);
    }
};

/* Tables statiques constexpr — générées à la compilation, en ROM */
inline constexpr CRC16Table<4>  crc16_tab4;
inline constexpr CRC16Table<5>  crc16_tab5;
inline constexpr CRC16Table<6>  crc16_tab6;
inline constexpr CRC16Table<7>  crc16_tab7;
inline constexpr CRC16Table<8>  crc16_tab8;
inline constexpr CRC16Table<9>  crc16_tab9;
inline constexpr CRC16Table<10> crc16_tab10;

/* ================================================================
 * Calcul CRC-16 sur tableau de symboles Q bits (uint16_t[])
 * Template : Q connu à la compilation
 * ================================================================ */

template<int Q>
uint16_t crc16_symbols(const uint16_t* syms, int K,
                       uint16_t init = 0xFFFF) {
    static_assert(Q >= 4 && Q <= 10);

    /* Sélection de la bonne table (résolu à la compilation) */
    constexpr const CRC16Table<Q>* tab = [] {
        if constexpr      (Q == 4)  return &crc16_tab4;
        else if constexpr (Q == 5)  return &crc16_tab5;
        else if constexpr (Q == 6)  return &crc16_tab6;
        else if constexpr (Q == 7)  return &crc16_tab7;
        else if constexpr (Q == 8)  return &crc16_tab8;
        else if constexpr (Q == 9)  return &crc16_tab9;
        else                        return &crc16_tab10;
    }();

    uint16_t crc = init;

    /* Déroulage x4 pour réduire la pression sur le prédicteur */
    int i = 0;
    for (; i + 3 < K; i += 4) {
        crc = tab->update(crc, syms[i]);
        crc = tab->update(crc, syms[i+1]);
        crc = tab->update(crc, syms[i+2]);
        crc = tab->update(crc, syms[i+3]);
    }
    for (; i < K; i++)
        crc = tab->update(crc, syms[i]);

    return crc;
}

/* ================================================================
 * Finalisation : insert / verify dans le tableau de symboles
 * Le CRC 16 bits est stocké dans les derniers ceil(16/Q) symboles
 * ================================================================ */

template<int Q>
int crc16_insert(uint16_t* syms, int K_data) {
    static_assert(Q >= 4 && Q <= 10);
    constexpr int N_CRC  = (15 / Q) + 1;   /* ceil(16/Q) */
    constexpr uint16_t MASK = (1u << Q) - 1;

    uint16_t crc = crc16_symbols<Q>(syms, K_data - N_CRC);

    /* Stockage MSB-first dans les N_CRC symboles suivants */
    int shift = 16 - Q;
    for (int i = 0; i < N_CRC; i++) {
        if (shift >= 0)
            syms[K_data + i - N_CRC] = (crc >> shift) & MASK;
        else
            syms[K_data + i - N_CRC] = (crc << -shift) & MASK;
        shift -= Q;
    }
    return N_CRC;
}

template<int Q>
bool crc16_verify(const uint16_t* syms, int K_data) {
    static_assert(Q >= 4 && Q <= 10);
    constexpr int N_CRC  = (15 / Q) + 1;
    constexpr uint16_t MASK = (1u << Q) - 1;

    uint16_t crc_calc = crc16_symbols<Q>(syms, K_data - N_CRC);

    /* Lecture du CRC stocké */
    uint16_t crc_stored = 0;
    int shift = 16 - Q;
    for (int i = 0; i < N_CRC; i++) {
        uint16_t s = syms[K_data + i - N_CRC] & MASK;
        if (shift >= 0) crc_stored |= (uint16_t)(s << shift);
        else            crc_stored |= (uint16_t)(s >> -shift);
        shift -= Q;
    }
    return crc_calc == crc_stored;
}

/* ================================================================
 * Dispatch dynamique (Q connu au runtime)
 * ================================================================ */
//
//
void crc16_insert(uint16_t* data, const int K, const int q)
{
	     if( q ==   16 ) crc16_insert< 4>(data, K);
	else if( q ==   32 ) crc16_insert< 5>(data, K);
	else if( q ==   64 ) crc16_insert< 6>(data, K);
	else if( q ==  128 ) crc16_insert< 7>(data, K);
	else if( q ==  256 ) crc16_insert< 8>(data, K);
	else if( q ==  512 ) crc16_insert< 9>(data, K);
	else if( q == 1024 ) crc16_insert<10>(data, K);
	else exit(EXIT_FAILURE);
}
//
//
bool crc16_verify(const uint16_t* data, const int K, const int q)
{
    bool ok;
	     if( q ==   16 ) ok = crc16_verify< 4>(data, K);
	else if( q ==   32 ) ok = crc16_verify< 5>(data, K);
	else if( q ==   64 ) ok = crc16_verify< 6>(data, K);
	else if( q ==  128 ) ok = crc16_verify< 7>(data, K);
	else if( q ==  256 ) ok = crc16_verify< 8>(data, K);
	else if( q ==  512 ) ok = crc16_verify< 9>(data, K);
	else if( q == 1024 ) ok = crc16_verify<10>(data, K);
	else exit(EXIT_FAILURE);
    return ok;
}
//
//
