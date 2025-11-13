#ifndef CONFIG_CONSTANTS_H
#define CONFIG_CONSTANTS_H

// FAKE frozen bit vector !!!

#define _GF_    8
#define _logGF_ 3

#define _N_     64
#define _logN_  6

constexpr int reliab_seq[_N_] = {63, 62, 61, 59, 55, 47, 31, 60, 58, 57, 54, 51, 53, 46, 45, 43, 39, 30, 56, 52, 29, 50, 49, 27, 44, 42, 41, 23, 38, 37, 28, 26, 15, 48, 35, 25, 22, 40, 21, 19, 36, 14, 7, 11, 13, 34, 24, 33, 3, 20, 17, 5, 9, 18, 6, 10, 12, 1, 32, 2, 16, 4, 8, 0};

#endif