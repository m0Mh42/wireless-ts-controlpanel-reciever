#pragma once
#include <Arduino.h>
#include <stdint.h>
#include <cstring>
#include <cstdio>

extern char mock_lcd_buffer[128];
extern int mock_lcd_col;
extern int mock_lcd_row;
extern bool mock_lcd_cleared;
extern bool mock_lcd_backlight;
extern bool mock_lcd_init_called;

class LiquidCrystal_I2C {
public:
    LiquidCrystal_I2C(uint8_t addr, uint8_t cols, uint8_t rows) {}

    void init() {
        mock_lcd_init_called = true;
    }

    void backlight() {
        mock_lcd_backlight = true;
    }

    void noBacklight() {
        mock_lcd_backlight = false;
    }

    void clear() {
        mock_lcd_cleared = true;
        mock_lcd_buffer[0] = '\0';
        mock_lcd_col = 0;
        mock_lcd_row = 0;
    }

    void setCursor(uint8_t col, uint8_t row) {
        mock_lcd_col = col;
        mock_lcd_row = row;
    }

    void print(const char* s) {
        if (s) {
            strncpy(mock_lcd_buffer, s, sizeof(mock_lcd_buffer) - 1);
            mock_lcd_buffer[sizeof(mock_lcd_buffer) - 1] = '\0';
        }
    }

    void print(char c) {
        mock_lcd_buffer[0] = c;
        mock_lcd_buffer[1] = '\0';
    }

    void print(int n, int base = DEC) {
        if (base == HEX) {
            snprintf(mock_lcd_buffer, sizeof(mock_lcd_buffer), "%X", n);
        } else {
            snprintf(mock_lcd_buffer, sizeof(mock_lcd_buffer), "%d", n);
        }
    }

    void print(unsigned int n, int base = DEC) {
        if (base == HEX) {
            snprintf(mock_lcd_buffer, sizeof(mock_lcd_buffer), "%X", n);
        } else {
            snprintf(mock_lcd_buffer, sizeof(mock_lcd_buffer), "%u", n);
        }
    }

    void print(long n, int base = DEC) {
        if (base == HEX) {
            snprintf(mock_lcd_buffer, sizeof(mock_lcd_buffer), "%lX", n);
        } else {
            snprintf(mock_lcd_buffer, sizeof(mock_lcd_buffer), "%ld", n);
        }
    }

    void print(unsigned long n, int base = DEC) {
        if (base == HEX) {
            snprintf(mock_lcd_buffer, sizeof(mock_lcd_buffer), "%lX", n);
        } else {
            snprintf(mock_lcd_buffer, sizeof(mock_lcd_buffer), "%lu", n);
        }
    }

    void print(double n, int digits = 2) {
        snprintf(mock_lcd_buffer, sizeof(mock_lcd_buffer), "%.*f", digits, n);
    }
};
