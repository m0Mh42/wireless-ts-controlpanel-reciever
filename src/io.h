#ifndef _H_IO
#define _H_IO 1

#include "header.h"
#include <Arduino.h>

// UTECO Machine PLC Inputs (Active Unit BCD)
#define PLCI_DISPLAYA 2
#define PLCI_DISPLAYB 3
#define PLCI_DISPLAYC 4
#define PLCI_DISPLAYD 5

// UTECO Machine PLC Inputs (Time-Multiplexed Scan Slices)
#define PLCI_MUX1 6
#define PLCI_MUX2 7
#define PLCI_MUX3 A7 // Note: Dedicated analog pin on ATmega328P; sampled via analogRead threshold
#define PLCI_MUX4 A6 // Note: Dedicated analog pin on ATmega328P; sampled via analogRead threshold

// UTECO Machine PLC Outputs (Button Actuators)
#define PLCO_REGISTER A0
#define PLCO_ANILOX A1
#define PLCO_CLICHE A2
#define PLCO_COLOR A3

// Dedicated Safety Lines
#define PLCO_FASTSTOP 1
#define PLCO_ENABLE 0

void io_init();
uint8_t io_read_encode_mux();
uint8_t io_read_active_unit();
void io_write_buttons(uint16_t _buttons);
void io_write_faststop(bool _state);
void io_write_enable(bool _state);
void io_clear_all_buttons();

#endif // _H_IO
