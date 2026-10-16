#include <Unity.h>
#include <cstring>
#include <cstdio>
#include "Arduino.h"
#include "SPI.h"
#include "TM1638.h"

HardwareSerial Serial;

// ============================================================
// Mock state — must be defined before including sources
// ============================================================
int mock_pin_read[32];
int mock_pin_written[32];
unsigned long mock_millis_value;
unsigned long mock_last_delay_ms;
uint32_t mock_tm1638_buttons;
char mock_tm1638_display_str[32];
bool mock_tm1638_cleared;
bool mock_tm1638_init_called;
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
bool     mock_rf24_ack_payload_enabled;
bool     mock_rf24_dynamic_payloads_enabled;
bool     mock_rf24_ack_payload_available;
uint8_t  mock_rf24_tx_buf[MOCK_RF24_BUF_SIZE];
uint8_t  mock_rf24_tx_len;
uint8_t  mock_rf24_rx_buf[MOCK_RF24_BUF_SIZE];
uint8_t  mock_rf24_rx_len;
uint8_t  mock_rf24_ack_payload_buf[MOCK_RF24_BUF_SIZE];
uint8_t  mock_rf24_ack_payload_len;

static void mock_reset(void) {
    for (int i = 0; i < 32; i++) {
        mock_pin_read[i] = 0;
        mock_pin_written[i] = 0;
    }
    mock_millis_value = 0;
    mock_last_delay_ms = 0;
    mock_tm1638_buttons = 0;
    mock_tm1638_display_str[0] = '\0';
    mock_tm1638_cleared = false;
    mock_tm1638_init_called = false;
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
    mock_rf24_ack_payload_enabled = false;
    mock_rf24_dynamic_payloads_enabled = false;
    mock_rf24_ack_payload_available = false;
    mock_rf24_tx_len = 0;
    mock_rf24_rx_len = 0;
    mock_rf24_ack_payload_len = 0;
    memset(mock_rf24_tx_buf, 0, MOCK_RF24_BUF_SIZE);
    memset(mock_rf24_rx_buf, 0, MOCK_RF24_BUF_SIZE);
    memset(mock_rf24_ack_payload_buf, 0, MOCK_RF24_BUF_SIZE);
}

// ============================================================
// Arduino mock function definitions
// ============================================================
extern "C" {

int digitalRead(uint8_t pin) {
    if (pin >= 6 && pin <= 9 && mock_mux_cycling_enabled) {
        int mux_pos = (mock_mux_read_count / 4) % 4;
        mock_mux_read_count++;
        return (pin - 6 == mux_pos) ? HIGH : LOW;
    }
    return mock_pin_read[pin];
}

void digitalWrite(uint8_t pin, uint8_t val) {
    mock_pin_written[pin] = val;
}

void pinMode(uint8_t pin, uint8_t mode) {}

unsigned long millis(void) {
    return mock_millis_value;
}

unsigned long micros(void) {
    return mock_millis_value * 1000UL;
}

void delay(unsigned long ms) {
    mock_last_delay_ms = ms;
}

void delayMicroseconds(unsigned int us) {
    (void)us;
}

int analogRead(uint8_t pin) {
    return mock_pin_read[pin];
}

} // extern "C"

SPIClass SPI;

// ============================================================
// Include sources under test
// ============================================================
#include "protocol.h"
#include "header.h"
#include "../src/utils.cpp"
#include "../src/io.cpp"
#include "../src/radio.cpp"
#include "../src/display.cpp"

// ============================================================
// PROTOCOL TESTS
// ============================================================
static void test_protocol_packet_sizes(void) {
    TEST_ASSERT_EQUAL_UINT32(8, sizeof(ControlPacket));
    TEST_ASSERT_EQUAL_UINT32(4, sizeof(TelemetryPacket));
}

