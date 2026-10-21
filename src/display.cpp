#include "display.h"
#include "radio.h"
#include <stdio.h>
#include <string.h>
#include <TM16xx.h>

// Internal 16-byte RAM mirror for TM1638 (GRID1..GRID8, 2 bytes each)
static uint8_t tm_ram[16];
static uint8_t current_brightness = 7;
static bool display_enabled = true;

// Segment-to-GRID lookup table for custom ALC-40266RGY2 display module
// Standard 7-segment bit index:
// bit 0 (a)  -> GRID5 (index 4)
// bit 1 (b)  -> GRID4 (index 3)
// bit 2 (c)  -> GRID2 (index 1)
// bit 3 (d)  -> GRID1 (index 0)
// bit 4 (e)  -> GRID3 (index 2)
// bit 5 (f)  -> GRID6 (index 5)
// bit 6 (g)  -> GRID8 (index 7)
// bit 7 (DP) -> GRID7 (index 6)
static const uint8_t FONT_BIT_TO_GRID[8] = {
    4, // a  -> GRID5 (0x08, 0x09)
    3, // b  -> GRID4 (0x06, 0x07)
    1, // c  -> GRID2 (0x02, 0x03)
    0, // d  -> GRID1 (0x00, 0x01)
    2, // e  -> GRID3 (0x04, 0x05)
    5, // f  -> GRID6 (0x0A, 0x0B)
    7, // g  -> GRID8 (0x0E, 0x0F)
    6  // DP -> GRID7 (0x0C, 0x0D)
};

// Digit to TM1638 SEG line mapping (byte offset and bit mask within that byte)
struct DigitMapping
{
    uint8_t byte_offset; // 0 for low byte (SEG1..SEG8), 1 for high byte (SEG9..SEG10)
    uint8_t bit_mask;    // bit mask in that byte
};

static const DigitMapping DIGIT_MAP[8] = {
    // Top Row: Digits 1..4 (Red)
    {0, 0x40}, // S1_UP  (SEG7)
    {0, 0x80}, // S2_UP  (SEG8)
    {1, 0x02}, // S3_UP  (SEG10)
    {1, 0x01}, // S4_UP  (SEG9)
    // Bottom Row: Digits 1..4 (Green)
    {0, 0x08}, // S1_DOWN (SEG4)
    {0, 0x04}, // S2_DOWN (SEG3)
    {0, 0x01}, // S3_DOWN (SEG1)
    {0, 0x02}  // S4_DOWN (SEG2)
};

// Left vertical bar graph (10 levels from bottom to top)
// Levels 1..2: Green bars driven by DS_SEG_L (SEG5 -> low byte bit 4, mask 0x10, Grids g and f)
// Levels 3..10: Yellow and Red bars driven by DS_SEG_B (SEG6 -> low byte bit 5, mask 0x20, Grids DP, g, f, e, d, c, b, a)
struct BarSegmentMapping
{
    uint8_t grid_index; // 0..7 (GRID1..GRID8)
    uint8_t seg_mask;   // 0x10 for SEG_L, 0x20 for SEG_B
};

static const BarSegmentMapping BAR_MAP[10] = {
    {7, 0x10}, // Level 1  (L-g,  Green bottom) -> GRID8, SEG_L
    {5, 0x10}, // Level 2  (L-f,  Green)        -> GRID6, SEG_L
    {6, 0x20}, // Level 3  (B-DP, Yellow)       -> GRID7, SEG_B
    {7, 0x20}, // Level 4  (B-g,  Yellow)       -> GRID8, SEG_B
    {5, 0x20}, // Level 5  (B-f,  Yellow)       -> GRID6, SEG_B
    {2, 0x20}, // Level 6  (B-e,  Yellow)       -> GRID3, SEG_B
    {0, 0x20}, // Level 7  (B-d,  Yellow)       -> GRID1, SEG_B
    {1, 0x20}, // Level 8  (B-c,  Red)          -> GRID2, SEG_B
    {3, 0x20}, // Level 9  (B-b,  Red)          -> GRID4, SEG_B
    {4, 0x20}  // Level 10 (B-a,  Red top)      -> GRID5, SEG_B
};

// Bottom horizontal status bar L (Indicators, 5 blocks left to right)
// Driven by DS_SEG_L (SEG5 -> low byte bit 4, mask 0x10)
static const uint8_t INDICATOR_L_GRID[5] = {
    4, // Block 1 (L-a, Green leftmost) -> GRID5
    3, // Block 2 (L-b, Red)            -> GRID4
    1, // Block 3 (L-c, Yellow)         -> GRID2
    0, // Block 4 (L-d, Yellow)         -> GRID1
    2  // Block 5 (L-e, Yellow)         -> GRID3
};

