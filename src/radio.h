#ifndef _H_RADIO
#define _H_RADIO 1

#include <Arduino.h>
#include "header.h"
#include "protocol.h"

class RF24;

struct RadioRxDiagnostics
{
    uint32_t rx_raw_count;
    uint32_t rx_valid_count;
    uint32_t rx_crc_err_count;
    uint32_t rx_len_err_count;
    uint32_t rx_reject_count;
    uint8_t last_error_code;
    bool last_rx_ok;
    uint8_t datarate_mode;
    uint8_t datarate_1mbps;
};

void radio_setup(RF24 *_radio = NULL);
bool radio_receive_control(ControlPacket *out_packet, RF24 *_radio = NULL);
bool radio_send_telemetry(const TelemetryPacket &telemetry, RF24 *_radio = NULL);
bool radio_send_telemetry(uint8_t active_unit, ts_status status, uint8_t seq_ack, RF24 *_radio = NULL);

inline bool radio_stage_telemetry(uint8_t active_unit, ts_status status, uint8_t seq_ack, RF24 *_radio = NULL)
{
    return radio_send_telemetry(active_unit, status, seq_ack, _radio);
}

inline bool radio_reply_telemetry(uint8_t active_unit, ts_status status, uint32_t seq, RF24 *_radio = NULL)
{
    return radio_send_telemetry(active_unit, status, (uint8_t)(seq & 0xFF), _radio);
}

bool radio_set_channel(RF24 *_radio, uint8_t channel);
inline bool radio_set_channel(uint8_t channel) { return radio_set_channel(NULL, channel); }
uint8_t radio_get_channel();

void radio_hop_next_channel(RF24 *_radio = NULL);
void radio_recover_channel(RF24 *_radio = NULL);

uint8_t radio_get_lqi();
void radio_update_lqi(bool success);
void radio_lqi_tick(uint32_t now_ms);
bool radio_is_chip_connected(RF24 *_radio = NULL);

void radio_set_mode_rx(RF24 *_radio = NULL);
void radio_set_mode_tx(RF24 *_radio = NULL);
bool radio_is_rx_mode();

const RadioRxDiagnostics &radio_get_diagnostics();
void radio_reset_diagnostics();

#endif // _H_RADIO
