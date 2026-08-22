#include "io.h"

void io_init()
{
    // Configure PLC multiplexer input pins
    pinMode(PLCI_MUX1, INPUT);
    pinMode(PLCI_MUX2, INPUT);
    pinMode(PLCI_MUX3, INPUT);
    pinMode(PLCI_MUX4, INPUT);

    // Configure PLC active unit BCD display inputs
    pinMode(PLCI_DISPLAYA, INPUT);
    pinMode(PLCI_DISPLAYB, INPUT);
    pinMode(PLCI_DISPLAYC, INPUT);
    pinMode(PLCI_DISPLAYD, INPUT);

    // Configure PLC button outputs
    pinMode(PLCO_REGISTER, OUTPUT);
    pinMode(PLCO_ANILOX, OUTPUT);
    pinMode(PLCO_CLICHE, OUTPUT);
    pinMode(PLCO_COLOR, OUTPUT);

    // Configure FASTSTOP safety output
    pinMode(PLCO_FASTSTOP, OUTPUT);

    io_clear_all_buttons();
    io_write_faststop(false); // Normal running state on boot
}

uint8_t io_read_encode_mux()
{
    if (digitalRead(PLCI_MUX1) == HIGH)
        return 0;
    if (digitalRead(PLCI_MUX2) == HIGH)
        return 1;
    if (digitalRead(PLCI_MUX3) == HIGH)
        return 2;
    if (digitalRead(PLCI_MUX4) == HIGH)
        return 3;
    return 255; // No valid MUX active
}

uint8_t io_read_active_unit()
{
    uint8_t active_unit = 0;
    active_unit += (digitalRead(PLCI_DISPLAYA) == HIGH ? 1 : 0);
    active_unit += (digitalRead(PLCI_DISPLAYB) == HIGH ? 2 : 0);
    active_unit += (digitalRead(PLCI_DISPLAYC) == HIGH ? 4 : 0);
    active_unit += (digitalRead(PLCI_DISPLAYD) == HIGH ? 8 : 0);
    return active_unit;
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
    if (_buttons == 0) {
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