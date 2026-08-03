#include "lcd.h"

// Set the LCD address to 0x27 for a 20 chars and 4 line display
LiquidCrystal_I2C lcd(0x27, 20, 4); 

void lcd_init() {
    lcd.init();
    lcd.backlight();
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Receiver Node");
    lcd.setCursor(15, 0);
    lcd.print("L: NO");
}

void lcd_update_status(ts_status status, uint8_t active_unit, uint16_t buttons, bool rx) {
    static ts_status last_status = (ts_status)-1;
    static uint8_t last_unit = 255;
    static uint16_t last_buttons = 0xFFFF;
    static bool last_rx = false;
    static unsigned long last_rx_time = 0;
    if (rx) {
        last_rx_time = millis();
    }
    bool link_ok = (millis() - last_rx_time) < 1000;

    if (last_rx != link_ok) {
        lcd.setCursor(15, 0);
        if (link_ok) lcd.print("L: OK");
        else         lcd.print("L: NO");
        last_rx = link_ok;
    }

    if (last_status != status) {
        lcd.setCursor(0, 1);
        lcd.print("TS Status:          ");
        lcd.setCursor(11, 1);
        if (status == TS_STAT_ON) lcd.print("ON");
        else if (status == TS_STAT_OFF) lcd.print("OFF");
        else if (status == TS_STAT_BATTERYLOW) lcd.print("BAT_LOW");
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
        lcd.print("Buttons:            ");
        lcd.setCursor(9, 3);
        lcd.print(buttons, HEX);
        last_buttons = buttons;
    }
}
