#pragma once
#include <stdint.h>
#include <stddef.h>
#include <string.h>

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
