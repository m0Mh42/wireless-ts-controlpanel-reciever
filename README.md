# UTECO Wireless Control Panel — Receiver Firmware

Industrial safety firmware for the **receiver (PLC Interface Base Station)** node of the UTECO flexographic printing press wireless control system. It runs on an ATmega328P (Arduino Uno) and implements the **EN 50159 / IEC 61784-3 Black Channel Safety Protocol** over an nRF24L01+ 2.4 GHz transceiver.

---

## Safety Architecture (EN 50159 / IEC 61784-3)

- **Black Channel Integrity**: No safety integrity is credited to the nRF24L01+ hardware or 2.4 GHz RF medium.
- **IEEE 802.3 CRC-32**: Full 32-bit polynomial (`0xEDB88320`) protecting each 32-byte frame against random and burst transmission errors.
- **Flash ROM CRC Optimization**: Lookup table stored in `PROGMEM` (`pgm_read_dword`), consuming **0 bytes of RAM** and reserving 70%+ of SRAM for stack/application safety.
- **Dual-Rail Verification**: Checks that `buttons_primary ^ buttons_complement == 0xFFFF` on every frame to detect single-event upsets (SEU) and RAM faults.
- **Strict Monotonic Sequence Counter**: Defends against stale, delayed, or replayed packets.
- **150 ms Safety Watchdog**: If communication is lost or corrupted for > 150 ms, **all PLC button outputs (`PLCO_REGISTER`, `PLCO_ANILOX`, `PLCO_CLICHE`, `PLCO_COLOR`) are instantly zeroed and de-energized**.
- **Auto-Resynchronization**: Automatically detects transmitter reboot (seq <= 5) and re-synchronizes without requiring manual power cycling or handshake locks.
- **Deterministic Non-Blocking IO**: $O(1)$ constant-time multiplexer driver synchronized with PLC multiplexer slices.

---

## Hardware Pinout & Wiring (Arduino Pro Mini / Uno)

### 1. nRF24L01+ Radio Interface (Connector J3: B7B-XH-A)
| Signal | Pin | Direction | Description |
| :--- | :--- | :--- | :--- |
| **VDD** | 3.3V (Ext. Filtered) | Power | 3.3V DC Power with 10µF + 100nF decoupling |
| **GND** | GND | Power | Common Ground |
| **CE** | D13 (Pin 13) | Output | Chip Enable (Rx/Tx mode transition) |
| **SCK** | D12 (Pin 12) | Output | SPI Clock |
| **MISO** | D11 (Pin 11) | Input | Master In Slave Out |
| **MOSI** | D10 (Pin 10) | Output | Master Out Slave In |
| **CSN** | A7 (Pin 21) | Output | SPI Chip Select Not |

### 2. UTECO Machine PLC Interface
| Signal | Pin | Direction | Function |
| :--- | :--- | :--- | :--- |
| `PLCI_DISPLAYA` | D2 | Input | BCD Active Printing Unit Bit 0 (Weight 1) |
| `PLCI_DISPLAYB` | D3 | Input | BCD Active Printing Unit Bit 1 (Weight 2) |
| `PLCI_DISPLAYC` | D4 | Input | BCD Active Printing Unit Bit 2 (Weight 4) |
| `PLCI_DISPLAYD` | D5 | Input | BCD Active Printing Unit Bit 3 (Weight 8) |
| `PLCI_MUX1` | D6 | Input | PLC Scan Slice 0 Select (Register Avanti, Anilox Av Ant, Cliche Av Ant, Color Attacco) |
| `PLCI_MUX2` | D7 | Input | PLC Scan Slice 1 Select (Register Indietro, Anilox In Ant, Cliche In Ant, Color Stacco) |
| `PLCI_MUX3` | D8 | Input | PLC Scan Slice 2 Select (Register Abteriore, Anilox Av Post, Cliche Av Post, Color Colore) |
| `PLCI_MUX4` | D9 | Input | PLC Scan Slice 3 Select (Register Posteriore, Anilox In Post, Cliche In Post, Spare) |
| `PLCO_REGISTER` | A0 (Pin 14) | Output | Register Actuation Output |
| `PLCO_ANILOX` | A1 (Pin 15) | Output | Anilox Actuation Output |
| `PLCO_CLICHE` | A2 (Pin 16) | Output | Cliche Actuation Output |
| `PLCO_COLOR` | A3 (Pin 17) | Output | Color Actuation Output |
| `PLCO_FASTSTOP` | D1 (TXO_2) | Output | Emergency Fast Stop Line |
| `PLCO_ENABLE` | D0 (RXI_2) | Output | PLC Output Enable Safety Line |

### 3. TM1638 Telemetry LED Display (Connector P3: B5B-XH-A)
Mates directly with `ALC-40266RGY2` dual 4-digit 7-segment display module (`Transmitter-Display.SchDoc`):
| Signal | Pin | Direction | Description |
| :--- | :--- | :--- | :--- |
| **GND** | GND (Pin 1) | Power | Common Ground |
| **TM_DIO** | A6 (Pin 20) | Bidirectional | TM1638 Serial Data Input/Output |
| **TM_CLK** | A4 (Pin 18) | Output | TM1638 Serial Clock |
| **TM_STB** | A5 (Pin 19) | Output | TM1638 Chip Strobe |
| **VCC** | VCC 5V (Pin 5) | Power | 5V DC Supply |

---

## RF Configuration & Dual Protocol Modes

- **RF Channel**: Pinned to **Channel 78 (2478 MHz)** by default (Wi-Fi guard band above channel 11).
- **RF Output Power**: Configured to `RF24_PA_MAX` for maximum industrial floor range.
- **Data Rate**: Standardized on **1 Mbps** (`RF24_1MBPS`) with 16-bit hardware CRC.
- **Protocol Modes**:
  - `RF_MODE_ACK_PAYLOAD` (Default): Dynamic ACK payloads loaded into FIFO via `writeAckPayload()`.
  - `RF_MODE_PING_PONG`: Master-Slave request-response mode via separate RX/TX pipes.

---

## Build, Upload & Test

This is a [PlatformIO](https://platformio.org/) project.

```sh
# Run native unit tests on host (33/33 tests)
pio test -e native

# Compile production firmware for Arduino Uno / Pro Mini (ACK payload mode)
pio run -e uno

# Compile production firmware for Arduino Uno / Pro Mini (Ping-Pong mode)
pio run -e uno-pingpong

# Upload to Arduino Uno / Pro Mini
pio run -e uno -t upload
```

### Memory Footprint (ATmega328P)
- **RAM**: 415 bytes / 2048 bytes (**20.3% utilized**, 1.63 KB free)
- **Flash ROM**: 10,750 bytes / 30,720 bytes (**35.0% utilized**)
