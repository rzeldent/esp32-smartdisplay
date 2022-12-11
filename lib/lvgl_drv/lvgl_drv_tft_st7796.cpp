#ifdef LVGL_TFT_ST7796

#include <Arduino.h>
#include <SPI.h>

#include "lvgl_drv_tft.h"

// Command table 1
constexpr uint8_t CMD_NOP = 0x00;       // No Operation
constexpr uint8_t CMD_SWRESET = 0x01;   // Software Reset
constexpr uint8_t CMD_RDDID = 0x04;     // Read Display ID
constexpr uint8_t CMD_RDNUMED = 0x05;   // Read Number of the Errors on DSI
constexpr uint8_t CMD_RDDST = 0x09;     // Read Display Status
constexpr uint8_t CMD_RDDPM = 0x0A;     // Read Display Power Mode
constexpr uint8_t CMD_RDDMADCTL = 0x0B; // Read Display MADCTL
constexpr uint8_t CMD_RDDCOLMOD = 0x0C; // Read Display Pixel Format
constexpr uint8_t CMD_RDDIM = 0x0D;     // Read Display Image Mode
constexpr uint8_t CMD_RDDSM = 0x0E;     // Read Display Signal Mode
constexpr uint8_t CMD_RDDSDR = 0x0F;    // Read Display Self-Diagnostic Result
constexpr uint8_t CMD_SLPIN = 0x10;     // Sleep in
constexpr uint8_t CMD_SLPOUT = 0x11;    // Sleep out
constexpr uint8_t CMD_PTLON = 0x12;     // Partial Display Mode On
constexpr uint8_t CMD_NORON = 0x13;     // Normal Display Mode On
constexpr uint8_t CMD_INVOFF = 0x20;    // Display Inversion Off
constexpr uint8_t CMD_INVON = 0x21;     // Display Inversion On
constexpr uint8_t CMD_DISPOFF = 0x28;   // Display Off
constexpr uint8_t CMD_DISPON = 0x29;    // Display On
constexpr uint8_t CMD_CASET = 0x2A;     // Column Address Set
constexpr uint8_t CMD_RASET = 0x2B;     // Row Address Set
constexpr uint8_t CMD_RAMWR = 0x2C;     // Memory Write
constexpr uint8_t CMD_RAMRD = 0x2E;     // Memory Read
constexpr uint8_t CMD_PTLAR = 0x30;     // Partial Area
constexpr uint8_t CMD_VSCRDEF = 0x33;   // Vertical Scrolling Definition
constexpr uint8_t CMD_TEOFF = 0x34;     // Tearing Effect Line Off
constexpr uint8_t CMD_TEON = 0x35;      // Tearing Effect Line On
constexpr uint8_t CMD_MADCTL = 0x36;    // Memory Data Access Control
constexpr uint8_t CMD_VSCSAD = 0x37;    // Vertical Scroll Start Address of RAM
constexpr uint8_t CMD_IDMOFF = 0x38;    // Idle Mode Off
constexpr uint8_t CMD_IDMON = 0x39;     // Idle Mode On
constexpr uint8_t CMD_COLMOD = 0x3A;    // Interface Pixel Format
constexpr uint8_t CMD_WRMEMC = 0x3C;    // Write Memory Continue
constexpr uint8_t CMD_RDMEMC = 0x3E;    // Read Memory Continue
constexpr uint8_t CMD_STE = 0x44;       // Set Tear Scanline
constexpr uint8_t CMD_GSCAN = 0x45;     // Get Scanline
constexpr uint8_t CMD_WRDISBV = 0x51;   // Write Display Brightness Value
constexpr uint8_t CMD_RDDISBV = 0x52;   // Read Display Brightness Value
constexpr uint8_t CMD_WRCTRLD = 0x53;   // Write CTRL Display
constexpr uint8_t CMD_RDCTRLD = 0x54;   // Read CTRL Display
constexpr uint8_t CMD_WRCABC = 0x55;    // Write Content Adaptive Brightness Control
constexpr uint8_t CMD_RDCABC = 0x56;    // Read Content Adaptive Brightness Control
constexpr uint8_t CMD_WRCABCMB = 0x5E;  // Write CABC Minimum Brightness
constexpr uint8_t CMD_RDCABCMB = 0x5F;  // Read CABC Minimum Brightness
constexpr uint8_t CMD_RDFCS = 0xAA;     // Read First Checksum
constexpr uint8_t CMD_RDCFCS = 0xAF;    // Read Continue Checksum
constexpr uint8_t CMD_RDID1 = 0xDA;     // Read ID1
constexpr uint8_t CMD_RDID2 = 0xDB;     // Read ID2
constexpr uint8_t CMD_RDID13 = 0xDC;    // Read ID3

