#include "utils.h"

uint64_t powtwo(uint8_t pow)
{
    uint64_t num = 1;
    for (size_t i = 0; i < pow; i++)
    {
        num *= 2;
    }
    return num;
}