#include "radio.h"
#include "display.h"
#include <RF24.h>

static RF24 s_hardware_radio(CE_PIN, CSN_PIN);
static RF24 *s_active_radio = &s_hardware_radio;

static uint8_t current_channel = CHANNEL;
static uint8_t current_lqi = 100;
static uint16_t lqi_scaled = 10000;
static RadioRxDiagnostics diag = {0, 0, 0, 0, 0, 0, false, 1, 1};

void radio_setup(RF24 *_radio)
{
    if (_radio != NULL)
    {
        s_active_radio = _radio;
    }
    else
    {
        s_active_radio = &s_hardware_radio;
    }

    pinMode(CSN_PIN, OUTPUT);
    digitalWrite(CSN_PIN, HIGH);
    pinMode(CE_PIN, OUTPUT);
    digitalWrite(CE_PIN, LOW);

    delay(15); // Silicon Power-On Reset settling delay

    s_active_radio->begin();
    s_active_radio->setAddressWidth(5);
    s_active_radio->setPALevel(RF24_PA_LEVEL_DEFAULT);
    s_active_radio->setCRCLength(RF24_CRC_16);
    s_active_radio->setDataRate(RF24_DATARATE_DEFAULT);
    s_active_radio->setChannel(current_channel);

    // Hardware Auto-ACK and Dynamic Payloads for ACK payloads
    s_active_radio->enableDynamicPayloads();
    s_active_radio->enableAckPayload();

    // Open reading pipe 1 for control packets
    s_active_radio->openReadingPipe(1, RF24_PIPE_CONTROL);

    // Preload first ACK payload on pipe 1
    TelemetryPacket initial_tel = {0, 1, 0, 0};
    s_active_radio->writeAckPayload(1, &initial_tel, sizeof(TelemetryPacket));

    // Receiver permanently in RX mode
    s_active_radio->startListening();

    if (!s_active_radio->isChipConnected())
    {
        diag.last_error_code = 1;
        display_show_radio_error();
    }
}

void radio_lqi_tick(uint32_t now_ms)
{
    (void)now_ms;
}

void radio_update_lqi(bool success)
{
    if (success)
    {
        lqi_scaled = (uint16_t)(((uint32_t)lqi_scaled * 95 + 50000UL + 99) / 100);
        if (lqi_scaled > 10000)
        {
            lqi_scaled = 10000;
        }
    }
    else
    {
        lqi_scaled = (uint16_t)(((uint32_t)lqi_scaled * 95) / 100);
    }
    current_lqi = (uint8_t)((lqi_scaled + 50) / 100);
}

uint8_t radio_get_lqi()
{
    return current_lqi;
}

bool radio_send_telemetry(const TelemetryPacket &telemetry, RF24 *_radio)
{
    RF24 *r = (_radio != NULL) ? _radio : s_active_radio;
    // Stage telemetry payload into the ACK FIFO for pipe 1
    return r->writeAckPayload(1, &telemetry, sizeof(TelemetryPacket));
}

bool radio_send_telemetry(uint8_t active_unit, ts_status status, uint8_t seq_ack, RF24 *_radio)
{
    TelemetryPacket tel;
    tel.active_unit = active_unit;
    tel.status = (uint8_t)status;
    tel.seq_ack = seq_ack;
    tel.padding = 0;
    return radio_send_telemetry(tel, _radio);
}

bool radio_receive_control(ControlPacket *out_packet, RF24 *_radio)
{
    RF24 *r = (_radio != NULL) ? _radio : s_active_radio;
    if (!r->available())
    {
        return false;
    }

    diag.rx_raw_count++;
    ControlPacket pkt;
    r->read(&pkt, sizeof(ControlPacket));

    // Dual-rail bitwise complement safety check
    if (!protocol_verify_buttons(pkt.buttons, pkt.buttons_inv))
    {
        diag.rx_reject_count++;
        diag.last_rx_ok = false;
        diag.last_error_code = 4; // VALIDATION_ERR
        radio_update_lqi(false);
        return false;
    }

    diag.rx_valid_count++;
    diag.last_rx_ok = true;
    diag.last_error_code = 0;
    radio_update_lqi(true);

    if (out_packet != NULL)
    {
        *out_packet = pkt;
    }
    return true;
}

bool radio_set_channel(RF24 *_radio, uint8_t channel)
{
    RF24 *r = (_radio != NULL) ? _radio : s_active_radio;
    current_channel = channel;
    r->setChannel(current_channel);
    return true;
}

uint8_t radio_get_channel()
{
    return current_channel;
}

void radio_hop_next_channel(RF24 *_radio)
{
    uint8_t next = (current_channel == 78) ? 88 : 78;
    radio_set_channel(_radio, next);
}

void radio_recover_channel(RF24 *_radio)
{
    radio_set_channel(_radio, CHANNEL);
}

bool radio_is_chip_connected(RF24 *_radio)
{
    RF24 *r = (_radio != NULL) ? _radio : s_active_radio;
    return r->isChipConnected();
}

const RadioRxDiagnostics &radio_get_diagnostics()
{
    return diag;
}

void radio_reset_diagnostics()
{
    diag.rx_raw_count = 0;
    diag.rx_valid_count = 0;
    diag.rx_crc_err_count = 0;
    diag.rx_len_err_count = 0;
    diag.rx_reject_count = 0;
    diag.last_error_code = 0;
    diag.last_rx_ok = false;
    current_lqi = 100;
    lqi_scaled = 10000;
}

void radio_set_mode_rx(RF24 *_radio)
{
    RF24 *r = (_radio != NULL) ? _radio : s_active_radio;
    r->startListening();
}

void radio_set_mode_tx(RF24 *_radio)
{
    (void)_radio;
}

bool radio_is_rx_mode()
{
    return true;
}