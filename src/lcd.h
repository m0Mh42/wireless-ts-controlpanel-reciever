#ifndef _H_LCD
#define _H_LCD 1

#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include "header.h"

void lcd_init();
void lcd_update_status(ts_status status, uint8_t active_unit, uint16_t buttons, bool rx);

#endif
