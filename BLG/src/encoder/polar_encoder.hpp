#pragma once
#include <cstdint>
#include <cstdio>
#include <vector>

class polar_encoder {
public:
    polar_encoder(const int* vec, const int k, const int n);

    virtual ~polar_encoder();

    virtual void encode(uint16_t* dst, const uint16_t* src);

    virtual void decode(uint16_t* dst, const uint16_t* src);

private:
    const int K;
    const int N;
    std::vector<uint16_t> liste; // 16 is necessary when GF > 256 !
};