static void test_protocol_verify_buttons_valid(void) {
    uint16_t b = BUTTON_REGISTER_AVANTI | BUTTON_CLICHE_AV_ANTERIORE;
    uint16_t b_inv = (uint16_t)~b;
    TEST_ASSERT_TRUE(protocol_verify_buttons(b, b_inv));
}

static void test_protocol_verify_buttons_corrupted(void) {
    uint16_t b = BUTTON_REGISTER_AVANTI;
    uint16_t b_inv = (uint16_t)b;
    TEST_ASSERT_FALSE(protocol_verify_buttons(b, b_inv));
}

static void test_protocol_control_packet_construction(void) {
    ControlPacket pkt;
    pkt.buttons = BUTTON_COLOR_ATTACCO;
    pkt.buttons_inv = (uint16_t)~BUTTON_COLOR_ATTACCO;
    pkt.battery_mv = 11500;
    pkt.status = (uint8_t)TS_STAT_ON;
    pkt.seq = 50;

    TEST_ASSERT_EQUAL_HEX16(BUTTON_COLOR_ATTACCO, pkt.buttons);
    TEST_ASSERT_TRUE(protocol_verify_buttons(pkt.buttons, pkt.buttons_inv));
    TEST_ASSERT_EQUAL_UINT16(11500, pkt.battery_mv);
    TEST_ASSERT_EQUAL_UINT8(TS_STAT_ON, pkt.status);
    TEST_ASSERT_EQUAL_UINT8(50, pkt.seq);
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
    TEST_ASSERT_EQUAL_UINT8(5, io_read_active_unit());

    mock_pin_read[PLCI_DISPLAYB] = HIGH;
    mock_pin_read[PLCI_DISPLAYD] = HIGH;
    TEST_ASSERT_EQUAL_UINT8(15, io_read_active_unit());
}

static void test_io_write_faststop_states(void) {
    mock_reset();
    io_write_faststop(true);
    TEST_ASSERT_EQUAL_INT(HIGH, mock_pin_written[PLCO_FASTSTOP]);
    io_write_faststop(false);
    TEST_ASSERT_EQUAL_INT(LOW, mock_pin_written[PLCO_FASTSTOP]);
}

static void test_io_write_enable_states(void) {
    mock_reset();
    io_write_enable(true);
    TEST_ASSERT_EQUAL_INT(HIGH, mock_pin_written[PLCO_ENABLE]);
    io_write_enable(false);
    TEST_ASSERT_EQUAL_INT(LOW, mock_pin_written[PLCO_ENABLE]);
}

static void test_io_clear_all_buttons(void) {
    mock_reset();
    mock_pin_written[PLCO_REGISTER] = HIGH;
    mock_pin_written[PLCO_ANILOX] = HIGH;
    mock_pin_written[PLCO_CLICHE] = HIGH;
    mock_pin_written[PLCO_COLOR] = HIGH;
    io_clear_all_buttons();
    TEST_ASSERT_EQUAL_INT(LOW, mock_pin_written[PLCO_REGISTER]);
    TEST_ASSERT_EQUAL_INT(LOW, mock_pin_written[PLCO_ANILOX]);
    TEST_ASSERT_EQUAL_INT(LOW, mock_pin_written[PLCO_CLICHE]);
    TEST_ASSERT_EQUAL_INT(LOW, mock_pin_written[PLCO_COLOR]);
}

static void test_io_write_buttons_16bit_fixed(void) {
    mock_reset();
    uint16_t b = BUTTON_REGISTER_AVANTI;
    mock_pin_read[PLCI_MUX1] = HIGH;
    io_write_buttons(b);
    TEST_ASSERT_EQUAL_INT(HIGH, mock_pin_written[PLCO_REGISTER]);
    TEST_ASSERT_EQUAL_INT(LOW, mock_pin_written[PLCO_ANILOX]);
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
    TEST_ASSERT_EQUAL(RF24_PA_LEVEL_DEFAULT, mock_rf24_pa_level);
    TEST_ASSERT_EQUAL(RF24_DATARATE_DEFAULT, mock_rf24_data_rate);
    TEST_ASSERT_EQUAL(CHANNEL, mock_rf24_channel);
    TEST_ASSERT_TRUE(mock_rf24_ack_payload_enabled);
    TEST_ASSERT_TRUE(mock_rf24_dynamic_payloads_enabled);
    TEST_ASSERT_TRUE(mock_rf24_listening);
    TEST_ASSERT_EQUAL_UINT8(1, mock_rf24_read_pipe_number);
}

