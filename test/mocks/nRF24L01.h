#pragma once
// Register map constants for nRF24L01+ — bare minimum for compilation.
// Not used directly by radio.cpp, but required by RF24.h includes.
#define RF24_CMD_R_REGISTER    0x00
#define RF24_CMD_W_REGISTER    0x20
#define RF24_CMD_R_RX_PAYLOAD  0x61
#define RF24_CMD_W_TX_PAYLOAD  0xA0
#define RF24_CMD_FLUSH_TX      0xE1
#define RF24_CMD_FLUSH_RX      0xE2
#define RF24_CMD_REUSE_TX_PL   0xE3
#define RF24_CMD_NOP           0xFF