// Command table 2
constexpr uint8_t CMD_IFMODE = 0xB0;    // Interface Mode Control
constexpr uint8_t CMD_FRMCTR1 = 0xB1;   // Frame Rate Control (In Normal Mode/Full Colors)
constexpr uint8_t CMD_FRMCTR2 = 0xB2;   // Frame Rate Control2 (In Idle Mode/8 colors)
constexpr uint8_t CMD_FRMCTR3 = 0xB3;   // Frame Rate Control3 (In Partial Mode/Full Colors)
constexpr uint8_t CMD_DIC = 0xB4;       // Display Inversion Control
constexpr uint8_t CMD_BPC = 0xB5;       // Blanking Porch Control
constexpr uint8_t CMD_DFC = 0xB6;       // Display Function Control
constexpr uint8_t CMD_EM = 0xB7;        // Entry Mode Set
constexpr uint8_t CMD_PWCTR1 = 0xC0;    // Power Control 1
constexpr uint8_t CMD_PWCTR2 = 0xC1;    // Power Control 2
constexpr uint8_t CMD_PWCTR3 = 0xC2;    // Power Control 3
constexpr uint8_t CMD_VCMPCTL = 0xC5;   // VCOM Control
constexpr uint8_t CMD_VCMOFFSET = 0xC6; // VCOM Offset Register
constexpr uint8_t CMD_NVMADW = 0xD0;    // NVM Address/Data Write
constexpr uint8_t CMD_NVMBPROG = 0xD1;  // NVM Byte Program
constexpr uint8_t CMD_NVMSTRD = 0xD2;   // NVM Status Read
constexpr uint8_t CMD_RDID4 = 0xD3;     // Read ID4
constexpr uint8_t CMD_PGC = 0E0;        // Positive Gamma Control
constexpr uint8_t CMD_NGC = 0xE1;       // Negative Gamma Control
constexpr uint8_t CMD_DGC1 = 0xE2;      // Digital Gamma Control 1
constexpr uint8_t CMD_DGC2 = 0xE3;      // Digital Gamma Control 2
constexpr uint8_t CMD_DOCA = 0xE8;      // Display Output Ctrl Adjust
constexpr uint8_t CMD_CSCON = 0xF0;     // Command Set Control
constexpr uint8_t CMD_SPIRC = 0xFB;     // SPI Read Control

// MADCTL flags
constexpr uint8_t MADCTL_MY = 0x80;  // Row Address Order - 0=Increment (Top to Bottom), 1=Decrement (Bottom to Top)
constexpr uint8_t MADCTL_MX = 0x40;  // Column Address Order - 0=Increment (Left to Right), 1=Decrement (Right to Left)
constexpr uint8_t MADCTL_MV = 0x20;  // Row/Column exchange - 0=Normal, 1=Row/Column exchanged
constexpr uint8_t MADCTL_ML = 0x10;  // Scan Address Order - 0=Decrement, 1=Increment
constexpr uint8_t MADCTL_BGR = 0x08; // RGB/BGR Order - BGR
constexpr uint8_t MADCTL_MH = 0x04;  // Horizontal Order - 0=Decrement (Left to Right), 1=Increment (Right to Left)
constexpr uint8_t MADCTL_HF = 0x02;
constexpr uint8_t MADCTL_VF = 0x01;
constexpr uint8_t MADCTL_RGB = 0x00; // RGB/BGR Order - RGB

// COLMOD modes
constexpr uint8_t COLMOD_RGB_12BIT = 0x30;
constexpr uint8_t COLMOD_RGB_16BIT = 0x50;
constexpr uint8_t COLMOD_RGB_18BIT = 0x60;
constexpr uint8_t COLMOD_CTRL_12BIT = 0x03;
constexpr uint8_t COLMOD_CTRL_16BIT = 0x05;
constexpr uint8_t COLMOD_CTRL_18BIT = 0x06;
constexpr uint8_t COLMOD_CTRL_24BIT = 0x07;
constexpr uint8_t COLMOD_RGB444 = COLMOD_RGB_12BIT | COLMOD_CTRL_12BIT;
constexpr uint8_t COLMOD_RGB656 = COLMOD_RGB_16BIT | COLMOD_CTRL_16BIT;
constexpr uint8_t COLMOD_RGB = COLMOD_RGB_18BIT | COLMOD_CTRL_18BIT;

