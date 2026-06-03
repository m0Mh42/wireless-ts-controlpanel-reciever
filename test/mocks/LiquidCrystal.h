#pragma once
#include <Arduino.h>

// Mock state — set by tests, read by mock, checked by tests
extern char mock_lcd_buffer[16 * 4 + 1];  // virtual display content
extern int mock_lcd_col;
extern int mock_lcd_row;
extern bool mock_lcd_cleared;
extern int mock_lcd_last_int;

class LiquidCrystal {
public:
    LiquidCrystal(uint8_t rs, uint8_t enable,
                  uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3) {}
    void begin(uint8_t cols, uint8_t rows) {}
    void clear();
    void setCursor(uint8_t col, uint8_t row);
    void print(const char* s);
    void print(char c);
    void print(int n, int base = DEC);
    void print(unsigned int n, int base = DEC);
    void print(long n, int base = DEC);
    void print(unsigned long n, int base = DEC);
    void print(double n, int digits = 2);
};
