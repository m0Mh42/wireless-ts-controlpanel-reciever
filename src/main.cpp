#include <Arduino.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include "header.h"
#include "radio.h"
#include "error.h"

/*

  Module // Arduino UNO //         ESP32         //       ESP8266
    GND    ->   GND            ->  GND           ->       GND
    Vcc    ->   3.3V(External) ->  3.3V          ->       3.3V
    CE     ->   D9             ->  D4            ->       D4
    CSN    ->   D10            ->  D5            ->       D8
    CLK    ->   D13            ->  D18           ->       D5
    MOSI   ->   D11            ->  D23           ->       D7
    MISO   ->   D12            ->  D19           ->       D6

*/

#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega328PB__) || defined(__AVR_ATmega2560__) || defined(__AVR_ATmega1280__)
#define CSN 10
#define CE 9
#elif defined(ESP32)
#define CSN 5
#define CE 4
#elif defined(ESP8266)
#define CSN D8
#define CE D4
#else
#error "Make Config yourself"
#endif

#define DEBUG true
#define TEST true

// Local Global-Variables
RF24 radio(CE_PIN, CSN_PIN); // RF24 Radio
transaction_unit local_transaction_unit;
ts_status local_ts_status = TS_STAT_OFF;
uint8_t local_buttons = 0, battery_cap = 0, local_active_unit = 0;
bool flag = false;

void setup()
{
  Serial.begin(9600);
  if (DEBUG || TEST)
  {
    // while (!Serial)
    //   ; // Wait for USB Serial
  }

  Serial.println("TEST");

  // Setup Radio
  radio_setup(&radio);

  Serial.println("TEST2");
}

void loop()
{
  while (TEST)
  {
    transaction_unit test_transaction_unit;
    test_transaction_unit.active_unit = 0;
    test_transaction_unit.buttons = 0;
    test_transaction_unit.command = COMM_NOTHING;
    bool rx = false;
    while (true)
    {
      if (rx)
      {
        test_transaction_unit.command = COMM_ACK;
        test_transaction_unit.buttons = 0;
        test_transaction_unit.active_unit = local_active_unit++;
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

  // TODO Recieve and Parse and Validate Data
  // TODO Do GPIO Pin Changes
  // TODO Connect LCD
}
