#include <Arduino.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include "header.h"
#include "radio.h"
#include "error.h"
#include "buttons.h"
#include "lcd.h"

// Local Global-Variables
RF24 radio(CE_PIN, CSN_PIN); // RF24 Radio
LCD_Manager lcd_manager;     // LCD Manager
transaction_unit local_transaction_unit;
ts_status local_ts_status = TS_STAT_OFF;
uint8_t active_unit = 0;
uint16_t buttons;
bool rx = false, ack = true;

void setup()
{
  if (DEBUG || TEST)
  {
    Serial.begin(9600);
    while (!Serial)
      ; // Wait for USB Serial
  }

  // Setup LCD
  lcd_manager.init();

  // Setup Radio
  radio_setup(&radio);
}

void loop()
{
  while (TEST)
  {
    transaction_unit test_transaction_unit;
    test_transaction_unit.active_unit = 0;
    test_transaction_unit.buttons = 0;
    test_transaction_unit.command = COMM_NOTHING;
    rx = false;
    while (true)
    {
      if (rx)
      {
        test_transaction_unit.command = COMM_ACK;
        test_transaction_unit.buttons = 0;
        test_transaction_unit.active_unit = active_unit++;
        radio_transact(&radio, &test_transaction_unit);
      }
      if (radio_read(&radio, &test_transaction_unit))
      {
        if (test_transaction_unit.command == COMM_START_TX)
        {
          rx = true;
          Serial.println("Established Communication");
          break;
        }
        else if (test_transaction_unit.command == COMM_BUTTON)
        {
          Serial.print("Data: ");
          Serial.println(test_transaction_unit.buttons);
          rx = true;
        }
        else if (test_transaction_unit.command == COMM_STOP_TX)
        {
          Serial.println("Stop RX");
          rx = true;
          break;
        }
      }
      delay(250);
    }
  }

  // Recieve and Parse and Validate Data
  if (radio_read(&radio, &local_transaction_unit))
  {
    rx = true;
    ack = false;
    switch (local_transaction_unit.command)
    {
    case COMM_START_TX:
      active_unit = 0;
      buttons = 0;
      local_ts_status = TS_STAT_ON;
      break;
    case COMM_BUTTON:
      buttons = buttons_process(local_transaction_unit.buttons);
      break;
    case COMM_STOP_TX:
      active_unit = 0;
      buttons = 0;
      local_ts_status = TS_STAT_OFF;
      break;
    default:
      break;
    }
  }

  // Update LCD Display with current status
  lcd_manager.update(active_unit, buttons, rx, local_ts_status);

  // TODO Do GPIO Pin Changes
}
