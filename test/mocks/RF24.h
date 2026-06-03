#pragma once
#include <stdint.h>
#include <cstring>
#include "nRF24L01.h"

// Matches real RF24 enum: RF24_PA_MIN=0, RF24_PA_LOW, RF24_PA_HIGH, RF24_PA_MAX
#define RF24_PA_MAX  3

// Mock state — set/check by tests
extern bool     mock_rf24_begin_called;
extern uint8_t  mock_rf24_pa_level;
extern uint8_t  mock_rf24_channel;
extern bool     mock_rf24_chip_connected;
extern bool     mock_rf24_write_result;
extern bool     mock_rf24_available;
extern uint8_t  mock_rf24_read_pipe_number;
extern const uint8_t* mock_rf24_write_addr;
extern const uint8_t* mock_rf24_read_addr;

#define MOCK_RF24_BUF_SIZE 32
extern uint8_t  mock_rf24_tx_buf[MOCK_RF24_BUF_SIZE];
extern uint8_t  mock_rf24_tx_len;
extern uint8_t  mock_rf24_rx_buf[MOCK_RF24_BUF_SIZE];
extern uint8_t  mock_rf24_rx_len;

class RF24 {
public:
    RF24(uint8_t ce_pin, uint8_t csn_pin) {}

    void begin() { mock_rf24_begin_called = true; }

    void openWritingPipe(const uint8_t* addr) {
        mock_rf24_write_addr = addr;
    }

    void openReadingPipe(uint8_t num, const uint8_t* addr) {
        mock_rf24_read_pipe_number = num;
        mock_rf24_read_addr = addr;
    }

    void setPALevel(uint8_t level) { mock_rf24_pa_level = level; }

    void setChannel(uint8_t ch) { mock_rf24_channel = ch; }

    bool isChipConnected() { return mock_rf24_chip_connected; }

    bool write(const void* buf, uint8_t len) {
        if (buf && len > 0 && len <= MOCK_RF24_BUF_SIZE) {
            memcpy(mock_rf24_tx_buf, buf, len);
            mock_rf24_tx_len = len;
        }
        return mock_rf24_write_result;
    }

    bool available() { return mock_rf24_available; }

    void read(void* buf, uint8_t len) {
        if (buf && len > 0 && len <= MOCK_RF24_BUF_SIZE) {
            memcpy(buf, mock_rf24_rx_buf, len);
            mock_rf24_rx_len = len;
        }
    }
};