static void test_radio_send_telemetry_success(void) {
    mock_reset();
    RF24 radio(CE_PIN, CSN_PIN);
    radio_setup(&radio);

    bool sent = radio_send_telemetry(7, TS_STAT_ON, 100, &radio);
    TEST_ASSERT_TRUE(sent);
    TEST_ASSERT_EQUAL(sizeof(TelemetryPacket), mock_rf24_tx_len);

    TelemetryPacket* tel = (TelemetryPacket*)mock_rf24_tx_buf;
    TEST_ASSERT_EQUAL_UINT8(7, tel->active_unit);
    TEST_ASSERT_EQUAL_UINT8(TS_STAT_ON, tel->status);
    TEST_ASSERT_EQUAL_UINT8(100, tel->seq_ack);
    TEST_ASSERT_TRUE(mock_rf24_listening);
    TEST_ASSERT_EQUAL_UINT8(1, mock_rf24_read_pipe_number);
}

static void test_radio_read_frame_success(void) {
    mock_reset();
    RF24 radio(CE_PIN, CSN_PIN);
    radio_setup(&radio);

    ControlPacket pkt;
    pkt.buttons = BUTTON_REGISTER_INDIETRO;
    pkt.buttons_inv = (uint16_t)~BUTTON_REGISTER_INDIETRO;
    pkt.battery_mv = 12000;
    pkt.status = (uint8_t)TS_STAT_ON;
    pkt.seq = 15;

    memcpy(mock_rf24_rx_buf, &pkt, sizeof(ControlPacket));
    mock_rf24_rx_len = sizeof(ControlPacket);
    mock_rf24_available = true;

    ControlPacket rx;
    bool read_ok = radio_receive_control(&rx, &radio);

    TEST_ASSERT_TRUE(read_ok);
    TEST_ASSERT_EQUAL_HEX16(BUTTON_REGISTER_INDIETRO, rx.buttons);
    TEST_ASSERT_EQUAL_UINT8(15, rx.seq);
}

static void test_radio_hop_channels(void) {
    mock_reset();
    RF24 radio(CE_PIN, CSN_PIN);
    radio_setup(&radio);

    uint8_t ch1 = radio_get_channel();
    radio_hop_next_channel(&radio);
    uint8_t ch2 = radio_get_channel();

    TEST_ASSERT_NOT_EQUAL(ch1, ch2);
}

static void test_radio_is_chip_connected(void) {
    mock_reset();
    RF24 radio(CE_PIN, CSN_PIN);
    mock_rf24_chip_connected = false;
    TEST_ASSERT_FALSE(radio_is_chip_connected(&radio));
    mock_rf24_chip_connected = true;
    TEST_ASSERT_TRUE(radio_is_chip_connected(&radio));
}

