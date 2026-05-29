#pragma once
#include <cstdint>
#include <cstddef>
#include <cassert>

/* ================================================================
 * Polynômes standards
 *
 * CRC-24/OpenPGP  : 0x864CFB
 * CRC-32/IEEE     : 0x04C11DB7
 * CRC-32C/Castagnoli : 0x1EDC6F41
 * ================================================================ */

/* ================================================================
 * Générateurs d'entrées de table — constexpr
 * ================================================================ */

constexpr uint32_t crc24_entry(uint32_t sym, int Q, uint32_t poly) {
    uint32_t crc = sym << (24 - Q);
    for (int i = 0; i < Q; i++)
        crc = (crc & 0x800000u) ? ((crc << 1) ^ poly) : (crc << 1);
    return crc & 0xFFFFFFu;
}

constexpr uint32_t crc32_entry(uint32_t sym, int Q, uint32_t poly) {
    uint32_t crc = sym << (32 - Q);
    for (int i = 0; i < Q; i++)
        crc = (crc & 0x80000000u) ? ((crc << 1) ^ poly) : (crc << 1);
    return crc;
}

/* ================================================================
 * Tables CRC-24 — constexpr, générées à la compilation
 * ================================================================ */

template<int Q, uint32_t POLY = 0x864CFBu>
struct CRC24Table {
    static_assert(Q >= 4 && Q <= 10);
    static constexpr int SIZE = 1 << Q;

    uint32_t t[SIZE];

    constexpr CRC24Table() : t{} {
        for (int i = 0; i < SIZE; i++)
            t[i] = crc24_entry((uint32_t)i, Q, POLY);
    }

    constexpr uint32_t update(uint32_t crc, uint16_t sym) const {
        uint32_t idx = ((crc >> (24 - Q)) ^ sym) & (SIZE - 1);
        return ((crc << Q) ^ t[idx]) & 0xFFFFFFu;
    }
};

/* ================================================================
 * Tables CRC-32 — constexpr, générées à la compilation
 * ================================================================ */

template<int Q, uint32_t POLY = 0x04C11DB7u>
struct CRC32Table {
    static_assert(Q >= 4 && Q <= 10);
    static constexpr int SIZE = 1 << Q;

    uint32_t t[SIZE];

    constexpr CRC32Table() : t{} {
        for (int i = 0; i < SIZE; i++)
            t[i] = crc32_entry((uint32_t)i, Q, POLY);
    }

    constexpr uint32_t update(uint32_t crc, uint16_t sym) const {
        uint32_t idx = ((crc >> (32 - Q)) ^ sym) & (SIZE - 1);
        return (crc << Q) ^ t[idx];
    }
};

/* ================================================================
 * Instances statiques constexpr — toutes en ROM
 * CRC-24
 * ================================================================ */

inline constexpr CRC24Table<4>  crc24_tab4;
inline constexpr CRC24Table<5>  crc24_tab5;
inline constexpr CRC24Table<6>  crc24_tab6;
inline constexpr CRC24Table<7>  crc24_tab7;
inline constexpr CRC24Table<8>  crc24_tab8;
inline constexpr CRC24Table<9>  crc24_tab9;
inline constexpr CRC24Table<10> crc24_tab10;

/* CRC-32 (IEEE 802.3) */
inline constexpr CRC32Table<4>  crc32_tab4;
inline constexpr CRC32Table<5>  crc32_tab5;
inline constexpr CRC32Table<6>  crc32_tab6;
inline constexpr CRC32Table<7>  crc32_tab7;
inline constexpr CRC32Table<8>  crc32_tab8;
inline constexpr CRC32Table<9>  crc32_tab9;
inline constexpr CRC32Table<10> crc32_tab10;

