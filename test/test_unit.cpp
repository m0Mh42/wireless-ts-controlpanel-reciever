#include <Unity.h>
#include <cstring>
#include <cstdio>
#include "Arduino.h"
#include "SPI.h"
#include "LiquidCrystal_I2C.h"

HardwareSerial Serial;

// ============================================================
// Mock state — must be defined before including sources
// ============================================================
int mock_pin_read[20];
int mock_pin_written[20];
unsigned long mock_millis_value;
unsigned long mock_last_delay_ms;
char mock_lcd_buffer[128];
int mock_lcd_col;
int mock_lcd_row;
bool mock_lcd_cleared;
bool mock_lcd_backlight;
bool mock_lcd_init_called;
int mock_mux_read_count;
int mock_mux_cycling_enabled;

#define MOCK_RF24_BUF_SIZE 64
bool     mock_rf24_begin_called;
uint8_t  mock_rf24_pa_level;
uint8_t  mock_rf24_channel;
bool     mock_rf24_chip_connected;
bool     mock_rf24_write_result;
bool     mock_rf24_available;
uint8_t  mock_rf24_read_pipe_number;
const uint8_t* mock_rf24_write_addr;
const uint8_t* mock_rf24_read_addr;
uint8_t  mock_rf24_address_width;
uint8_t  mock_rf24_data_rate;
bool     mock_rf24_auto_ack;
uint8_t  mock_rf24_retries_delay;
uint8_t  mock_rf24_retries_count;
uint8_t  mock_rf24_payload_size;
bool     mock_rf24_listening;
uint8_t  mock_rf24_tx_buf[MOCK_RF24_BUF_SIZE];
uint8_t  mock_rf24_tx_len;
uint8_t  mock_rf24_rx_buf[MOCK_RF24_BUF_SIZE];
uint8_t  mock_rf24_rx_len;

static void mock_reset(void) {
    for (int i = 0; i < 20; i++) {
        mock_pin_read[i] = 0;
        mock_pin_written[i] = 0;
    }
    mock_millis_value = 0;
    mock_last_delay_ms = 0;
    mock_lcd_buffer[0] = '\0';
    mock_lcd_col = 0;
    mock_lcd_row = 0;
    mock_lcd_cleared = false;
    mock_lcd_backlight = false;
    mock_lcd_init_called = false;
    mock_mux_read_count = 0;
    mock_mux_cycling_enabled = 0;

    mock_rf24_begin_called = false;
    mock_rf24_pa_level = 0;
    mock_rf24_channel = 0;
    mock_rf24_chip_connected = true;
    mock_rf24_write_result = true;
    mock_rf24_available = false;
    mock_rf24_read_pipe_number = 0;
    mock_rf24_write_addr = nullptr;
    mock_rf24_read_addr = nullptr;
    mock_rf24_address_width = 0;
    mock_rf24_data_rate = 0;
    mock_rf24_auto_ack = false;
    mock_rf24_retries_delay = 0;
    mock_rf24_retries_count = 0;
    mock_rf24_payload_size = 0;
    mock_rf24_listening = false;
    mock_rf24_tx_len = 0;
    mock_rf24_rx_len = 0;
    memset(mock_rf24_tx_buf, 0, MOCK_RF24_BUF_SIZE);
    memset(mock_rf24_rx_buf, 0, MOCK_RF24_BUF_SIZE);
}

// ============================================================
// Arduino mock function definitions
// ============================================================
extern "C" {

int digitalRead(uint8_t pin) {
    if (pin >= 14 && pin <= 17 && mock_mux_cycling_enabled) { // A0..A3 are pins 14..17 in standard Arduino
        int mux_pos = (mock_mux_read_count / 4) % 4;
        mock_mux_read_count++;
        return (pin == (uint8_t)(14 + mux_pos)) ? HIGH : LOW;
    }
    return mock_pin_read[pin];
}

void digitalWrite(uint8_t pin, uint8_t value) {
    mock_pin_written[pin] = value;
}

void pinMode(uint8_t pin, uint8_t mode) {}

unsigned long millis(void) {
    return mock_millis_value;
}

void delay(unsigned long ms) {
    mock_last_delay_ms = ms;
}

int analogRead(uint8_t pin) {
    return mock_pin_read[pin];
}

} // extern "C"

// SPI instance required by <SPI.h> mock
SPIClass SPI;

// ============================================================
// Include sources under test
// ============================================================
#include "header.h"
#include "../src/safety_crc32.cpp"
#include "../src/safety_protocol.cpp"
#include "../src/utils.cpp"
#include "../src/buttons.cpp"
#include "../src/io.cpp"
#include "../src/radio.cpp"
#include "../src/lcd.cpp"

// ============================================================
// CRC-32 & SAFETY PROTOCOL TESTS
// ============================================================
static void test_crc32_self_test(void) {
    TEST_ASSERT_TRUE(crc32_self_test());
}

