#pragma once
#include "definitions/custom_types.hpp"
//
//
//
//
//
class decoder {
public:
    virtual ~decoder() {};
    virtual void execute(void * channel, uint16_t * decoded, uint16_t * ksymb, float * entrop, float * one_err_prob) = 0;
    virtual int  GF()                                                                                                = 0;
};
//
//
//
//
//
