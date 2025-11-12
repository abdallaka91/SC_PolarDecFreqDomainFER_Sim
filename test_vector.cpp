#include "BLG/decoders/dedicated/f_function_freq_in.hpp"
#include <iostream>

int main()
{
    constexpr int GF = 64;
    constexpr int N = 8;

    symbols_t dst[N];
    symbols_t src_a[N];
    symbols_t src_b[N];

    // initialize to avoid undefined data
    for (int s = 0; s < N; ++s)
        for (int i = 0; i < GF; ++i)
        {
            src_a[s].value[i] = i + 1;
            src_b[s].value[i] = i + 2;
        }

    f_function_freq_in<GF, N>(dst, src_a, src_b);

    // use the result to prevent optimization
    float checksum = 0;
    for (int s = 0; s < N; ++s)
        for (int i = 0; i < GF; ++i)
            checksum += dst[s].value[i];

    std::cout << checksum << std::endl;
    return 0;
}