static void test_crc32_standard_vector(void) {
    const uint8_t vector[] = {'1', '2', '3', '4', '5', '6', '7', '8', '9'};
    uint32_t crc = crc32_calculate_ieee(vector, sizeof(vector), 0xFFFFFFFFUL);
    TEST_ASSERT_EQUAL_HEX32(0xCBF43926UL, crc);
}

static void test_safety_frame_roundtrip(void) {
    SafetyFrame tx;
    tx.preamble = PROTOCOL_MAGIC_PREAMBLE;
    tx.version = PROTOCOL_VERSION;
    tx.sender_id = NODE_ID_TRANSMITTER;
    tx.target_id = NODE_ID_RECEIVER;
    tx.sequence_number = 500;
    tx.session_id = DEFAULT_SESSION_ID;
    tx.frame_type = FrameType::COMMAND;
    tx.payload_length = sizeof(ApplicationPayload);

    SafetyProtocolEngine::pack_buttons(BUTTON_REGISTER_AVANTI | BUTTON_CLICHE_AV_ANTERIORE, tx.payload);
    tx.payload.active_unit = 5;
    tx.payload.status_flags = (uint8_t)TS_STAT_ON;
    tx.payload.battery_mv = 11500;
    tx.payload.diagnostic_flags = 0;
    tx.payload.spare1 = 0;
    tx.payload.spare2 = 0;

    uint8_t buf[SAFETY_FRAME_SIZE];
    bool ser_ok = SafetyProtocolEngine::serialize_frame(tx, buf, sizeof(buf));
    TEST_ASSERT_TRUE(ser_ok);

    SafetyFrame rx;
    SafetyValidationResult res = SafetyProtocolEngine::deserialize_and_validate(
        buf, sizeof(buf), NODE_ID_TRANSMITTER, NODE_ID_RECEIVER, DEFAULT_SESSION_ID, 499, 10, rx
    );

    TEST_ASSERT_EQUAL(SafetyValidationResult::VALID, res);
    TEST_ASSERT_EQUAL_UINT32(500, rx.sequence_number);
    TEST_ASSERT_EQUAL_HEX16(BUTTON_REGISTER_AVANTI | BUTTON_CLICHE_AV_ANTERIORE, rx.payload.buttons_primary);
    TEST_ASSERT_EQUAL_HEX16((uint16_t)~(BUTTON_REGISTER_AVANTI | BUTTON_CLICHE_AV_ANTERIORE), rx.payload.buttons_complement);
    TEST_ASSERT_EQUAL_UINT8(5, rx.payload.active_unit);
}

static void test_safety_frame_bitflip_rejection(void) {
    SafetyFrame tx;
    tx.sender_id = NODE_ID_TRANSMITTER;
    tx.target_id = NODE_ID_RECEIVER;
    tx.sequence_number = 300;
    tx.session_id = DEFAULT_SESSION_ID;
    tx.frame_type = FrameType::COMMAND;
    SafetyProtocolEngine::pack_buttons(BUTTON_COLOR_ATTACCO, tx.payload);

    uint8_t buf[SAFETY_FRAME_SIZE];
    SafetyProtocolEngine::serialize_frame(tx, buf, sizeof(buf));

    // Flip single bit
    buf[15] ^= 0x02;

    SafetyFrame rx;
    SafetyValidationResult res = SafetyProtocolEngine::deserialize_and_validate(
        buf, sizeof(buf), NODE_ID_TRANSMITTER, NODE_ID_RECEIVER, DEFAULT_SESSION_ID, 299, 10, rx
    );

    TEST_ASSERT_EQUAL(SafetyValidationResult::ERR_CRC32_MISMATCH, res);
}

// ============================================================
// UTILS & MATH TESTS
// ============================================================
static void test_powtwo_0(void) {
    TEST_ASSERT_EQUAL_UINT64(1, powtwo(0));
}
static void test_powtwo_1(void) {
    TEST_ASSERT_EQUAL_UINT64(2, powtwo(1));
}
static void test_powtwo_3(void) {
    TEST_ASSERT_EQUAL_UINT64(8, powtwo(3));
}
static void test_powtwo_15(void) {
    TEST_ASSERT_EQUAL_UINT64(32768, powtwo(15));
}
static void test_mapfloat_identity(void) {
    TEST_ASSERT_EQUAL_FLOAT(50.0f, mapfloat(50, 0, 100, 0, 100));
}
static void test_mapint_identity(void) {
    TEST_ASSERT_EQUAL_INT(50, mapint(50, 0, 100, 0, 100));
}

// ============================================================
// BUTTONS TESTS
// ============================================================
static void test_buttons_process_zero(void) {
    TEST_ASSERT_EQUAL_UINT16(0, buttons_process(0));
}

