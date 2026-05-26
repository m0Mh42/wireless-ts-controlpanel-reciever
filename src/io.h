#ifndef _H_IO
#define _H_IO 1

#include <Arduino.h>
#include "header.h"
#include "buttons.h"

#define PLCI_MUX1 0
#define PLCI_MUX2 1
#define PLCI_MUX3 2
#define PLCI_MUX4 3

#define PLCI_DISPLAYA 4
#define PLCI_DISPLAYB 5
#define PLCI_DISPLAYC 6
#define PLCI_DISPLAYD 7

#define PLCO_REGISTER 8
#define PLCO_ANILOX 9
#define PLCO_CLICHE 10
#define PLCO_COLOR 11

#define PLCO_FASTSTOP 12

uint8_t io_read_encode_mux();
uint8_t io_read_active_unit();
void io_write_buttons(uint16_t _buttons);
void io_write_faststop(bool _state);

#endif