// OPTIONS
constexpr uint8_t OPTIONS_WRAP_V = 0x01;
constexpr uint8_t OPTIONS_WRAP_H = 0x02;
constexpr uint8_t OPTIONS_WRAP = 0x03;

static constexpr uint8_t madctl_table[] =
    {
        0,
        MADCTL_MV | MADCTL_MX | MADCTL_MH,
        MADCTL_MX | MADCTL_MH | MADCTL_MY | MADCTL_ML,
        MADCTL_MV | MADCTL_MY | MADCTL_ML,
        MADCTL_MY | MADCTL_ML,
        MADCTL_MV,
        MADCTL_MX | MADCTL_MH,
        MADCTL_MV | MADCTL_MX | MADCTL_MY | MADCTL_MH | MADCTL_ML,
};

// Color definitions RGB565
constexpr uint16_t RGB565_BLACK = 0x0000;
constexpr uint16_t RGB565_BLUE = 0x001F;
constexpr uint16_t RGB565_RED = 0xF800;
constexpr uint16_t RGB565_GREEN = 0x07E0;
constexpr uint16_t RGB565_CYAN = 0x07FF;
constexpr uint16_t RGB565_MAGENTA = 0xF81F;
constexpr uint16_t RGB565_YELLOW = 0xFFE0;
constexpr uint16_t RGB565_WHITE = 0xFFFF;

static const SPISettings spi_settings(TFT_SPI_FREQ, MSBFIRST, SPI_MODE0);

void lvgl_tft_st7796_send_command(const uint8_t command, const uint8_t data[] = nullptr, const ushort length = 0)
{
    digitalWrite(TFT_PIN_DC, false); // Command mode => command
    SPI.beginTransaction(spi_settings);
    digitalWrite(TFT_PIN_CS, false); // Chip select => enable
    SPI.write(command);
    if (length > 0)
    {
        digitalWrite(TFT_PIN_DC, true); // Command mode => data
        SPI.writeBytes(data, length);
    }
    digitalWrite(TFT_PIN_CS, true); // Chip select => disable
    SPI.endTransaction();
}

void lvgl_tft_st7796_send_pixels(const uint8_t command, const lv_color_t data[], const ushort length)
{
    digitalWrite(TFT_PIN_DC, false); // Command mode => command
    SPI.beginTransaction(spi_settings);
    digitalWrite(TFT_PIN_CS, false); // Chip select => enable
    SPI.write(command);
    if (length > 0)
    {
        digitalWrite(TFT_PIN_DC, true); // Command mode => data
        SPI.writePixels(data, sizeof(lv_color_t) * length);
    }
    digitalWrite(TFT_PIN_CS, true); // Chip select => disable
    SPI.endTransaction();
}

