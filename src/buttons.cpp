#include "buttons.h"

uint16_t buttons_process(uint8_t _buttons)
{
    uint16_t expanded_buttons = 0;
    for (size_t i = 0; i < BUTTONS_ROWS; i++)
    {
        if ((_buttons >> (7 - i)) & 0x01)
        {
            expanded_buttons |= (_buttons & (powtwo(BUTTONS_COLUMNS)) - 1) << (15 - i * BUTTONS_COLUMNS);
        }
    }
    return expanded_buttons;
}