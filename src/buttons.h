#ifndef _H_BUTTONS
#define _H_BUTTONS 1

#include <Arduino.h>
#include "header.h"
#include "utils.h"

#define BUTTONS_ROWS 5
#define BUTTONS_COLUMNS 3

uint16_t buttons_process(uint8_t _buttons);

#endif