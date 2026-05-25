#ifndef _H_UTILS
#define _H_UTILS 1

#include <Arduino.h>

uint64_t powtwo(uint8_t pow);
float mapfloat(float x, long in_min, long in_max, long out_min, long out_max);
int mapint(float x, long in_min, long in_max, long out_min, long out_max);

#endif