#ifndef _H_RADIO
#define _H_RADIO 1

#include <Arduino.h>
#include <SPI.h>
#include <RF24.h>
#include <nRF24L01.h>
#include "header.h"
#include "safety_protocol.h"

void radio_setup(RF24 *_radio);
bool radio_read_frame(RF24 *_radio, SafetyFrame *out_frame, uint32_t *last_valid_seq);
bool radio_send_telemetry(RF24 *_radio, uint8_t active_unit, ts_status status, uint32_t seq);
bool radio_set_channel(RF24 *_radio, uint8_t channel);
uint8_t radio_get_channel();
void radio_hop_next_channel(RF24 *_radio);
uint8_t radio_get_lqi();
void radio_update_lqi(bool success);

#endif // _H_RADIO
