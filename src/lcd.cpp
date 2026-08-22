#include "lcd.h"
#include "radio.h"

// Set the LCD address to 0x27 for a 20 chars and 4 line display
LiquidCrystal_I2C lcd(0x27, 20, 4); 

void lcd_init() {
    lcd.init();
    lcd.backlight();
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("UTECO RX Safe Node  ");
    lcd.setCursor(0, 1);
    lcd.print("Initializing Radio..");
    delay(500);
    lcd.clear();
}

void lcd_update_status(ts_status status, uint8_t active_unit, uint16_t buttons, bool link_healthy) {
    static ts_status last_status = (ts_status)-1;
    static uint8_t last_unit = 255;
    static uint16_t last_buttons = 0xFFFF;
    static bool last_link = false;
    static uint8_t last_channel = 255;
    static uint8_t last_lqi = 255;

    uint8_t cur_chan = radio_get_channel();
    uint8_t cur_lqi = radio_get_lqi();

    if (last_link != link_healthy || last_channel != cur_chan || last_lqi != cur_lqi) {
        lcd.setCursor(0, 0);
        if (link_healthy) {
            lcd.print("L:OK ");
        } else {
            lcd.print("L:NO ");
        }
        lcd.print("Ch:");
        if (cur_chan < 10) lcd.print("0");
        lcd.print(cur_chan);
        lcd.print(" Q:");
        lcd.print(cur_lqi);
        lcd.print("%   ");

        last_link = link_healthy;
        last_channel = cur_chan;
        last_lqi = cur_lqi;
    }

    if (last_status != status) {
        lcd.setCursor(0, 1);
        lcd.print("TS Status:          ");
        lcd.setCursor(11, 1);
        if (status == TS_STAT_ON) lcd.print("ON");
        else if (status == TS_STAT_OFF) lcd.print("OFF");
        else if (status == TS_STAT_BATTERYLOW) lcd.print("BAT_LOW");
        else if (status == TS_STAT_FAULT) lcd.print("FAULT");
        last_status = status;
    }

    if (last_unit != active_unit) {
        lcd.setCursor(0, 2);
        lcd.print("Active Unit:        ");
        lcd.setCursor(13, 2);
        lcd.print(active_unit);
        last_unit = active_unit;
    }

    if (last_buttons != buttons) {
        lcd.setCursor(0, 3);
        lcd.print("Buttons: 0x         ");
        lcd.setCursor(11, 3);
        if (buttons < 0x1000) lcd.print("0");
        if (buttons < 0x0100) lcd.print("0");
        if (buttons < 0x0010) lcd.print("0");
        lcd.print(buttons, HEX);
        last_buttons = buttons;
    }
}
