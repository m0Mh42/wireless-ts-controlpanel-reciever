#include <Arduino.h>
#include <SPI.h>
#include "header.h"
#include "protocol.h"
#include "radio.h"
#include "io.h"
#include "display.h"

#define RADIO_DISCONNECT_DEBOUNCE_COUNT 20 // 20 cycles @ ~2ms = ~40 ms persistence

// Global objects
ControlPacket rx_pkt;
ts_status local_ts_status = TS_STAT_OFF;
uint8_t active_unit = 0;
uint16_t buttons = 0;
uint8_t last_valid_sequence = 0;
unsigned long last_valid_packet_ms = 0;
bool link_healthy = false;

void setup()
{
    // 0. Immediately park nRF24 SPI bus lines to prevent floating-pin bus corruption
    pinMode(CSN_PIN, OUTPUT);
    digitalWrite(CSN_PIN, HIGH);
    pinMode(CE_PIN, OUTPUT);
    digitalWrite(CE_PIN, LOW);

    // Initialize PLC IO subsystem
    io_init();
    delay(10); // Allow PLC optocoupler pullups to settle

    // Initialize TM1638 LED display
    display_init();

    // Initialize RF24 radio
    radio_setup();

    // Read active unit from PLC inputs
    active_unit = io_read_active_unit();

    last_valid_packet_ms = millis();
}

void loop()
{
    unsigned long now = millis();

    // 1. Hardware Connection Health Check of nRF24 Transceiver (with debouncing)
    static uint8_t radio_disconnect_count = 0;
    static bool radio_hardware_error = false;

    if (!radio_is_chip_connected())
    {
        if (radio_disconnect_count < RADIO_DISCONNECT_DEBOUNCE_COUNT)
        {
            radio_disconnect_count++;
        }
        else
        {
            radio_hardware_error = true;
        }
    }
    else
    {
        radio_disconnect_count = 0;
        radio_hardware_error = false;
    }

    if (radio_hardware_error)
    {
        link_healthy = false;
        buttons = 0;
        io_clear_all_buttons();
        io_write_enable(false);
        io_write_faststop(false);

        static unsigned long last_err_display_ms = 0;
        if (now - last_err_display_ms >= 50UL)
        {
            last_err_display_ms = now;
            display_update(local_ts_status, active_unit, 0, false, radio_get_channel(), 0, true);
        }

        // Auto-recovery: Re-initialize nRF24 every 500ms when module is reconnected
        static unsigned long last_reinit_ms = 0;
        if (now - last_reinit_ms >= 500UL)
        {
            last_reinit_ms = now;
            radio_setup();
            if (radio_is_chip_connected())
            {
                radio_hardware_error = false;
                radio_disconnect_count = 0;
            }
        }

        delay(10);
        return;
    }

    // 2. Read Active Printing Unit from PLC inputs
    uint8_t current_active_unit = io_read_active_unit();
    if (current_active_unit != active_unit)
    {
        active_unit = current_active_unit;
    }

    // 3. Read and Validate Incoming Control Packets
    if (radio_receive_control(&rx_pkt))
    {
        last_valid_packet_ms = now;
        link_healthy = true;
        last_valid_sequence = rx_pkt.seq;

        buttons = rx_pkt.buttons;
        local_ts_status = (ts_status)rx_pkt.status;

        // Check if E-Stop button is pressed in packet
        if (buttons & BUTTON_ESTOP_ACTIVE)
        {
            io_write_faststop(true);
        }
        else
        {
            io_write_faststop(false);
        }

        // Send immediate software telemetry reply with latest active unit and echoed sequence
        radio_send_telemetry(active_unit, local_ts_status, rx_pkt.seq);
    }

    // 4. Safety Watchdog Timeout Check (150 ms)
    unsigned long elapsed = now - last_valid_packet_ms;
    if (elapsed > SAFETY_WATCHDOG_TIMEOUT_MS)
    {
        link_healthy = false;
        // Failsafe: Clear all button outputs immediately upon timeout
        buttons = 0;
        io_clear_all_buttons();
        io_write_enable(false);
    }
    else
    {
        // Healthy link: enable PLC outputs and multiplex button outputs
        io_write_enable(local_ts_status == TS_STAT_ON);
        io_write_buttons(buttons);
    }

    // 5. Update TM1638 LED display telemetry (rate-limited to 40 ms / 25 Hz)
    static unsigned long last_display_ms = 0;
    if (now - last_display_ms >= 40UL)
    {
        last_display_ms = now;
#if defined(TEST) && TEST
        display_show_packet_debug(radio_get_diagnostics(), radio_get_channel(), radio_get_lqi(), !radio_hardware_error);
#else
        display_update(local_ts_status, active_unit, buttons, link_healthy, radio_get_channel(), radio_get_lqi());
#endif
    }
}
