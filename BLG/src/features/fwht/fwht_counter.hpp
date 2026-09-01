#pragma once

#include <cstdint>

inline uint64_t & fwht_call_counter_ref() {
    static uint64_t counter = 0;
    return counter;
}

#define fwht_call_counter (fwht_call_counter_ref())

inline void reset_fwht_counter() {
    fwht_call_counter = 0;
}

inline uint64_t get_fwht_counter() {
    return fwht_call_counter;
}