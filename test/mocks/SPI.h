#pragma once
#include <stdint.h>

class SPIClass {
public:
    void begin() {}
    void beginTransaction(uint32_t) {}
    void endTransaction() {}
    uint8_t transfer(uint8_t d) { return d; }
};

extern SPIClass SPI;
