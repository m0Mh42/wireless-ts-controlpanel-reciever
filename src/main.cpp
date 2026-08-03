#include <Arduino.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include "header.h"
#include "radio.h"
#include "error.h"
#include "buttons.h"
#ifdef TEST
#include "test_tm1637.h"
#endif
#include "io.h"
#include "lcd.h"

// Local Global-Variables
RF24 radio(CE_PIN, CSN_PIN); // RF24 Radio
transaction_unit local_transaction_unit;
ts_status local_ts_status = TS_STAT_OFF;
uint8_t active_unit = 0;
uint16_t buttons;
bool rx = false;

void setup()
{
  if (DEBUG || TEST)
  {
    Serial.begin(9600);
    while (!Serial)
      ; // Wait for USB Serial
  }

  // Setup Radio
  radio_setup(&radio);

#ifdef TEST
  tm1637_init();
#endif

  // Setup LCD
  lcd_init();
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
#ifdef TEST
      tm1637_show_status(local_ts_status, test_transaction_unit.active_unit, test_transaction_unit.buttons);
#endif
      delay(250);
    }
  }

  // Recieve and Parse and Validate Data
  if (radio_read(&radio, &local_transaction_unit))
  {
    rx = true;
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

  // Do GPIO pin changes
  // Wait for PLC to multiplex and read all buttons
  // TODO Buttons have to be non-permanent commands
  io_write_buttons(buttons);

  // Read active unit
  active_unit = io_read_active_unit();

  // Send ACK packet after recieving and executing button commands.
  //      that get executed and reverted for a period of time.
  if (rx == true)
  {
    local_transaction_unit.command = COMM_DATA;
    local_transaction_unit.active_unit = active_unit;
    local_transaction_unit.buttons = 0;
    radio_transact(&radio, &local_transaction_unit);
    
    lcd_update_status(local_ts_status, active_unit, buttons, true);
    rx = false;
  }
  else
  {
    lcd_update_status(local_ts_status, active_unit, buttons, false);
  }
}
