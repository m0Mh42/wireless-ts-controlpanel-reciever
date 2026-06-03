#include "utils.h"

uint64_t powtwo(uint8_t pow)
{
    uint16_t mask = 1 << pow;
    return mask;
}

float mapfloat(float x, long in_min, long in_max, long out_min, long out_max)
{
    return (float)(x - in_min) * (float)(out_max - out_min) / (float)(in_max - in_min) + out_min;
}

int mapint(float x, long in_min, long in_max, long out_min, long out_max)
{
    return (int)((float)(x - in_min) * (float)(out_max - out_min) / (float)(in_max - in_min) + out_min);
}