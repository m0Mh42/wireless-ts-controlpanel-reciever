#ifndef _H_PROTOCOL
#define _H_PROTOCOL 1

#include <stdint.h>
#include <stdbool.h>

#pragma pack(push, 1)

/**
 * @brief Control Packet sent from Transmitter Pendant to PLC Receiver (8 Bytes)
 */
struct ControlPacket
{
    uint16_t buttons;     // Bitmask of 15 buttons (bits 0..14, bit 15 = ESTOP)
    uint16_t buttons_inv; // Bitwise complement (~buttons) for dual-rail safety
    uint16_t battery_mv;  // Battery voltage in millivolts (e.g. 11800)
    uint8_t status;       // 1 = ON, 2 = BATTERY_LOW, 0 = OFF
    uint8_t seq;          // Incrementing counter 0..255 for freshness
};

/**
 * @brief Telemetry Packet returned from PLC Receiver to Transmitter inside ACK Payload (4 Bytes)
 */
struct TelemetryPacket
{
    uint8_t active_unit; // Active printing unit (0..15 BCD from PLC)
    uint8_t status;      // Receiver status (e.g. 1 = ON)
    uint8_t seq_ack;     // Echo of last received sequence counter
    uint8_t padding;     // Alignment padding (0x00)
};

#pragma pack(pop)

/**
 * @brief Dual-rail validation check: ensures no stuck-at bit in RAM or registers
 */
static inline bool protocol_verify_buttons(uint16_t buttons, uint16_t buttons_inv)
{
    return (uint16_t)(buttons ^ buttons_inv) == 0xFFFFU;
}

#endif // _H_PROTOCOL
