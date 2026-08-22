#ifndef _H_HEADER
#define _H_HEADER 1

#include <Arduino.h>
#include "safety_protocol.h"

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
#define CSN_PIN 10
#define CE_PIN 9
#endif

#ifndef CHANNEL
#define CHANNEL 78
#endif

#ifndef DEBUG
#define DEBUG 0
#endif

#ifndef TEST
#define TEST 0
#endif

// Wi-Fi Guard Channel Hopping Table (Inter-Band low interference channels)
static const uint8_t RADIO_CHANNELS[] = {4, 28, 54, 78, 82};
static const uint8_t RADIO_CHANNEL_COUNT = sizeof(RADIO_CHANNELS) / sizeof(RADIO_CHANNELS[0]);

// Logical Node IDs for Black Channel safety
#define NODE_ID_RECEIVER    0x0001
#define NODE_ID_TRANSMITTER 0x0002
#define DEFAULT_SESSION_ID  0x1234

// RF24 5-Byte Pipe Addresses
static const uint8_t RF24_PIPE_RX[5] = {'S', 'F', '0', '0', '1'};
static const uint8_t RF24_PIPE_TX[5] = {'S', 'F', '0', '0', '2'};

// Button bitmasks for 15-button matrix (16-bit uint16_t)
typedef enum buttons_enum
{
    // Register (4 buttons)
    BUTTON_REGISTER_AVANTI     = 0x0001,
    BUTTON_REGISTER_INDIETRO   = 0x0002,
    BUTTON_REGISTER_ABTERIORE  = 0x0004,
    BUTTON_REGISTER_POSTERIORE = 0x0008,

    // Anilox (4 buttons)
    BUTTON_ANILOX_AV_ANTERIORE = 0x0010,
    BUTTON_ANILOX_IN_ANTERIORE = 0x0020,
    BUTTON_ANILOX_AV_POSTERIORE= 0x0040,
    BUTTON_ANILOX_IN_POSTERIORE= 0x0080,

    // Cliche (4 buttons)
    BUTTON_CLICHE_AV_ANTERIORE = 0x0100,
    BUTTON_CLICHE_IN_ANTERIORE = 0x0200,
    BUTTON_CLICHE_AV_POSTERIORE= 0x0400,
    BUTTON_CLICHE_IN_POSTERIORE= 0x0800,

    // Color (3 buttons)
    BUTTON_COLOR_ATTACCO       = 0x1000,
    BUTTON_COLOR_STACCO        = 0x2000,
    BUTTON_COLOR_COLORE        = 0x4000,

    // Emergency / Safety stop bit
    BUTTON_ESTOP_ACTIVE        = 0x8000
} buttons_enum;

typedef enum ts_status
{
    TS_STAT_OFF = 0,
    TS_STAT_ON = 1,
    TS_STAT_BATTERYLOW = 2,
    TS_STAT_FAULT = 3
} ts_status;

#endif // _H_HEADER