// ASCII character to 7-segment bit pattern
static uint8_t ascii_to_7seg(char c)
{
    switch (c)
    {
    case '0':
        return 0x3F;
    case '1':
        return 0x06;
    case '2':
        return 0x5B;
    case '3':
        return 0x4F;
    case '4':
        return 0x66;
    case '5':
        return 0x6D;
    case '6':
        return 0x7D;
    case '7':
        return 0x07;
    case '8':
        return 0x7F;
    case '9':
        return 0x6F;
    case '-':
        return 0x40;
    case '_':
        return 0x08;
    case 'A':
    case 'a':
        return 0x77;
    case 'B':
    case 'b':
        return 0x7C;
    case 'C':
    case 'c':
        return 0x39;
    case 'D':
    case 'd':
        return 0x5E;
    case 'E':
    case 'e':
        return 0x79;
    case 'F':
    case 'f':
        return 0x71;
    case 'H':
    case 'h':
        return 0x76;
    case 'L':
    case 'l':
        return 0x38;
    case 'M':
    case 'm':
        return 0x37;
    case 'N':
    case 'n':
        return 0x54;
    case 'O':
    case 'o':
        return 0x5C;
    case 'P':
    case 'p':
        return 0x73;
    case 'Q':
    case 'q':
        return 0x67;
    case 'R':
    case 'r':
        return 0x50;
    case 'S':
    case 's':
        return 0x6D;
    case 'T':
    case 't':
        return 0x78;
    case 'U':
        return 0x3E;
    case 'u':
        return 0x1C;
    default:
        return 0x00;
    }
}

class TM1638HardwareDriver : public TM16xx
{
public:
    TM1638HardwareDriver(uint8_t dio, uint8_t clk, uint8_t stb)
        : TM16xx(dio, clk, stb, 8, 8, true, 7) {}

    void writeRegister(uint8_t addr, uint8_t data)
    {
        sendData(addr, data);
    }
};

static TM1638HardwareDriver tm_module(DISPLAY_TM1638_DIO_PIN, DISPLAY_TM1638_CLK_PIN, DISPLAY_TM1638_STB_PIN);

void display_refresh()
{
    tm_module.setupDisplay(display_enabled, current_brightness);
    for (uint8_t addr = 0; addr < 16; addr++)
    {
        tm_module.writeRegister(addr, tm_ram[addr]);
    }
}

void display_clear()
{
    memset(tm_ram, 0, sizeof(tm_ram));
    display_refresh();
}

void display_set_brightness(uint8_t brightness)
{
    current_brightness = (brightness > 7) ? 7 : brightness;
    display_refresh();
}

void display_set_top(const char *str)
{
    // Clear top row digits (indices 0..3)
    for (uint8_t d = 0; d < 4; d++)
    {
        uint8_t offset = DIGIT_MAP[d].byte_offset;
        uint8_t mask = DIGIT_MAP[d].bit_mask;
        for (uint8_t g = 0; g < 8; g++)
        {
            tm_ram[g * 2 + offset] &= ~mask;
        }
    }

    if (!str)
        return;

    size_t len = strlen(str);
    for (uint8_t d = 0; d < 4 && d < len; d++)
    {
        uint8_t pat = ascii_to_7seg(str[d]);
        uint8_t offset = DIGIT_MAP[d].byte_offset;
        uint8_t mask = DIGIT_MAP[d].bit_mask;
        for (uint8_t b = 0; b < 8; b++)
        {
            if (pat & (1 << b))
            {
                uint8_t grid = FONT_BIT_TO_GRID[b];
                tm_ram[grid * 2 + offset] |= mask;
            }
        }
    }
}

void display_set_bottom(const char *str)
{
    // Clear bottom row digits (indices 4..7)
    for (uint8_t d = 0; d < 4; d++)
    {
        uint8_t offset = DIGIT_MAP[4 + d].byte_offset;
        uint8_t mask = DIGIT_MAP[4 + d].bit_mask;
        for (uint8_t g = 0; g < 8; g++)
        {
            tm_ram[g * 2 + offset] &= ~mask;
        }
    }

    if (!str)
        return;

    size_t len = strlen(str);
    for (uint8_t d = 0; d < 4 && d < len; d++)
    {
        uint8_t pat = ascii_to_7seg(str[d]);
        uint8_t offset = DIGIT_MAP[4 + d].byte_offset;
        uint8_t mask = DIGIT_MAP[4 + d].bit_mask;
        for (uint8_t b = 0; b < 8; b++)
        {
            if (pat & (1 << b))
            {
                uint8_t grid = FONT_BIT_TO_GRID[b];
                tm_ram[grid * 2 + offset] |= mask;
            }
        }
    }
}