/* CRC-32C (Castagnoli) — variante pour usage interne */
inline constexpr CRC32Table<4,  0x1EDC6F41u> crc32c_tab4;
inline constexpr CRC32Table<5,  0x1EDC6F41u> crc32c_tab5;
inline constexpr CRC32Table<6,  0x1EDC6F41u> crc32c_tab6;
inline constexpr CRC32Table<7,  0x1EDC6F41u> crc32c_tab7;
inline constexpr CRC32Table<8,  0x1EDC6F41u> crc32c_tab8;
inline constexpr CRC32Table<9,  0x1EDC6F41u> crc32c_tab9;
inline constexpr CRC32Table<10, 0x1EDC6F41u> crc32c_tab10;

/* ================================================================
 * Sélecteur de table à la compilation
 * ================================================================ */

template<int Q>
constexpr const CRC24Table<Q>* get_crc24_table() {
    if constexpr      (Q == 4)  return &crc24_tab4;
    else if constexpr (Q == 5)  return &crc24_tab5;
    else if constexpr (Q == 6)  return &crc24_tab6;
    else if constexpr (Q == 7)  return &crc24_tab7;
    else if constexpr (Q == 8)  return &crc24_tab8;
    else if constexpr (Q == 9)  return &crc24_tab9;
    else                        return &crc24_tab10;
}

template<int Q, uint32_t POLY = 0x04C11DB7u>
constexpr const CRC32Table<Q, POLY>* get_crc32_table() {
    if constexpr (POLY == 0x1EDC6F41u) {
        if constexpr      (Q == 4)  return &crc32c_tab4;
        else if constexpr (Q == 5)  return &crc32c_tab5;
        else if constexpr (Q == 6)  return &crc32c_tab6;
        else if constexpr (Q == 7)  return &crc32c_tab7;
        else if constexpr (Q == 8)  return &crc32c_tab8;
        else if constexpr (Q == 9)  return &crc32c_tab9;
        else                        return &crc32c_tab10;
    } else {
        if constexpr      (Q == 4)  return &crc32_tab4;
        else if constexpr (Q == 5)  return &crc32_tab5;
        else if constexpr (Q == 6)  return &crc32_tab6;
        else if constexpr (Q == 7)  return &crc32_tab7;
        else if constexpr (Q == 8)  return &crc32_tab8;
        else if constexpr (Q == 9)  return &crc32_tab9;
        else                        return &crc32_tab10;
    }
}

/* ================================================================
 * Calcul CRC-24 sur symboles Q bits
 * ================================================================ */