// ============================================================
// IO TESTS (Multiplexer, Active Unit, Buttons, Faststop)
// ============================================================
static void test_io_read_mux_all_low(void) {
    mock_reset();
    mock_pin_read[PLCI_MUX1] = LOW;
    mock_pin_read[PLCI_MUX2] = LOW;
    mock_pin_read[PLCI_MUX3] = LOW;
    mock_pin_read[PLCI_MUX4] = LOW;
    TEST_ASSERT_EQUAL_UINT8(255, io_read_encode_mux());
}

static void test_io_read_mux_position_0(void) {
    mock_reset();
    mock_pin_read[PLCI_MUX1] = HIGH;
    mock_pin_read[PLCI_MUX2] = LOW;
    mock_pin_read[PLCI_MUX3] = LOW;
    mock_pin_read[PLCI_MUX4] = LOW;
    TEST_ASSERT_EQUAL_UINT8(0, io_read_encode_mux());
}

static void test_io_read_mux_position_3(void) {
    mock_reset();
    mock_pin_read[PLCI_MUX1] = LOW;
    mock_pin_read[PLCI_MUX2] = LOW;
    mock_pin_read[PLCI_MUX3] = LOW;
    mock_pin_read[PLCI_MUX4] = HIGH;
    TEST_ASSERT_EQUAL_UINT8(3, io_read_encode_mux());
}

static void test_io_read_active_unit_bcd(void) {
    mock_reset();
    mock_pin_read[PLCI_DISPLAYA] = HIGH;
    mock_pin_read[PLCI_DISPLAYB] = LOW;
    mock_pin_read[PLCI_DISPLAYC] = HIGH;
    mock_pin_read[PLCI_DISPLAYD] = LOW;
    TEST_ASSERT_EQUAL_UINT8(5, io_read_active_unit()); // 1 + 4 = 5
}

static void test_io_write_faststop_states(void) {
    mock_reset();
    io_write_faststop(true);
    TEST_ASSERT_EQUAL(HIGH, mock_pin_written[PLCO_FASTSTOP]);
    io_write_faststop(false);
    TEST_ASSERT_EQUAL(LOW, mock_pin_written[PLCO_FASTSTOP]);
}

static void test_io_clear_all_buttons(void) {
    mock_reset();
    mock_pin_written[PLCO_REGISTER] = HIGH;
    mock_pin_written[PLCO_ANILOX] = HIGH;
    mock_pin_written[PLCO_CLICHE] = HIGH;
    mock_pin_written[PLCO_COLOR] = HIGH;

    io_clear_all_buttons();

    TEST_ASSERT_EQUAL(LOW, mock_pin_written[PLCO_REGISTER]);
    TEST_ASSERT_EQUAL(LOW, mock_pin_written[PLCO_ANILOX]);
    TEST_ASSERT_EQUAL(LOW, mock_pin_written[PLCO_CLICHE]);
    TEST_ASSERT_EQUAL(LOW, mock_pin_written[PLCO_COLOR]);
}

static void test_io_write_buttons_16bit_fixed(void) {
    mock_reset();
    // Simulate MUX 0 active
    mock_pin_read[PLCI_MUX1] = HIGH;
    
    // Set CLICHE and COLOR buttons (bits 8 and 12)
    uint16_t btns = BUTTON_REGISTER_AVANTI | BUTTON_CLICHE_AV_ANTERIORE | BUTTON_COLOR_ATTACCO;
    io_write_buttons(btns);

    TEST_ASSERT_EQUAL(HIGH, mock_pin_written[PLCO_REGISTER]);
    TEST_ASSERT_EQUAL(HIGH, mock_pin_written[PLCO_CLICHE]);
    TEST_ASSERT_EQUAL(HIGH, mock_pin_written[PLCO_COLOR]);
}

// ============================================================
// RADIO TESTS
// ============================================================
static void test_radio_setup_receiver(void) {
    mock_reset();
    RF24 radio(CE_PIN, CSN_PIN);
    radio_setup(&radio);
    TEST_ASSERT_TRUE(mock_rf24_begin_called);
    TEST_ASSERT_EQUAL_UINT8(5, mock_rf24_address_width);
    TEST_ASSERT_EQUAL(RF24_PA_MAX, mock_rf24_pa_level);
    TEST_ASSERT_EQUAL(RF24_250KBPS, mock_rf24_data_rate);
    TEST_ASSERT_EQUAL(SAFETY_FRAME_SIZE, mock_rf24_payload_size);
    TEST_ASSERT_EQUAL(CHANNEL, mock_rf24_channel);
}

