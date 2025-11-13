#pragma once
#include <cstdint>

class nb_sc_dec{
public:
    virtual ~nb_sc_dec() = default;

    virtual int GF() = 0;

    virtual void execute(void* channel, uint16_t *  decoded) = 0;
};
