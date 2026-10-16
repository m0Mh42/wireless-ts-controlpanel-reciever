#pragma once
#include <stdint.h>
#include <cstring>
#include <Arduino.h>

extern uint32_t mock_tm1638_buttons;
extern char mock_tm1638_display_str[32];
extern bool mock_tm1638_cleared;
extern bool mock_tm1638_init_called;

class TM1638 {
public:
    TM1638(uint8_t dio, uint8_t clk, uint8_t stb,
           uint8_t digits = 8, bool activate = true, uint8_t intensity = 7) {}

    void begin(bool activate = true, uint8_t intensity = 7) {
        mock_tm1638_init_called = true;
    }

    void clearDisplay() {
        mock_tm1638_cleared = true;
        mock_tm1638_display_str[0] = '\0';
    }

    void setDisplayDigit(uint8_t digit, uint8_t pos, bool dot = false) {
        if (pos < 16) {
            mock_tm1638_display_str[pos] = (char)('0' + (digit % 10));
            mock_tm1638_display_str[pos + 1] = '\0';
        }
    }

    void setDisplayToString(const char* str) {
        if (str) {
            strncpy(mock_tm1638_display_str, str, sizeof(mock_tm1638_display_str) - 1);
            mock_tm1638_display_str[sizeof(mock_tm1638_display_str) - 1] = '\0';
        }
    }

    uint32_t getButtons() { return mock_tm1638_buttons; }
};
