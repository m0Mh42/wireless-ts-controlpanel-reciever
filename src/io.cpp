#include "io.h"

// ATmega328P pins A6 and A7 are dedicated analog inputs (ADC6/ADC7) without digital I/O buffers.
// digitalRead() on AVR always returns LOW for A6 and A7.
// We read their digital state via analogRead() with a 2.5V threshold (512 / 1023).
#define ANALOG_PIN_HIGH_THRESHOLD 512

static inline bool io_read_mux_pin(uint8_t pin)
{
    if (pin == A6 || pin == A7)
    {
        return analogRead(pin) >= ANALOG_PIN_HIGH_THRESHOLD;
    }
    return digitalRead(pin) == HIGH;
}

void io_init()
{
    // Configure PLC multiplexer input pins
    pinMode(PLCI_MUX1, INPUT);
    pinMode(PLCI_MUX2, INPUT);
    // Note: PLCI_MUX3 (A7) and PLCI_MUX4 (A6) are dedicated analog inputs on ATmega328P (no digital I/O buffers).
    // pinMode() is a no-op for A6/A7 in Arduino AVR core, but calling it is harmless.
    pinMode(PLCI_MUX3, INPUT);
    pinMode(PLCI_MUX4, INPUT);

    // Configure PLC active unit BCD display inputs (pullup ensures stable HIGH=15 on bench when open)
    pinMode(PLCI_DISPLAYA, INPUT_PULLUP);
    pinMode(PLCI_DISPLAYB, INPUT_PULLUP);
    pinMode(PLCI_DISPLAYC, INPUT_PULLUP);
    pinMode(PLCI_DISPLAYD, INPUT_PULLUP);

    // Configure PLC button outputs
    pinMode(PLCO_REGISTER, OUTPUT);
    pinMode(PLCO_ANILOX, OUTPUT);
    pinMode(PLCO_CLICHE, OUTPUT);
    pinMode(PLCO_COLOR, OUTPUT);

    // Configure FASTSTOP safety output
    pinMode(PLCO_FASTSTOP, OUTPUT);

    // Configure PLC ENABLE safety output
    pinMode(PLCO_ENABLE, OUTPUT);

    io_clear_all_buttons();
    io_write_faststop(false); // Normal running state on boot
    io_write_enable(false);   // Disabled on boot until link established
}

uint8_t io_read_encode_mux()
{
    if (io_read_mux_pin(PLCI_MUX1))
        return 0;
    if (io_read_mux_pin(PLCI_MUX2))
        return 1;
    if (io_read_mux_pin(PLCI_MUX3))
        return 2;
    if (io_read_mux_pin(PLCI_MUX4))
        return 3;
    return 255; // No valid MUX active
}

uint8_t io_read_active_unit()
{
    uint8_t u1 = 0;
    u1 += (digitalRead(PLCI_DISPLAYA) == HIGH ? 1 : 0);
    u1 += (digitalRead(PLCI_DISPLAYB) == HIGH ? 2 : 0);
    u1 += (digitalRead(PLCI_DISPLAYC) == HIGH ? 4 : 0);
    u1 += (digitalRead(PLCI_DISPLAYD) == HIGH ? 8 : 0);

    delayMicroseconds(50);

    uint8_t u2 = 0;
    u2 += (digitalRead(PLCI_DISPLAYA) == HIGH ? 1 : 0);
    u2 += (digitalRead(PLCI_DISPLAYB) == HIGH ? 2 : 0);
    u2 += (digitalRead(PLCI_DISPLAYC) == HIGH ? 4 : 0);
    u2 += (digitalRead(PLCI_DISPLAYD) == HIGH ? 8 : 0);

    return (u1 == u2) ? u1 : u2;
}

void io_clear_all_buttons()
{
    digitalWrite(PLCO_REGISTER, LOW);
    digitalWrite(PLCO_ANILOX, LOW);
    digitalWrite(PLCO_CLICHE, LOW);
    digitalWrite(PLCO_COLOR, LOW);
}

// Non-blocking deterministic software output multiplexing per PLC scan slice
void io_write_buttons(uint16_t _buttons)
{
    if (_buttons == 0)
    {
        io_clear_all_buttons();
        return;
    }

    uint8_t mux = io_read_encode_mux();

    switch (mux)
    {
    case 0:
        digitalWrite(PLCO_REGISTER, (_buttons & BUTTON_REGISTER_AVANTI) ? HIGH : LOW);
        digitalWrite(PLCO_ANILOX, (_buttons & BUTTON_ANILOX_AV_ANTERIORE) ? HIGH : LOW);
        digitalWrite(PLCO_CLICHE, (_buttons & BUTTON_CLICHE_AV_ANTERIORE) ? HIGH : LOW);
        digitalWrite(PLCO_COLOR, (_buttons & BUTTON_COLOR_ATTACCO) ? HIGH : LOW);
        break;
    case 1:
        digitalWrite(PLCO_REGISTER, (_buttons & BUTTON_REGISTER_INDIETRO) ? HIGH : LOW);
        digitalWrite(PLCO_ANILOX, (_buttons & BUTTON_ANILOX_IN_ANTERIORE) ? HIGH : LOW);
        digitalWrite(PLCO_CLICHE, (_buttons & BUTTON_CLICHE_IN_ANTERIORE) ? HIGH : LOW);
        digitalWrite(PLCO_COLOR, (_buttons & BUTTON_COLOR_STACCO) ? HIGH : LOW);
        break;
    case 2:
        digitalWrite(PLCO_REGISTER, (_buttons & BUTTON_REGISTER_ABTERIORE) ? HIGH : LOW);
        digitalWrite(PLCO_ANILOX, (_buttons & BUTTON_ANILOX_AV_POSTERIORE) ? HIGH : LOW);
        digitalWrite(PLCO_CLICHE, (_buttons & BUTTON_CLICHE_AV_POSTERIORE) ? HIGH : LOW);
        digitalWrite(PLCO_COLOR, (_buttons & BUTTON_COLOR_COLORE) ? HIGH : LOW);
        break;
    case 3:
        digitalWrite(PLCO_REGISTER, (_buttons & BUTTON_REGISTER_POSTERIORE) ? HIGH : LOW);
        digitalWrite(PLCO_ANILOX, (_buttons & BUTTON_ANILOX_IN_POSTERIORE) ? HIGH : LOW);
        digitalWrite(PLCO_CLICHE, (_buttons & BUTTON_CLICHE_IN_POSTERIORE) ? HIGH : LOW);
        digitalWrite(PLCO_COLOR, LOW);
        break;

    default:
        // No valid mux active: keep outputs low
        io_clear_all_buttons();
        break;
    }
}

void io_write_faststop(bool _state)
{
    digitalWrite(PLCO_FASTSTOP, _state ? HIGH : LOW);
}

void io_write_enable(bool _state)
{
    digitalWrite(PLCO_ENABLE, _state ? HIGH : LOW);
}