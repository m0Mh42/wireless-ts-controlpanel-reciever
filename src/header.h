#ifndef _H_HEADER
#define _H_HEADER 1

#include <Arduino.h>

#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega328PB__) || defined(__AVR_ATmega2560__) || defined(__AVR_ATmega1280__)
#define CSN_PIN 10
#define CE_PIN 9
#elif defined(ESP32)
#define CSN_PIN 5
#define CE_PIN 4
#elif defined(ESP8266)
#define CSN_PIN D8
#define CE_PIN D4
#else
#error "Make Config yourself"
#endif

#define CHANNEL 42
#define DEBUG false
#define TEST true

// Battery Voltage Input
#define BATTERY_INPUT_PIN A0
#define VOLTAGE_DIVIDER_COEFFICIENT 0.3
#define BATTERY_FULL_VOLTAGE 12
#define BATTERY_EMPTY_VOLTAGE 10

// Button Input Pins
#define P1 1
#define P2 2
#define P3 3
#define P4 4
#define P5 5
#define P6 6
#define P7 7
#define P8 8

// Device Addresses Enumeration
enum device_addresses_enum
{
    DEVICE_MASTER = 0,
    DEVICE_TS_SLAVE = 1
};

// Device Addresses
const uint8_t addresses[][4] = {"RX0", "TS1"};

// Transaction Unit Command Byte
typedef enum commands_enum
{
    COMM_NOTHING = 0,
    COMM_START_TX = 1,
    COMM_BUTTON = 2,
    COMM_STOP_TX = 4,
    COMM_ACK = 8
} commands_enum;

typedef enum ts_status
{
    TS_STAT_OFF = 0,
    TS_STAT_ON = 1,
    TS_STAT_BATTERYLOW = 2
} ts_status;

// Transaction Unit
typedef struct transaction_unit
{
    uint64_t seq;
    uint8_t command;
    uint8_t buttons;
} transaction_unit;

float mapfloat(float x, long in_min, long in_max, long out_min, long out_max);
int mapint(float x, long in_min, long in_max, long out_min, long out_max);

#endif
