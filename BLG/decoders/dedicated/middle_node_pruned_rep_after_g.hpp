#pragma once
//
//
//
//
//
#include "decoders/features/archi.hpp"
#include "definitions/custom_types.hpp"
//
//
//
//
//
extern void local_remove_xors(uint16_t *values, int size);

template <int gf_size>
void middle_node_pruned_rep_after_g(
    symbols_t *__restrict inputs, // Inputs are the symbols from the channel (from the right)
    uint16_t *__restrict decoded, // Decoded symbols are the final output of the decoder (done on the left)
    uint16_t *__restrict symbols, // Symbols are the ones going from leafs to root (done on the left)
    const int size                // Size is the number of symbols (should be a power of 2)
)
{
    // ON PEUT REUTILISER LE SYMBOL n°0 POUR LES CALCULS
    // + NORMALISATION A LA VOLEE
#if 1 // optimization
    float temp[gf_size];
    for (int j = 0; j < gf_size; j++)
        temp[j] = inputs[0].value[j] * inputs[1].value[j];

    for (int i = 2; i < size; i++)
    {
        if ((i & 0x1) == 1)
            normalize<gf_size>(temp);
        for (int j = 0; j < gf_size; j++)
            temp[j] *= inputs[i].value[j];
    }
#else
    float temp[gf_size];
    for (int j = 0; j < gf_size; j++)
        temp[j] = inputs[0].value[j] * inputs[1].value[j];

    for (int i = 2; i < size; i += 2)
    {
        float sum = 0.f;
        for (int j = 0; j < gf_size; j++)
        {
            const float val = temp[j] * inputs[i].value[j];
            temp[j] = val;
            sum += val;
        }
        const float inv = 1.f / sum;
        for (int j = 0; j < gf_size; j++)
            temp[j] *= inputs[i + 1].value[j] * inv;
    }
#endif
    const int value = argmax<gf_size>(temp);

    for (int i = 0; i < size; i++)
    {
        symbols[i] = value;
        decoded[i] = 0; // should be corrected (it is systematic solution actually)
    }
    decoded[size - 1] = value; // should be corrected (it is systematic solution actually)
}
