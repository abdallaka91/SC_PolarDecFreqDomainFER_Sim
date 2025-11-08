#include "utilities/utility_functions.hpp"

#include "definitions/custom_types.hpp"

void local_remove_xors(uint16_t *values, int size)
{
    if (size == 1)
        return;
    for (int i = 0; i < size / 2; i += 1)
        values[i] ^= values[i + size / 2];
    local_remove_xors(values, size / 2);
    local_remove_xors(values + size / 2, size / 2);
}

void remove_xors(uint16_t *values, int size)
{
    if (size == 1)
    {
        return;
    }
    for (int i = 0; i < size / 2; i += 1)
    {
        values[i] ^= values[i + size / 2];
    }
    remove_xors(values, size / 2);
    remove_xors(values + size / 2, size / 2);
}
