#pragma once
#include <stdint.h>
#include <cstring>
#include <Arduino.h>

class TM16xx {
public:
    TM16xx(uint8_t dataPin, uint8_t clockPin, uint8_t strobePin,
           uint8_t maxDisplays, uint8_t nDigitsUsed,
           bool activateDisplay = true, uint8_t intensity = 7) {}

    virtual void begin(bool activateDisplay = true, uint8_t intensity = 7) {}
    virtual void setupDisplay(bool active, uint8_t intensity) {}
    virtual void clearDisplay() {}

protected:
    virtual void sendData(uint8_t addr, uint8_t data) {}
    virtual void sendCommand(uint8_t cmd) {}
};