void display_set_bar(uint8_t bars)
{
    if (bars > 10)
        bars = 10;

    // Clear all SEG_B bar bits (0x20 on all grids)
    for (uint8_t g = 0; g < 8; g++)
    {
        tm_ram[g * 2] &= ~0x20;
    }

    // Clear only the two SEG_L bar bits: L-f (GRID6, index 5) and L-g (GRID8, index 7)
    // Keep indicator bits L-a..L-e (grids 0, 1, 2, 3, 4) intact
    tm_ram[5 * 2] &= ~0x10; // L-f
    tm_ram[7 * 2] &= ~0x10; // L-g

    // Set active bars from bottom to top
    for (uint8_t lvl = 0; lvl < bars; lvl++)
    {
        uint8_t grid = BAR_MAP[lvl].grid_index;
        uint8_t mask = BAR_MAP[lvl].seg_mask;
        tm_ram[grid * 2] |= mask;
    }
}

void display_set_indicators(uint8_t mask)
{
    // Clear only indicator bits L-a..L-e on SEG_L (grids 4, 3, 1, 0, 2)
    // Keep bar graph bits L-f (grid 5) and L-g (grid 7) intact
    for (uint8_t i = 0; i < 5; i++)
    {
        uint8_t grid = INDICATOR_L_GRID[i];
        tm_ram[grid * 2] &= ~0x10;
    }

    // Set requested indicator blocks (L-a..L-e)
    for (uint8_t i = 0; i < 5; i++)
    {
        if (mask & (1 << i))
        {
            uint8_t grid = INDICATOR_L_GRID[i];
            tm_ram[grid * 2] |= 0x10;
        }
    }
}

void display_test_pattern()
{
    display_set_top("8888");
    display_set_bottom("8888");
    display_set_bar(10);
    display_set_indicators(0x1F); // All 5 indicator blocks on
    display_refresh();
}

const uint8_t *display_get_ram()
{
    return tm_ram;
}

void display_init()
{
    display_enabled = true;
    current_brightness = 7;
    tm_module.begin(true, 7);

    // Power-on self-test: illuminate all digits, bar graph, and indicators for 1000 ms
    display_test_pattern();
    delay(1000);

    // Immediately transition to initial standby display ("no L" on channel 78)
    display_update(TS_STAT_OFF, 0, 0, false, CHANNEL, 0, false);
}

void display_show_radio_error()
{
    display_enabled = true;
    display_set_top("Err ");
    display_set_bottom("nRF ");
    display_set_bar(0);
    display_set_indicators(DISPLAY_IND_RED);
    display_refresh();
}

void display_update(ts_status status, uint8_t active_unit, uint16_t buttons, bool link_healthy, uint8_t channel, uint8_t lqi, bool radio_error)
{
    char buf[8];

    // Prioritize hardware nRF24 error indication
    if (radio_error)
    {
        display_show_radio_error();
        return;
    }

    if (!link_healthy)
    {
        // Link lost or establishing connection
        display_enabled = true;
        display_set_top("no L");
        snprintf(buf, sizeof(buf), "C %02d", channel);
        display_set_bottom(buf);
        display_set_bar(0);
        display_set_indicators(DISPLAY_IND_RED);
    }
    else if (buttons & BUTTON_ESTOP_ACTIVE)
    {
        // Emergency stop active
        display_enabled = true;
        display_set_top("E-St");
        display_set_bottom("OP  ");
        display_set_bar(0);
        display_set_indicators(DISPLAY_IND_RED);
    }
    else if (status == TS_STAT_BATTERYLOW)
    {
        display_enabled = true;
        snprintf(buf, sizeof(buf), "U%02d ", active_unit);
        display_set_top(buf);
        display_set_bottom("bAtL");

        // Display LQI on bar graph
        uint8_t bars = (uint8_t)(((uint16_t)lqi * 10 + 50) / 100);
        if (bars > 10)
            bars = 10;
        display_set_bar(bars);
        display_set_indicators(DISPLAY_IND_RED | DISPLAY_IND_GREEN);
    }
    else if (status == TS_STAT_FAULT)
    {
        display_enabled = true;
        display_set_top("FAUL");
        display_set_bottom("t   ");
        display_set_bar(0);
        display_set_indicators(DISPLAY_IND_RED);
    }
    else if (status == TS_STAT_OFF)
    {
        display_enabled = false;
        display_clear();
        return;
    }
    else
    {
        // Normal operating mode (TS_STAT_ON) with healthy link
        display_enabled = true;
        snprintf(buf, sizeof(buf), "U%02d ", active_unit);
        display_set_top(buf);

        if (buttons != 0)
        {
            // Show active button hex mask
            snprintf(buf, sizeof(buf), "%04X", buttons);
        }
        else
        {
            // Show current hopping channel
            snprintf(buf, sizeof(buf), "C%02d ", channel);
        }
        display_set_bottom(buf);

        // Compute LQI bars (0..10) from lqi (0..100)
        uint8_t bars = 0;
        if (lqi > 0)
        {
            bars = (uint8_t)(((uint16_t)lqi * 10 + 50) / 100);
            if (bars > 10)
                bars = 10;
            if (bars == 0 && lqi > 0)
                bars = 1;
        }
        display_set_bar(bars);
        display_set_indicators(DISPLAY_IND_GREEN);
    }

    display_refresh();
}

