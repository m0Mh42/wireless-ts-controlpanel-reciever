#include "test_tm1637.h"

TM1637 tm(TM1637_DIO_PIN, TM1637_CLK_PIN);

void tm1637_init()
{
    tm.begin();
}

void tm1637_show_status(bool ts_enable, uint8_t active_unit, uint8_t buttons)
{
    tm.setSegments(ts_enable, 4);
    tm.setDisplayDigit(active_unit);
    tm.setDisplayDigit(buttons, 1U);
}