static void test_radio_rx_diagnostics_tracking(void) {
    mock_reset();
    RF24 radio(CE_PIN, CSN_PIN);
    radio_setup(&radio);
    radio_reset_diagnostics();

    const RadioRxDiagnostics& d0 = radio_get_diagnostics();
    TEST_ASSERT_EQUAL_UINT32(0, d0.rx_raw_count);
    TEST_ASSERT_EQUAL_UINT32(0, d0.rx_valid_count);

    // 1. Simulate invalid payload (bad dual-rail complement)
    ControlPacket bad_pkt = {BUTTON_REGISTER_AVANTI, BUTTON_REGISTER_AVANTI, 12000, 1, 1};
    memcpy(mock_rf24_rx_buf, &bad_pkt, sizeof(ControlPacket));
    mock_rf24_rx_len = sizeof(ControlPacket);
    mock_rf24_available = true;

    ControlPacket rx;
    bool ok = radio_receive_control(&rx, &radio);
    TEST_ASSERT_FALSE(ok);
    const RadioRxDiagnostics& d1 = radio_get_diagnostics();
    TEST_ASSERT_EQUAL_UINT32(1, d1.rx_raw_count);
    TEST_ASSERT_EQUAL_UINT32(1, d1.rx_reject_count);
    TEST_ASSERT_EQUAL_UINT8(4, d1.last_error_code);

    // 2. Simulate valid packet
    mock_reset();
    radio_setup(&radio);
    radio_reset_diagnostics();

    ControlPacket valid_pkt = {BUTTON_REGISTER_AVANTI, (uint16_t)~BUTTON_REGISTER_AVANTI, 12000, 1, 2};
    memcpy(mock_rf24_rx_buf, &valid_pkt, sizeof(ControlPacket));
    mock_rf24_rx_len = sizeof(ControlPacket);
    mock_rf24_available = true;

    ok = radio_receive_control(&rx, &radio);
    TEST_ASSERT_TRUE(ok);
    const RadioRxDiagnostics& d2 = radio_get_diagnostics();
    TEST_ASSERT_EQUAL_UINT32(1, d2.rx_valid_count);
    TEST_ASSERT_TRUE(d2.last_rx_ok);
    TEST_ASSERT_EQUAL_UINT8(0, d2.last_error_code);
}

static void test_radio_lqi_scaled_convergence(void) {
    mock_reset();
    radio_reset_diagnostics();
    TEST_ASSERT_EQUAL_UINT8(100, radio_get_lqi());

    radio_update_lqi(false);
    TEST_ASSERT_EQUAL_UINT8(95, radio_get_lqi());

    for (int i = 0; i < 10; i++) {
        radio_update_lqi(false);
    }
    TEST_ASSERT_TRUE(radio_get_lqi() < 60);

    for (int i = 0; i < 100; i++) {
        radio_update_lqi(true);
    }
    TEST_ASSERT_EQUAL_UINT8(100, radio_get_lqi());
}

// ============================================================
// TM1638 DISPLAY TESTS
// ============================================================
static void test_display_init(void) {
    mock_reset();
    display_init();
    const uint8_t* ram = display_get_ram();
    TEST_ASSERT_BITS_HIGH(0x10, ram[6]); // Red indicator on GRID4
    TEST_ASSERT_BITS_LOW(0x10, ram[8]);  // Green indicator on GRID5 OFF
}

static void test_display_update_telemetry(void) {
    mock_reset();
    display_update(TS_STAT_ON, 4, 0x0001, true, 78, 95);
    const uint8_t* ram = display_get_ram();
    TEST_ASSERT_BITS_HIGH(0x10, ram[8]); // Green ON
    TEST_ASSERT_BITS_LOW(0x10, ram[6]);  // Red OFF
    TEST_ASSERT_BITS_HIGH(0x10, ram[14]);
    TEST_ASSERT_BITS_HIGH(0x20, ram[12]);
}

static void test_display_update_link_down(void) {
    mock_reset();
    display_update(TS_STAT_OFF, 0, 0, false, 78, 0);
    const uint8_t* ram = display_get_ram();
    TEST_ASSERT_BITS_HIGH(0x10, ram[6]);
    TEST_ASSERT_BITS_LOW(0x10, ram[8]);
    TEST_ASSERT_EQUAL_UINT8(0, ram[14] & 0x30);
    TEST_ASSERT_EQUAL_UINT8(0, ram[12] & 0x20);
}

