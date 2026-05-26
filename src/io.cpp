#include "io.h"

uint8_t io_read_encode_mux()
{
    uint8_t mux = 0;
    if (digitalRead(PLCI_MUX1))
        mux = 0;
    else if (digitalRead(PLCI_MUX2))
        mux = 1;
    else if (digitalRead(PLCI_MUX3))
        mux = 2;
    else if (digitalRead(PLCI_MUX4))
        mux = 3;
    else
        mux = 255;
    return mux;
}

uint8_t io_read_active_unit()
{
    uint8_t active_unit = 0;
    active_unit += digitalRead(PLCI_DISPLAYA);
    active_unit += digitalRead(PLCI_DISPLAYB) << 1;
    active_unit += digitalRead(PLCI_DISPLAYC) << 2;
    active_unit += digitalRead(PLCI_DISPLAYD) << 3;
    return active_unit;
}

// Software output multiplexing
void io_write_buttons(uint16_t _buttons)
{
    uint8_t mux = 0, last_mux = 255, mux_times = 0;
    while (true)
    {
        if (mux_times == 4)
            break;

        mux = io_read_encode_mux();
        if (mux != last_mux)
        {
            mux_times++;
            last_mux = mux;
            switch (mux)
            {
            case 0:
                digitalWrite(PLCO_REGISTER, _buttons & BUTTON_REGISTER_AVANTI);
                digitalWrite(PLCO_ANILOX, _buttons & BUTTON_ANILOX_AV_ANTERIORE);
                digitalWrite(PLCO_CLICHE, _buttons & BUTTON_CLICHE_AV_ANTERIORE);
                digitalWrite(PLCO_COLOR, _buttons & BUTTON_COLOR_ATTACCO);
                break;
            case 1:
                digitalWrite(PLCO_REGISTER, _buttons & BUTTON_REGISTER_INDIETRO);
                digitalWrite(PLCO_ANILOX, _buttons & BUTTON_ANILOX_IN_ANTERIORE);
                digitalWrite(PLCO_CLICHE, _buttons & BUTTON_CLICHE_IN_ANTERIORE);
                digitalWrite(PLCO_COLOR, _buttons & BUTTON_COLOR_STACCO);
                break;
            case 2:
                digitalWrite(PLCO_REGISTER, _buttons & BUTTON_REGISTER_ABTERIORE);
                digitalWrite(PLCO_ANILOX, _buttons & BUTTON_ANILOX_AV_POSTERIORE);
                digitalWrite(PLCO_CLICHE, _buttons & BUTTON_CLICHE_AV_POSTERIORE);
                digitalWrite(PLCO_COLOR, _buttons & BUTTON_COLOR_COLORE);
                break;
            case 3:
                digitalWrite(PLCO_REGISTER, _buttons & BUTTON_REGISTER_POSTERIORE);
                digitalWrite(PLCO_ANILOX, _buttons & BUTTON_ANILOX_IN_POSTERIORE);
                digitalWrite(PLCO_CLICHE, _buttons & BUTTON_CLICHE_IN_POSTERIORE);
                digitalWrite(PLCO_COLOR, LOW);
                break;

            default:
                break;
            }
        }
    }
}

void io_write_faststop(bool _state)
{
    digitalWrite(PLCO_FASTSTOP, _state);
}