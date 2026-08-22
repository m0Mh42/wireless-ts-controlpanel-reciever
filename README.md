# UTECO Wireless Control Panel — Receiver Firmware

Industrial safety firmware for the **receiver (PLC Interface Base Station)** node of the UTECO flexographic printing press wireless control system. It runs on an ATmega328P (Arduino Uno) and implements the **EN 50159 / IEC 61784-3 Black Channel Safety Protocol** over an nRF24L01+ 2.4 GHz transceiver.

---

## Safety Architecture (EN 50159 / IEC 61784-3)

- **Black Channel Integrity**: No safety integrity is credited to the nRF24L01+ hardware or 2.4 GHz RF medium.
- **IEEE 802.3 CRC-32**: Full 32-bit polynomial (`0xEDB88320`) protecting each 32-byte frame against random and burst transmission errors.
- **Flash ROM CRC Optimization**: Lookup table stored in `PROGMEM` (`pgm_read_dword`), consuming **0 bytes of RAM** and reserving 70%+ of SRAM for stack/application safety.
- **Dual-Rail Verification**: Checks that `buttons_primary ^ buttons_complement == 0xFFFF` on every frame to detect single-event upsets (SEU) and RAM faults.
- **Strict Monotonic Sequence Counter**: Defends against stale, delayed, or replayed packets.
- **100 ms Safety Watchdog**: If communication is lost or corrupted for > 100 ms, **all PLC button outputs (`PLCO_REGISTER`, `PLCO_ANILOX`, `PLCO_CLICHE`, `PLCO_COLOR`) are instantly zeroed and de-energized**.
- **Deterministic Non-Blocking IO**: $O(1)$ constant-time multiplexer driver synchronized with PLC multiplexer slices.

---

## Hardware Pinout & Wiring (ATmega328P / Arduino Uno)

### 1. nRF24L01+ Radio Interface
| Signal | Arduino Uno Pin | Description |
| :--- | :--- | :--- |
| **VCC** | 3.3V (Ext. Filtered) | 3.3V DC Power with 10µF + 100nF decoupling |
| **GND** | GND | Common Ground |
| **CE** | D9 | Chip Enable (Rx/Tx mode transition) |
| **CSN** | D10 | SPI Chip Select Not |
| **SCK** | D13 | SPI Clock |
| **MOSI** | D11 | Master Out Slave In |
| **MISO** | D12 | Master In Slave Out |

### 2. UTECO Machine PLC Interface
| Signal | Pin | Direction | Function |
| :--- | :--- | :--- | :--- |
| `PLCI_MUX1` | A0 (Pin 14) | Input | PLC Scan Slice 0 Select (Register Avanti, Anilox Av Ant, Cliche Av Ant, Color Attacco) |
| `PLCI_MUX2` | A1 (Pin 15) | Input | PLC Scan Slice 1 Select (Register Indietro, Anilox In Ant, Cliche In Ant, Color Stacco) |
| `PLCI_MUX3` | A2 (Pin 16) | Input | PLC Scan Slice 2 Select (Register Abteriore, Anilox Av Post, Cliche Av Post, Color Colore) |
| `PLCI_MUX4` | A3 (Pin 17) | Input | PLC Scan Slice 3 Select (Register Posteriore, Anilox In Post, Cliche In Post, Spare) |
| `PLCI_DISPLAYA` | D4 | Input | BCD Active Printing Unit Bit 0 (Weight 1) |
| `PLCI_DISPLAYB` | D5 | Input | BCD Active Printing Unit Bit 1 (Weight 2) |
| `PLCI_DISPLAYC` | D6 | Input | BCD Active Printing Unit Bit 2 (Weight 4) |
| `PLCI_DISPLAYD` | D7 | Input | BCD Active Printing Unit Bit 3 (Weight 8) |
| `PLCO_REGISTER` | D0 | Output | Register Actuation Output |
| `PLCO_ANILOX` | D1 | Output | Anilox Actuation Output |
| `PLCO_CLICHE` | D2 | Output | Cliche Actuation Output |
| `PLCO_COLOR` | D3 | Output | Color Actuation Output |
| `PLCO_FASTSTOP` | D8 | Output | Emergency Fast Stop Line |

### 3. I2C Telemetry Display (20x4 Character LCD)
| Signal | Pin | Description |
| :--- | :--- | :--- |
| **SDA** | A4 | I2C Data Line (Address `0x27` or `0x3F`) |
| **SCL** | A5 | I2C Clock Line |

---

## Wi-Fi Guard Channel Hopping

Operates at **250 kbps** data rate (+12 dB sensitivity margin over 2 Mbps) across Wi-Fi inter-band guard channels:
- **Channel 04 (2404 MHz)**
- **Channel 28 (2428 MHz)**
- **Channel 54 (2454 MHz)**
- **Channel 78 (2478 MHz)** — Default rendezvous channel
- **Channel 82 (2482 MHz)**

**Receiver Agility**: If no valid safety frame is received for > 500 ms, the receiver automatically hops to the next guard channel to re-synchronize.

---

## Build, Upload & Test

This is a [PlatformIO](https://platformio.org/) project.

```sh
# Run native unit tests on host (24/24 tests)
pio test -e native

# Compile production firmware for Arduino Uno
pio run -e uno

# Upload to Arduino Uno
pio run -e uno -t upload
```

### Memory Footprint (ATmega328P)
- **RAM**: 646 bytes / 2048 bytes (**31.5% utilized**, 1.4 KB free)
- **Flash ROM**: 10,720 bytes / 32,256 bytes (**33.2% utilized**)
