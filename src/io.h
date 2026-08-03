#ifndef _H_IO
#define _H_IO 1

#include "buttons.h"
#include "header.h"
#include <Arduino.h>

#define PLCI_MUX1 A0
#define PLCI_MUX2 A1
#define PLCI_MUX3 A2
#define PLCI_MUX4 A3

#define PLCI_DISPLAYA A4
#define PLCI_DISPLAYB A5
#define PLCI_DISPLAYC 2
#define PLCI_DISPLAYD 3

#define PLCO_REGISTER 4
#define PLCO_ANILOX 5
#define PLCO_CLICHE 6
#define PLCO_COLOR 7

#define PLCO_FASTSTOP 8

uint8_t io_read_encode_mux();
uint8_t io_read_active_unit();
void io_write_buttons(uint16_t _buttons);
void io_write_faststop(bool _state);

#endif
