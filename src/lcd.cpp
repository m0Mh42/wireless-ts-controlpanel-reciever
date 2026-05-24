#include "lcd.h"

LCD_Manager::LCD_Manager()
    : lcd(LCD_RS_PIN, LCD_E_PIN, LCD_D4_PIN, LCD_D5_PIN, LCD_D6_PIN, LCD_D7_PIN),
      last_update_time(0),
      display_page(0)
{
}

void LCD_Manager::init()
{
    // Initialize 16x2 LCD
    lcd.begin(16, 2);
    lcd.clear();
    display_startup();
}

void LCD_Manager::display_startup()
{
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("RF24 Receiver");
    lcd.setCursor(0, 1);
    lcd.print("Initializing...");
    delay(2000);
    lcd.clear();
}

void LCD_Manager::display_error(const char *error_msg)
{
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("ERROR");
    lcd.setCursor(0, 1);
    lcd.print(error_msg);
}

void LCD_Manager::update(uint8_t active_unit, uint16_t buttons, bool radio_rx, ts_status ts_stat)
{
    unsigned long current_time = millis();

    // Throttle updates to prevent flickering
    if (current_time - last_update_time < UPDATE_INTERVAL)
    {
        return;
    }

    last_update_time = current_time;

    // Alternate between status and buttons display
    if (radio_rx || buttons > 0)
    {
        display_page = (current_time / 2000) % 2; // Switch every 2 seconds
    }
    else
    {
        display_page = 0; // Show only status if no activity
    }

    if (display_page == 0)
    {
        display_status_page(active_unit, radio_rx, ts_stat);
    }
    else
    {
        display_buttons_page(buttons);
    }
}

void LCD_Manager::display_status_page(uint8_t active_unit, bool radio_rx, ts_status ts_stat)
{
    char ts_status_str[6];
    get_ts_status_string(ts_stat, ts_status_str);

    lcd.clear();

    // Line 1: Active Unit and Radio Status
    lcd.setCursor(0, 0);
    lcd.print("U:");
    lcd.print(active_unit);
    lcd.print(" RX:");
    lcd.print(radio_rx ? "ON " : "OFF");
    lcd.print(" TS:");
    lcd.print(ts_status_str);

    // Line 2: Status indicators
    lcd.setCursor(0, 1);
    if (radio_rx)
    {
        lcd.print("RX Active");
    }
    else
    {
        lcd.print("Waiting...");
    }

    if (ts_stat == TS_STAT_BATTERYLOW)
    {
        lcd.print(" LOW!");
    }
}

void LCD_Manager::display_buttons_page(uint16_t buttons)
{
    char button_str[16];
    get_button_names(buttons, button_str);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Buttons:");
    lcd.setCursor(0, 1);
    lcd.print(button_str);
}

void LCD_Manager::get_ts_status_string(ts_status ts_stat, char *buffer)
{
    switch (ts_stat)
    {
    case TS_STAT_OFF:
        strcpy(buffer, "OFF");
        break;
    case TS_STAT_ON:
        strcpy(buffer, "OK");
        break;
    case TS_STAT_BATTERYLOW:
        strcpy(buffer, "LOW");
        break;
    default:
        strcpy(buffer, "???");
        break;
    }
}

void LCD_Manager::get_button_names(uint16_t buttons, char *buffer)
{
    buffer[0] = '\0';
    int pos = 0;
    const char *button_names[] = {
        "P1", "P2", "P3", "P4", "P5", "P6", "P7", "P8", "P9", "P10", "P11", "P12", "P13", "P14", "P15", "P16"};

    for (int i = 0; i < 16 && pos < 15; i++)
    {
        if (buttons & (1 << i))
        {
            if (pos > 0)
            {
                buffer[pos++] = ',';
            }
            const char *name = button_names[i];
            int name_len = strlen(name);
            if (pos + name_len <= 15)
            {
                strcpy(buffer + pos, name);
                pos += name_len;
            }
        }
    }
    buffer[pos] = '\0';

    // If no buttons pressed, show a dash
    if (buffer[0] == '\0')
    {
        strcpy(buffer, "None");
    }
}