template<int Q>
uint32_t crc24_symbols(const uint16_t* syms, int K,
                       uint32_t init = 0xB704CEu) {
    static_assert(Q >= 4 && Q <= 10);
    constexpr auto* tab = get_crc24_table<Q>();
    uint32_t crc = init;
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
 * Calcul CRC-32 sur symboles Q bits
 * ================================================================ */

template<int Q, uint32_t POLY = 0x04C11DB7u>
uint32_t crc32_symbols(const uint16_t* syms, int K,
                       uint32_t init = 0xFFFFFFFFu) {
    static_assert(Q >= 4 && Q <= 10);
    constexpr auto* tab = get_crc32_table<Q, POLY>();
    uint32_t crc = init;
    int i = 0;
    for (; i + 3 < K; i += 4) {
        crc = tab->update(crc, syms[i]);
        crc = tab->update(crc, syms[i+1]);
        crc = tab->update(crc, syms[i+2]);
        crc = tab->update(crc, syms[i+3]);
    }
    for (; i < K; i++)
        crc = tab->update(crc, syms[i]);
    return crc ^ 0xFFFFFFFFu;   /* finalisation XOR-out standard */
}

/* ================================================================
 * Insert / Verify CRC-24
 * Stockage dans ceil(24/Q) symboles
 * ================================================================ */

template<int Q>
int crc24_insert(uint16_t* syms, int K_data) {
    static_assert(Q >= 4 && Q <= 10);
    constexpr int    N_CRC = (23 / Q) + 1;   /* ceil(24/Q) */
    constexpr uint16_t MASK = (1u << Q) - 1;

    uint32_t crc = crc24_symbols<Q>(syms, K_data);

    int shift = 24 - Q;
    for (int i = 0; i < N_CRC; i++) {
        if (shift >= 0)
            syms[K_data + i] = (uint16_t)((crc >> shift) & MASK);
        else
            syms[K_data + i] = (uint16_t)((crc << -shift) & MASK);
        shift -= Q;
    }
    return N_CRC;
}

template<int Q>
bool crc24_verify(const uint16_t* syms, int K_data) {
    static_assert(Q >= 4 && Q <= 10);
    constexpr int    N_CRC = (23 / Q) + 1;
    constexpr uint16_t MASK = (1u << Q) - 1;

    uint32_t crc_calc = crc24_symbols<Q>(syms, K_data);

    uint32_t crc_stored = 0;
    int shift = 24 - Q;
    for (int i = 0; i < N_CRC; i++) {
        uint32_t s = syms[K_data + i] & MASK;
        if (shift >= 0) crc_stored |= s << shift;
        else            crc_stored |= s >> -shift;
        shift -= Q;
    }
    return crc_calc == crc_stored;
}

/* ================================================================
 * Insert / Verify CRC-32
 * Stockage dans ceil(32/Q) symboles
 * ================================================================ */

template<int Q, uint32_t POLY = 0x04C11DB7u>
int crc32_insert(uint16_t* syms, int K_data) {
    static_assert(Q >= 4 && Q <= 10);
    constexpr int    N_CRC = (31 / Q) + 1;   /* ceil(32/Q) */
    constexpr uint16_t MASK = (1u << Q) - 1;

    uint32_t crc = crc32_symbols<Q, POLY>(syms, K_data);

    int shift = 32 - Q;
    for (int i = 0; i < N_CRC; i++) {
        if (shift >= 0)
            syms[K_data + i] = (uint16_t)((crc >> shift) & MASK);
        else
            syms[K_data + i] = (uint16_t)((crc << -shift) & MASK);
        shift -= Q;
    }
    return N_CRC;
}

template<int Q, uint32_t POLY = 0x04C11DB7u>
bool crc32_verify(const uint16_t* syms, int K_data) {
    static_assert(Q >= 4 && Q <= 10);
    constexpr int    N_CRC = (31 / Q) + 1;
    constexpr uint16_t MASK = (1u << Q) - 1;

    uint32_t crc_calc = crc32_symbols<Q, POLY>(syms, K_data);

    uint32_t crc_stored = 0;
    int shift = 32 - Q;
    for (int i = 0; i < N_CRC; i++) {
        uint32_t s = syms[K_data + i] & MASK;
        if (shift >= 0) crc_stored |= s << shift;
        else            crc_stored |= s >> -shift;
        shift -= Q;
    }
    return crc_calc == crc_stored;
}

/* ================================================================
 * Dispatch dynamique
 * ================================================================ */

uint32_t crc24_symbols_dynamic(const uint16_t* s, int K, int Q,
                                uint32_t init = 0xB704CEu) {
    switch (Q) {
        case  4: return crc24_symbols<4> (s, K, init);
        case  5: return crc24_symbols<5> (s, K, init);
        case  6: return crc24_symbols<6> (s, K, init);
        case  7: return crc24_symbols<7> (s, K, init);
        case  8: return crc24_symbols<8> (s, K, init);
        case  9: return crc24_symbols<9> (s, K, init);
        case 10: return crc24_symbols<10>(s, K, init);
        default: assert(false); return 0;
    }
}

uint32_t crc32_symbols_dynamic(const uint16_t* s, int K, int Q,
                                uint32_t init = 0xFFFFFFFFu) {
    switch (Q) {
        case  4: return crc32_symbols<4> (s, K, init);
        case  5: return crc32_symbols<5> (s, K, init);
        case  6: return crc32_symbols<6> (s, K, init);
        case  7: return crc32_symbols<7> (s, K, init);
        case  8: return crc32_symbols<8> (s, K, init);
        case  9: return crc32_symbols<9> (s, K, init);
        case 10: return crc32_symbols<10>(s, K, init);
        default: assert(false); return 0;
    }
}
