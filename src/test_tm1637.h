#ifndef _H_TM1637_TEST
#define _H_TM1637_TEST

#include "header.h"
#include <TM1637.h>

#define TM1637_DIO_PIN A0
#define TM1637_CLK_PIN A1

void tm1637_init();
void tm1637_show_status(bool ts_enable, uint8_t active_unit, uint8_t button_two_digit);

#endif