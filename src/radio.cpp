#include "radio.h"

static uint8_t current_channel = CHANNEL;
static uint8_t current_hop_index = 3; // Start at 78
static uint8_t current_lqi = 100;

void radio_setup(RF24 *_radio)
{
    _radio->begin();
    _radio->setAddressWidth(5);
    _radio->openWritingPipe(RF24_PIPE_TX);
    _radio->openReadingPipe(1, RF24_PIPE_RX);
    _radio->setPALevel(RF24_PA_MAX);
    _radio->setDataRate(RF24_250KBPS);
    _radio->setChannel(current_channel);
    _radio->setAutoAck(true);
    _radio->setRetries(5, 15);
    _radio->setPayloadSize(SAFETY_FRAME_SIZE);
    _radio->startListening();

    // Defected Chip Error Check
    if (!_radio->isChipConnected())
    {
        while (true)
        {
            digitalWrite(LED_BUILTIN, HIGH);
            delay(250);
            digitalWrite(LED_BUILTIN, LOW);
            delay(250);
        }
    }
}

void radio_update_lqi(bool success)
{
    // Exponential Moving Average
    uint16_t sample = success ? 100 : 0;
    current_lqi = (uint8_t)((20 * sample + 80 * current_lqi) / 100);
}

uint8_t radio_get_lqi()
{
    return current_lqi;
}

bool radio_read_frame(RF24 *_radio, SafetyFrame *out_frame, uint32_t *last_valid_seq)
{
    if (_radio->available())
    {
        uint8_t rx_buf[SAFETY_FRAME_SIZE];
        _radio->read(rx_buf, SAFETY_FRAME_SIZE);

        SafetyValidationResult res = SafetyProtocolEngine::deserialize_and_validate(
            rx_buf,
            SAFETY_FRAME_SIZE,
            NODE_ID_TRANSMITTER,
            NODE_ID_RECEIVER,
            DEFAULT_SESSION_ID,
            *last_valid_seq,
            100,
            *out_frame
        );

        if (res == SafetyValidationResult::VALID)
        {
            *last_valid_seq = out_frame->sequence_number;
            radio_update_lqi(true);
            return true;
        }
        else
        {
            radio_update_lqi(false);
            return false;
        }
    }
    return false;
}

bool radio_send_telemetry(RF24 *_radio, uint8_t active_unit, ts_status status, uint32_t seq)
{
    SafetyFrame tx_frame;
    tx_frame.preamble = PROTOCOL_MAGIC_PREAMBLE;
    tx_frame.version = PROTOCOL_VERSION;
    tx_frame.sender_id = NODE_ID_RECEIVER;
    tx_frame.target_id = NODE_ID_TRANSMITTER;
    tx_frame.sequence_number = seq;
    tx_frame.session_id = DEFAULT_SESSION_ID;
    tx_frame.frame_type = FrameType::TELEMETRY;
    tx_frame.payload_length = sizeof(ApplicationPayload);

    SafetyProtocolEngine::pack_buttons(0, tx_frame.payload);
    tx_frame.payload.active_unit = active_unit;
    tx_frame.payload.status_flags = (uint8_t)status;
    tx_frame.payload.battery_mv = 0;
    tx_frame.payload.diagnostic_flags = ((uint16_t)current_channel) | (((uint16_t)current_lqi) << 8);
    tx_frame.payload.spare1 = 0;
    tx_frame.payload.spare2 = 0;

    uint8_t tx_buf[SAFETY_FRAME_SIZE];
    if (!SafetyProtocolEngine::serialize_frame(tx_frame, tx_buf, sizeof(tx_buf)))
    {
        return false;
    }

    _radio->stopListening();
    bool sent = _radio->write(tx_buf, SAFETY_FRAME_SIZE);
    _radio->startListening();

    radio_update_lqi(sent);
    return sent;
}

bool radio_set_channel(RF24 *_radio, uint8_t channel)
{
    bool allowed = false;
    for (uint8_t i = 0; i < RADIO_CHANNEL_COUNT; ++i)
    {
        if (RADIO_CHANNELS[i] == channel)
        {
            allowed = true;
            current_hop_index = i;
            break;
        }
    }
    if (!allowed) return false;

    current_channel = channel;
    _radio->stopListening();
    _radio->setChannel(channel);
    _radio->startListening();
    return true;
}

uint8_t radio_get_channel()
{
    return current_channel;
}

void radio_hop_next_channel(RF24 *_radio)
{
    current_hop_index = (current_hop_index + 1) % RADIO_CHANNEL_COUNT;
    current_channel = RADIO_CHANNELS[current_hop_index];
    _radio->stopListening();
    _radio->setChannel(current_channel);
    _radio->startListening();
}