void lvgl_tft_st7796_send_init_commands()
{
    lvgl_tft_st7796_send_command(CMD_SWRESET); // Software reset
    delay(100);

    static const uint8_t cscon1[] = {0xC3}; // Enable extension command 2 part I
    lvgl_tft_st7796_send_command(CMD_CSCON, cscon1, sizeof(cscon1));
    static const uint8_t cscon2[] = {0x96}; // Enable extension command 2 part II
    lvgl_tft_st7796_send_command(CMD_CSCON, cscon2, sizeof(cscon2));

    static const uint8_t colmod[] = {COLMOD_RGB656};                  // 16 bits R5G6B5
    lvgl_tft_st7796_send_command(CMD_COLMOD, colmod, sizeof(colmod)); // Set color mode

    /*
        static const uint8_t pwctr1[] = {0x26};                           // AVDD=6.20v, AVCL=-4.8v
        lvgl_tft_st7796_send_command(CMD_PWCTR1, pwctr1, sizeof(pwctr1)); // Power control 1, VGH=?
        static const uint8_t pwctr2[] = {0x11};                           // 4.4+(vcom+vcom offset)
        lvgl_tft_st7796_send_command(CMD_PWCTR2, pwctr2, sizeof(pwctr2)); // Power control 2
        static const uint8_t pwctr3[] = {0xA7};                           // Source driving current level=low, Gamma driving current level=High
        lvgl_tft_st7796_send_command(CMD_PWCTR3, pwctr3, sizeof(pwctr3)); // Power control 2

        static const uint8_t vcmpctrl[] = {0x35, 0x3E};                        // 0x3E=53=1.625v
        lvgl_tft_st7796_send_command(CMD_VCMPCTL, vcmpctrl, sizeof(vcmpctrl)); // VCOM control
    */
    //    static const uint8_t c7[] = {0xBE};
    //    lvgl_tft_st7796_send(0xC7, c7,sizeof(c7)),		 // VCOM control?
    //    sleep(10);
    static const uint8_t pgc[] = {0xF0, 0x09, 0x0B, 0x06, 0x04, 0x15, 0x2F, 0x54, 0x42, 0x3C, 0x17, 0x14, 0x18, 0x1B};
    lvgl_tft_st7796_send_command(CMD_PGC, pgc, sizeof(pgc));
    static const uint8_t ngc[] = {0xE0, 0x09, 0x0B, 0x06, 0x04, 0x03, 0x2B, 0x43, 0x42, 0x3B, 0x16, 0x14, 0x17, 0x1B};
    lvgl_tft_st7796_send_command(CMD_NGC, ngc, sizeof(ngc));

    static const uint8_t cscon3[] = {0x3C}; // Disable extension command 2 part I
    lvgl_tft_st7796_send_command(CMD_CSCON, cscon3, sizeof(cscon3));
    static const uint8_t cscon4[] = {0x69}; // Disable extension command 2 part II
    lvgl_tft_st7796_send_command(CMD_CSCON, cscon4, sizeof(cscon4));

    lvgl_tft_st7796_send_command(CMD_INVOFF); // Inversion off
    lvgl_tft_st7796_send_command(CMD_NORON);  // Normal display on
    lvgl_tft_st7796_send_command(CMD_SLPOUT); // Out of sleep mode
    lvgl_tft_st7796_send_command(CMD_DISPON); // Main screen turn on
    delay(500);
};

void lvgl_tft_init()
{
    SPI.begin(TFT_PIN_SCLK, TFT_PIN_MISO, TFT_PIN_MOSI);
    SPI.setFrequency(TFT_SPI_FREQ);
    pinMode(TFT_PIN_DC, OUTPUT); // Data or Command
    pinMode(TFT_PIN_CS, OUTPUT); // Chip Select
    pinMode(TFT_PIN_BL, OUTPUT); // Backlight
    lvgl_tft_st7796_send_init_commands();
    lvgl_tft_set_orientation(lvgl_tft_orientation::portrait);
    digitalWrite(TFT_PIN_BL, true);
}

void lvgl_tft_flush(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_map)
{
    // Column addresses
    const uint8_t caset[] = {area->x1 >> 8, area->x1, area->x2 >> 8, area->x2};
    lvgl_tft_st7796_send_command(CMD_CASET, caset, sizeof(caset));
    // Page addresses
    const uint8_t raset[] = {area->y1 >> 8, area->y1, area->y2 >> 8, area->y2};
    lvgl_tft_st7796_send_command(CMD_RASET, raset, sizeof(raset));
    // Memory write
    const auto size = lv_area_get_width(area) * lv_area_get_height(area);
    lvgl_tft_st7796_send_pixels(CMD_RAMWR, color_map, size);
    lv_disp_flush_ready(drv);
}

void lvgl_tft_set_orientation(enum lvgl_tft_orientation orientation)
{
    static const uint8_t data[] = {
        0 | MADCTL_MY, // Portrait
        MADCTL_MX | MADCTL_MY | MADCTL_MH,
        MADCTL_MY | MADCTL_MV | MADCTL_MH,
        MADCTL_MH,
        MADCTL_MX | MADCTL_MV | MADCTL_MH};
    // static const uint8_t data[] = {0x48, 0x88, 0x28, 0xE8};
    lvgl_tft_st7796_send_command(CMD_MADCTL, &data[orientation], 1);
}

void lvgl_tft_set_backlight(uint8_t percent)
{
    auto duty_cycle = (1023 * percent) / 100; // resolution set to 10bits, thus: 100% = 1023
    analogWrite(TFT_PIN_BL, duty_cycle);
}

void vgl_tft_sleep()
{
    static const uint8_t slpin[] = {0x08};
    lvgl_tft_st7796_send_command(CMD_SLPIN, slpin, sizeof(slpin));
}

void vgl_tft_wake()
{
    static const uint8_t splout[] = {0x08};
    lvgl_tft_st7796_send_command(CMD_SLPOUT, splout, sizeof(splout));
}

#endif