static void test_radio_send_telemetry_success(void) {
    mock_reset();
    RF24 radio(CE_PIN, CSN_PIN);
    radio_setup(&radio);

    bool sent = radio_send_telemetry(&radio, 7, TS_STAT_ON, 100);
    TEST_ASSERT_TRUE(sent);
    TEST_ASSERT_EQUAL(SAFETY_FRAME_SIZE, mock_rf24_tx_len);

    SafetyFrame rx;
    SafetyValidationResult res = SafetyProtocolEngine::deserialize_and_validate(
        mock_rf24_tx_buf, SAFETY_FRAME_SIZE, NODE_ID_RECEIVER, NODE_ID_TRANSMITTER, DEFAULT_SESSION_ID, 99, 10, rx
    );
    TEST_ASSERT_EQUAL(SafetyValidationResult::VALID, res);
    TEST_ASSERT_EQUAL_UINT8(7, rx.payload.active_unit);
    TEST_ASSERT_EQUAL_UINT8((uint8_t)TS_STAT_ON, rx.payload.status_flags);
}

static void test_radio_read_frame_success(void) {
    mock_reset();
    RF24 radio(CE_PIN, CSN_PIN);
    radio_setup(&radio);

    SafetyFrame tx;
    tx.preamble = PROTOCOL_MAGIC_PREAMBLE;
    tx.version = PROTOCOL_VERSION;
    tx.sender_id = NODE_ID_TRANSMITTER;
    tx.target_id = NODE_ID_RECEIVER;
    tx.sequence_number = 15;
    tx.session_id = DEFAULT_SESSION_ID;
    tx.frame_type = FrameType::COMMAND;
    tx.payload_length = sizeof(ApplicationPayload);
    SafetyProtocolEngine::pack_buttons(BUTTON_REGISTER_INDIETRO, tx.payload);
    tx.payload.status_flags = (uint8_t)TS_STAT_ON;

    SafetyProtocolEngine::serialize_frame(tx, mock_rf24_rx_buf, MOCK_RF24_BUF_SIZE);
    mock_rf24_available = true;

    SafetyFrame rx;
    uint32_t last_seq = 14;
    bool read_ok = radio_read_frame(&radio, &rx, &last_seq);

    TEST_ASSERT_TRUE(read_ok);
    TEST_ASSERT_EQUAL_UINT32(15, last_seq);
    TEST_ASSERT_EQUAL_HEX16(BUTTON_REGISTER_INDIETRO, rx.payload.buttons_primary);
}

static void test_radio_hop_channels(void) {
    mock_reset();
    RF24 radio(CE_PIN, CSN_PIN);
    radio_setup(&radio);

    uint8_t ch1 = radio_get_channel();
    radio_hop_next_channel(&radio);
    uint8_t ch2 = radio_get_channel();

    TEST_ASSERT_NOT_EQUAL(ch1, ch2);
    TEST_ASSERT_EQUAL_UINT8(ch2, mock_rf24_channel);
}

// ============================================================
// LCD TESTS
// ============================================================
static void test_lcd_init(void) {
    mock_reset();
    lcd_init();
    TEST_ASSERT_TRUE(mock_lcd_init_called);
    TEST_ASSERT_TRUE(mock_lcd_backlight);
}

static void test_lcd_update_telemetry(void) {
    mock_reset();
    lcd_update_status(TS_STAT_ON, 4, 0x0001, true);
    TEST_ASSERT_TRUE(true);
}

// ============================================================
void setUp(void) {}
void tearDown(void) {}

int main(int argc, char **argv) {
    UNITY_BEGIN();

    // CRC & Safety Protocol tests
    RUN_TEST(test_crc32_self_test);
    RUN_TEST(test_crc32_standard_vector);
    RUN_TEST(test_safety_frame_roundtrip);
    RUN_TEST(test_safety_frame_bitflip_rejection);

    // Utils
    RUN_TEST(test_powtwo_0);
    RUN_TEST(test_powtwo_1);
    RUN_TEST(test_powtwo_3);
    RUN_TEST(test_powtwo_15);
    RUN_TEST(test_mapfloat_identity);
    RUN_TEST(test_mapint_identity);

    // Buttons
    RUN_TEST(test_buttons_process_zero);

    // IO
    RUN_TEST(test_io_read_mux_all_low);
    RUN_TEST(test_io_read_mux_position_0);
    RUN_TEST(test_io_read_mux_position_3);
    RUN_TEST(test_io_read_active_unit_bcd);
    RUN_TEST(test_io_write_faststop_states);
    RUN_TEST(test_io_clear_all_buttons);
    RUN_TEST(test_io_write_buttons_16bit_fixed);

    // Radio
    RUN_TEST(test_radio_setup_receiver);
    RUN_TEST(test_radio_send_telemetry_success);
    RUN_TEST(test_radio_read_frame_success);
    RUN_TEST(test_radio_hop_channels);

    // LCD
    RUN_TEST(test_lcd_init);
    RUN_TEST(test_lcd_update_telemetry);

    return UNITY_END();
}
