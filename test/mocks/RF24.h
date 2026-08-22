#pragma once
#include <stdint.h>
#include <cstring>
#include <cstdio>
#include <Arduino.h>

#define RF24_PA_MIN  0
#define RF24_PA_LOW  1
#define RF24_PA_HIGH 2
#define RF24_PA_MAX  3

#define RF24_250KBPS 0
#define RF24_1MBPS   1
#define RF24_2MBPS   2

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
extern uint8_t  mock_rf24_address_width;
extern uint8_t  mock_rf24_data_rate;
extern bool     mock_rf24_auto_ack;
extern uint8_t  mock_rf24_retries_delay;
extern uint8_t  mock_rf24_retries_count;
extern uint8_t  mock_rf24_payload_size;
extern bool     mock_rf24_listening;

#define MOCK_RF24_BUF_SIZE 64
extern uint8_t  mock_rf24_tx_buf[MOCK_RF24_BUF_SIZE];
extern uint8_t  mock_rf24_tx_len;
extern uint8_t  mock_rf24_rx_buf[MOCK_RF24_BUF_SIZE];
extern uint8_t  mock_rf24_rx_len;

class RF24 {
public:
    RF24(uint8_t ce_pin, uint8_t csn_pin) {}

    void begin() { mock_rf24_begin_called = true; }

    void setAddressWidth(uint8_t width) {
        mock_rf24_address_width = width;
    }

    void openWritingPipe(const uint8_t* addr) {
        mock_rf24_write_addr = addr;
    }

    void openReadingPipe(uint8_t num, const uint8_t* addr) {
        mock_rf24_read_pipe_number = num;
        mock_rf24_read_addr = addr;
    }

    void setPALevel(uint8_t level) { mock_rf24_pa_level = level; }

    void setDataRate(uint8_t rate) { mock_rf24_data_rate = rate; }

    void setChannel(uint8_t ch) { mock_rf24_channel = ch; }

    void setAutoAck(bool enable) { mock_rf24_auto_ack = enable; }

    void setRetries(uint8_t delay, uint8_t count) {
        mock_rf24_retries_delay = delay;
        mock_rf24_retries_count = count;
    }

    void setPayloadSize(uint8_t size) {
        mock_rf24_payload_size = size;
    }

    void startListening() { mock_rf24_listening = true; }

    void stopListening() { mock_rf24_listening = false; }

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