void display_show_packet_debug(const RadioRxDiagnostics &diag, uint8_t channel, uint8_t lqi, bool chip_connected)
{
    display_enabled = true;

    if (!chip_connected)
    {
        display_show_radio_error();
        return;
    }

    char top_buf[8];
    char btm_buf[8];

    // Cycle between Page 0 (RX Valid vs CRC/Reject Errs) and Page 1 (RF mode & Channel/Last Error) every 1500 ms
    static unsigned long last_page_switch_ms = 0;
    static uint8_t page = 0;
    unsigned long now = millis();
    if (now - last_page_switch_ms >= 1500UL)
    {
        last_page_switch_ms = now;
        page = (page + 1) % 2;
    }

    if (page == 0)
    {
        // Page 0: Received valid packets (r###) on top, CRC/rejection errors (c###) on bottom
        snprintf(top_buf, sizeof(top_buf), "r%03lu", (unsigned long)(diag.rx_valid_count % 1000UL));
        snprintf(btm_buf, sizeof(btm_buf), "c%03lu", (unsigned long)((diag.rx_crc_err_count + diag.rx_len_err_count + diag.rx_reject_count) % 1000UL));
    }
    else
    {
        // Page 1: Data rate on top, Channel or Error Code on bottom
        if (diag.datarate_mode == 2 || diag.datarate_1mbps == 2)
        {
            snprintf(top_buf, sizeof(top_buf), "d-2M");
        }
        else if (diag.datarate_mode == 1 || diag.datarate_1mbps == 1)
        {
            snprintf(top_buf, sizeof(top_buf), "d-1M");
        }
        else
        {
            snprintf(top_buf, sizeof(top_buf), "d250");
        }

        if (diag.last_error_code != 0)
        {
            snprintf(btm_buf, sizeof(btm_buf), "E-%02d", (int)diag.last_error_code);
        }
        else
        {
            snprintf(btm_buf, sizeof(btm_buf), "C %02d", (int)channel);
        }
    }

    display_set_top(top_buf);
    display_set_bottom(btm_buf);

    // Bar graph: LQI (0..10 bars)
    uint8_t bars = 0;
    if (lqi > 0)
    {
        bars = (uint8_t)(((uint16_t)lqi * 10 + 50) / 100);
        if (bars > 10)
            bars = 10;
        if (bars == 0 && lqi > 0)
            bars = 1;
    }
    display_set_bar(bars);

    // Indicator LEDs:
    // Bit 0: Green  (0x01) -> Last RX Valid
    // Bit 1: Red    (0x02) -> Last RX Error / Invalid
    // Bit 2: Yel 1  (0x04) -> ACK Payload armed / loaded
    // Bit 3: Yel 2  (0x08) -> 1 Mbps mode active
    // Bit 4: Yel 3  (0x10) -> nRF chip connected
    uint8_t ind_mask = 0;
    if (diag.last_rx_ok)
        ind_mask |= DISPLAY_IND_GREEN;
    if (!diag.last_rx_ok && diag.rx_raw_count > 0)
        ind_mask |= DISPLAY_IND_RED;
    ind_mask |= DISPLAY_IND_YEL1; // Receiver constantly arms ACK payload
    if (diag.datarate_1mbps)
        ind_mask |= DISPLAY_IND_YEL2;
    if (chip_connected)
        ind_mask |= DISPLAY_IND_YEL3;

    display_set_indicators(ind_mask);
    display_refresh();
}
