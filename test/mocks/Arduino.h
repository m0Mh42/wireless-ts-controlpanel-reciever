#pragma once
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <cstdio>

typedef bool boolean;

#define HIGH 0x1
#define LOW 0x0
#define INPUT 0x0
#define OUTPUT 0x1
#define INPUT_PULLUP 0x2
#define LED_BUILTIN 13

#define DEC 10
#define HEX 16
#define OCT 8
#define BIN 2

// Analog pin aliases (matching Arduino Uno)
#define A0 14
#define A1 15
#define A2 16
#define A3 17
#define A4 18
#define A5 19

// Mock state — set by tests, read by mocks
extern int mock_pin_read[20];
extern int mock_pin_written[20];
extern unsigned long mock_millis_value;
extern unsigned long mock_last_delay_ms;

#ifdef __cplusplus
extern "C" {
#endif
    int digitalRead(uint8_t pin);
    void digitalWrite(uint8_t pin, uint8_t value);
    void pinMode(uint8_t pin, uint8_t mode);
    unsigned long millis(void);
    void delay(unsigned long ms);
    int analogRead(uint8_t pin);
#ifdef __cplusplus
}
#endif

// Minimal mock for HardwareSerial
class HardwareSerial {
public:
    void begin(unsigned long) {}
    void print(const char*) {}
    void println(const char*) {}
    void print(int) {}
    void println(int) {}
    void print(unsigned int) {}
    void println(unsigned int) {}
    operator bool() { return true; }
};

extern HardwareSerial Serial;

// AVR pgmspace compatibility for native
#define PROGMEM
#define pgm_read_byte(addr) (*(const unsigned char *)(addr))
#define pgm_read_word(addr) (*(const unsigned short *)(addr))
#define pgm_read_dword(addr) (*(const uint32_t *)(addr))
#define strcpy_P(dest, src) strcpy((dest), (src))
#define strlen_P(src) strlen((src))
