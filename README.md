# wireless-ts-controlpanel-reciever

Firmware for the **receiver** node of the wireless handheld control panel. It runs
on an Arduino UNO (with build configs for ESP32 / ESP8266) and communicates with
the transmitter (TS) unit over an nRF24L01+ radio using the
[RF24](https://github.com/nRF24/RF24) library. The receiver drives an LCD and a
TM16xx-based display, reads button state, and reports battery voltage.

## Hardware

nRF24L01+ wiring per platform:

| Module | Arduino UNO | ESP32 | ESP8266 |
| ------ | ----------- | ----- | ------- |
| GND    | GND         | GND   | GND     |
| Vcc    | 3.3V (ext.) | 3.3V  | 3.3V    |
| CE     | D9          | D4    | D4      |
| CSN    | D10         | D5    | D8      |
| CLK    | D13         | D18   | D5      |
| MOSI   | D11         | D23   | D7      |
| MISO   | D12         | D19   | D6      |

Battery voltage is sampled on `A0` through a voltage divider
(`VOLTAGE_DIVIDER_COEFFICIENT`), with full/empty thresholds of 12 V / 10 V.

## Protocol

The receiver (`DEVICE_MASTER`, address `RX0`) exchanges a `transaction_unit`
struct with the TS slave (address `TS1`):

```c
typedef struct transaction_unit {
    uint8_t command;      // commands_enum: NOTHING/START_TX/BUTTON/STOP_TX/DATA/ACK
    uint8_t buttons;
    uint8_t active_unit;
} transaction_unit;
```

The radio channel is set at build time via `-DCHANNEL` (default `42`).

## Project layout

| Path        | Purpose                                            |
| ----------- | -------------------------------------------------- |
| `src/`      | Application sources (`main`, `radio`, `io`, `lcd`, `buttons`, `error`, `utils`) |
| `src/header.h` | Pin maps, addresses, command and status enums   |
| `lib/`      | Vendored libraries (RF24, LiquidCrystal, TM16xx)   |
| `test/`     | Unity unit tests and hardware mocks (`test/mocks/`) |
| `platformio.ini` | Build environments                            |

## Build & upload

This is a [PlatformIO](https://platformio.org/) project.

```sh
# Production build for Arduino UNO
pio run -e uno

# Build and upload
pio run -e uno -t upload

# Debug build (DEBUG=1, TEST=1, serial logging at 9600 baud)
pio run -e uno-debug -t upload
```

### Tests

Unit tests run on the `native` environment with the Unity framework and the
mocks under `test/mocks/`:

```sh
pio test -e native
```

## Build flags

| Flag        | Default | Meaning                          |
| ----------- | ------- | -------------------------------- |
| `DEBUG`     | `0`     | Enable serial debug logging      |
| `TEST`      | `0`     | Enable on-device test loop       |
| `CHANNEL`   | `42`    | nRF24 RF channel                 |
