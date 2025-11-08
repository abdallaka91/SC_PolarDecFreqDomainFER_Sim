#pragma once
#include "definitions/custom_types.hpp"
//
//
//
//
//
class decoder
{
public:
    virtual ~decoder() {};
    virtual void execute(symbols_t *channel, uint16_t *decoded) = 0;
};
//
//
//
//
//
