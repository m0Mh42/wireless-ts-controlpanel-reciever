#include <Arduino.h>
#include <SPI.h>
#include <RF24.h>
#include <nRF24L01.h>
#include "header.h"
#include "safety_protocol.h"
#include "radio.h"
#include "io.h"
#include "lcd.h"
#include "buttons.h"

#define SAFETY_WATCHDOG_TIMEOUT_MS 100UL
#define CHANNEL_HOP_SEARCH_TIMEOUT_MS 500UL

// Global objects
RF24 radio(CE_PIN, CSN_PIN);
SafetyFrame rx_frame;
ts_status local_ts_status = TS_STAT_OFF;
uint8_t active_unit = 0;
uint16_t buttons = 0;
uint32_t last_valid_sequence = 0;
uint32_t rx_tx_sequence = 1;
unsigned long last_valid_packet_ms = 0;
unsigned long last_hop_attempt_ms = 0;
bool link_healthy = false;

void setup() {
#if DEBUG
    Serial.begin(9600);
    while (!Serial)
        ;
    Serial.println(F("[SYSTEM] UTECO Receiver Safety Node Initializing..."));
#endif

    // Initialize PLC IO subsystem
    io_init();

    // Initialize LCD
    lcd_init();

    // Initialize RF24 radio
    radio_setup(&radio);

#if DEBUG
    Serial.println(F("[SYSTEM] Receiver Node Ready. Listening for Black Channel frames."));
#endif
}

void loop() {
    unsigned long now = millis();

    // 1. Read Active Printing Unit from PLC inputs
    active_unit = io_read_active_unit();

    // 2. Read and Validate Incoming Black Channel Safety Frames
    if (radio_read_frame(&radio, &rx_frame, &last_valid_sequence)) {
        last_valid_packet_ms = now;
        link_healthy = true;

        buttons = rx_frame.payload.buttons_primary;
        local_ts_status = (ts_status)rx_frame.payload.status_flags;

        // Check if E-Stop button is pressed in payload
        if ((buttons & BUTTON_ESTOP_ACTIVE) || (rx_frame.frame_type == FrameType::ESTOP_ACTIVE)) {
            io_write_faststop(true);
        } else {
            io_write_faststop(false);
        }

        // Send telemetry / ACK frame back to transmitter with active_unit
        radio_send_telemetry(&radio, active_unit, local_ts_status, rx_tx_sequence++);
    }

    // 3. Safety Watchdog Timeout Check (100 ms)
    unsigned long elapsed = now - last_valid_packet_ms;
    if (elapsed > SAFETY_WATCHDOG_TIMEOUT_MS) {
        link_healthy = false;
        // User directive: Zero all button outputs when signal is lost, do not trip faststop
        buttons = 0;
        io_clear_all_buttons();

        // If communication lost for > 500 ms, search next Wi-Fi guard channel
        if (now - last_hop_attempt_ms > CHANNEL_HOP_SEARCH_TIMEOUT_MS) {
            radio_hop_next_channel(&radio);
            last_hop_attempt_ms = now;
        }
    } else {
        // Healthy link: multiplex button outputs to PLC
        io_write_buttons(buttons);
    }

    // 4. Update LCD telemetry screen
    lcd_update_status(local_ts_status, active_unit, buttons, link_healthy);

    delay(2);
}
