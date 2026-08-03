#ifndef _H_HEADER
#define _H_HEADER 1

#include <Arduino.h>

/*

  Module // Arduino UNO //         ESP32         //       ESP8266
    GND    ->   GND            ->  GND           ->       GND
    Vcc    ->   3.3V(External) ->  3.3V          ->       3.3V
    CE     ->   D9             ->  D4            ->       D4
    CSN    ->   D10            ->  D5            ->       D8
    CLK    ->   D13            ->  D18           ->       D5
    MOSI   ->   D11            ->  D23           ->       D7
    MISO   ->   D12            ->  D19           ->       D6

*/

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
    COMM_DATA = 8,
    COMM_ACK = 16
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
    uint8_t command;
    uint8_t buttons;
    uint8_t active_unit;
} transaction_unit;

#endif
