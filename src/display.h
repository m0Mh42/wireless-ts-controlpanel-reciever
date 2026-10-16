#ifndef _H_DISPLAY
#define _H_DISPLAY 1

#include <Arduino.h>
#include "header.h"

// TM1638 display IC pins (Receiver: Pin 9->DIO, A4->CLK, A5->STB)
#ifndef DISPLAY_TM1638_DIO_PIN
#define DISPLAY_TM1638_DIO_PIN 9
#endif
#ifndef DISPLAY_TM1638_CLK_PIN
#define DISPLAY_TM1638_CLK_PIN A4
#endif
#ifndef DISPLAY_TM1638_STB_PIN
#define DISPLAY_TM1638_STB_PIN A5
#endif

// Indicator bitmask constants for horizontal status bar L (5 blocks: Green, Red, Yellow, Yellow, Yellow)
#define DISPLAY_IND_GREEN   0x01 // Block 1 (Green) - Comm Link OK
#define DISPLAY_IND_RED     0x02 // Block 2 (Red)   - Comm Link Fault / E-Stop / Radio Error
#define DISPLAY_IND_YEL1    0x04 // Block 3 (Yellow)
#define DISPLAY_IND_YEL2    0x08 // Block 4 (Yellow)
#define DISPLAY_IND_YEL3    0x10 // Block 5 (Yellow)

struct RadioRxDiagnostics;

void display_init();
void display_update(ts_status status, uint8_t active_unit, uint16_t buttons, bool link_healthy, uint8_t channel, uint8_t lqi, bool radio_error = false);
void display_show_packet_debug(const RadioRxDiagnostics& diag, uint8_t channel, uint8_t lqi, bool chip_connected);
void display_show_radio_error();
void display_clear();
void display_set_brightness(uint8_t brightness);
void display_set_top(const char* str);
void display_set_bottom(const char* str);
void display_set_bar(uint8_t bars);
inline void display_set_battery_bar(uint8_t bars) { display_set_bar(bars); }
void display_set_indicators(uint8_t mask);
void display_refresh();
void display_test_pattern();

// Access internal RAM buffer (for unit tests / validation)
const uint8_t* display_get_ram();

#endif // _H_DISPLAY
