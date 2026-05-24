#ifndef _H_LCD
#define _H_LCD 1

#include <Arduino.h>
#include <LiquidCrystal.h>
#include "header.h"

/*
  LCD Pinout (16x2 or 16x4):
  RS     -> D2
  E      -> D3
  D4     -> D4
  D5     -> D5
  D6     -> D6
  D7     -> D7
  GND    -> GND
  Vcc    -> 5V
  RW     -> GND (Write Only)
  VO     -> Potentiometer (Contrast)
  A      -> 5V with 330Ω resistor (Backlight)
  K      -> GND (Backlight)
*/

#define LCD_RS_PIN 2
#define LCD_E_PIN 3
#define LCD_D4_PIN 4
#define LCD_D5_PIN 5
#define LCD_D6_PIN 6
#define LCD_D7_PIN 7

class LCD_Manager
{
private:
    LiquidCrystal lcd;
    unsigned long last_update_time;
    uint8_t display_page;                             // 0 = Status, 1 = Buttons
    static const unsigned long UPDATE_INTERVAL = 500; // 500ms between updates

public:
    LCD_Manager();
    void init();
    void update(uint8_t active_unit, uint16_t buttons, bool radio_rx, ts_status ts_stat);
    void display_startup();
    void display_error(const char *error_msg);

private:
    void display_status_page(uint8_t active_unit, bool radio_rx, ts_status ts_stat);
    void display_buttons_page(uint16_t buttons);
    void get_ts_status_string(ts_status ts_stat, char *buffer);
    void get_button_names(uint16_t buttons, char *buffer);
};

#endif