static void test_display_show_radio_error(void) {
    mock_reset();
    display_show_radio_error();
    const uint8_t* ram = display_get_ram();
    TEST_ASSERT_BITS_HIGH(0x10, ram[6]);
    TEST_ASSERT_BITS_LOW(0x10, ram[8]);
    TEST_ASSERT_BITS_HIGH(0x40, ram[8]);
    TEST_ASSERT_BITS_HIGH(0x40, ram[0]);
    TEST_ASSERT_BITS_HIGH(0x40, ram[4]);
}

static void test_display_update_radio_error(void) {
    mock_reset();
    display_update(TS_STAT_ON, 2, 0, false, 78, 0, true);
    const uint8_t* ram = display_get_ram();
    TEST_ASSERT_BITS_HIGH(0x10, ram[6]);
    TEST_ASSERT_BITS_LOW(0x10, ram[8]);
    TEST_ASSERT_BITS_HIGH(0x40, ram[8]);
}

static void test_display_estop_active(void) {
    mock_reset();
    display_update(TS_STAT_ON, 1, BUTTON_ESTOP_ACTIVE, true, 78, 90);
    const uint8_t* ram = display_get_ram();
    TEST_ASSERT_BITS_HIGH(0x10, ram[6]);
    TEST_ASSERT_BITS_LOW(0x10, ram[8]);
    TEST_ASSERT_BITS_HIGH(0x40, ram[8]);
}

static void test_receiver_display_show_packet_debug(void) {
    mock_reset();
    RadioRxDiagnostics d;
    memset(&d, 0, sizeof(d));
    d.rx_raw_count = 10;
    d.rx_valid_count = 8;
    d.rx_crc_err_count = 2;
    d.last_rx_ok = true;
    d.datarate_1mbps = 1;

    display_show_packet_debug(d, 78, 80, true);
    const uint8_t* ram = display_get_ram();
    TEST_ASSERT_BITS_HIGH(0x10, ram[8]); // Green indicator
    TEST_ASSERT_BITS_HIGH(0x10, ram[2]); // Yel1 indicator
}

// ============================================================
void setUp(void) {}
void tearDown(void) {}

int main(int argc, char **argv) {
    UNITY_BEGIN();

    // Protocol tests
    RUN_TEST(test_protocol_packet_sizes);
    RUN_TEST(test_protocol_verify_buttons_valid);
    RUN_TEST(test_protocol_verify_buttons_corrupted);
    RUN_TEST(test_protocol_control_packet_construction);

    // Utils
    RUN_TEST(test_powtwo_0);
    RUN_TEST(test_powtwo_1);
    RUN_TEST(test_powtwo_3);
    RUN_TEST(test_powtwo_15);
    RUN_TEST(test_mapfloat_identity);
    RUN_TEST(test_mapint_identity);

    // IO
    RUN_TEST(test_io_read_mux_all_low);
    RUN_TEST(test_io_read_mux_position_0);
    RUN_TEST(test_io_read_mux_position_3);
    RUN_TEST(test_io_read_active_unit_bcd);
    RUN_TEST(test_io_write_faststop_states);
    RUN_TEST(test_io_write_enable_states);
    RUN_TEST(test_io_clear_all_buttons);
    RUN_TEST(test_io_write_buttons_16bit_fixed);

    // Radio
    RUN_TEST(test_radio_setup_receiver);
    RUN_TEST(test_radio_send_telemetry_success);
    RUN_TEST(test_radio_read_frame_success);
    RUN_TEST(test_radio_hop_channels);
    RUN_TEST(test_radio_is_chip_connected);
    RUN_TEST(test_radio_rx_diagnostics_tracking);
    RUN_TEST(test_radio_lqi_scaled_convergence);

    // TM1638 Display
    RUN_TEST(test_display_init);
    RUN_TEST(test_display_update_telemetry);
    RUN_TEST(test_display_update_link_down);
    RUN_TEST(test_display_show_radio_error);
    RUN_TEST(test_display_update_radio_error);
    RUN_TEST(test_display_estop_active);
    RUN_TEST(test_receiver_display_show_packet_debug);

    return UNITY_END();
}
