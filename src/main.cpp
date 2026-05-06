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

#define DEBUG false
#define TEST true

// Local Global-Variables
RF24 radio(CE_PIN, CSN_PIN); // RF24 Radio
transaction_unit local_transaction_unit;
uint64_t local_seq = 0;
ts_status local_ts_status = TS_STAT_OFF;
uint8_t local_buttons = 0, battery_cap = 0;
bool flag = false;

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
}

void loop()
{
  while (TEST)
  {
    transaction_unit test_transaction_unit;
    bool rx = false;
    while (true)
    {
      if (radio.available())
      {
        radio.read(&test_transaction_unit, sizeof(transaction_unit));
        if (test_transaction_unit.command == COMM_START_TX && test_transaction_unit.seq == 0)
        {
          rx = true;
          local_seq = 1;
          break;
        }
      }
      delay(250);
    }
    while (true)
    {
      if (rx)
      {
        test_transaction_unit.command = COMM_ACK;
        test_transaction_unit.seq = local_seq;
        test_transaction_unit.buttons = 0;
        radio.write(&test_transaction_unit, sizeof(test_transaction_unit));
        local_seq++;
      }
      if (radio.available())
      {
        radio.read(&test_transaction_unit, sizeof(transaction_unit));
        if (test_transaction_unit.command == COMM_BUTTON && test_transaction_unit.seq == local_seq)
        {
          Serial.println(test_transaction_unit.buttons);
          rx = true;
          local_seq++;
        }
        if (test_transaction_unit.command == COMM_STOP_TX && test_transaction_unit.seq == local_seq)
        {
          Serial.println("Stop RX");
          rx = true;
          local_seq++;
          break;
        }
      }
      delay(1000);
    }
  }
}
/*if (TEST)
  {
    transaction_unit test_transaction_unit;
    uint64_t local_seq = 0;
    test_transaction_unit.command = COMM_START_TX;
    test_transaction_unit.seq = local_seq;
    test_transaction_unit.buttons = 0U;
    radio.write(&test_transaction_unit, sizeof(test_transaction_unit));
    while (true)
    {
      test_transaction_unit.command = COMM_BUTTON;
      test_transaction_unit.seq++;
      radio.write(&test_transaction_unit, sizeof(test_transaction_unit));
      if (radio.available())
      {
        radio.read(&test_transaction_unit, sizeof(test_transaction_unit));
        if (test_transaction_unit.command == COMM_ACK)
        {
          if (test_transaction_unit.seq != (local_seq + 1))
          {
            test_transaction_unit.command = COMM_STOP_TX;
            test_transaction_unit.seq = 0U;
            radio.write(&test_transaction_unit, sizeof(test_transaction_unit));
            break;
          }
          continue;
        }
      }
    }
  }*/