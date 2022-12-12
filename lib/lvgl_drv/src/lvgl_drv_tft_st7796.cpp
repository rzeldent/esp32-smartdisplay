#ifdef LVGL_TFT_ST7796

#include <Arduino.h>
#include <SPI.h>

#include <lvgl_drv_tft.h>
#include <lvgl_drv_tft_st7796.h>

const SPISettings spi_settings(TFT_SPI_FREQ, MSBFIRST, SPI_MODE0);

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

#ifdef TFT_ORIENTATION_PORTRAIT
    static const uint8_t madctl[] = {MADCTL_MY | MADCTL_RGB}; // Portrait 0 Degrees
#else
#ifdef TFT_ORIENTATION_LANDSCAPE
    static const uint8_t madctl[] = {MADCTL_MV | MADCTL_RGB}; // Landscape 90 Degrees
#else
#ifdef TFT_ORIENTATION_PORTRAIT_INV
    static const uint8_t madctl[] = {MADCTL_MX | MADCTL_RGB}; // Portrait inverted 180 Degrees
#else
#ifdef TFT_ORIENTATION_LANDSCAPE_INV
    static const uint8_t madctl[] = {MADCTL_MY | MADCTL_MX | MADCTL_MV | MADCTL_RGB}; // Landscape inverted 270 Degrees
#else
#error TFT_ORIENTATION not defined!
#endif
#endif
#endif
#endif
    lvgl_tft_st7796_send_command(CMD_MADCTL, madctl, sizeof(madctl));

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
    digitalWrite(TFT_PIN_BL, true);
}

void lvgl_tft_flush(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_map)
{
    // Column addresses
    const uint8_t caset[] = {
        static_cast<uint8_t>(area->x1 >> 8),
        static_cast<uint8_t>(area->x1),
        static_cast<uint8_t>(area->x2 >> 8),
        static_cast<uint8_t>(area->x2)};
    lvgl_tft_st7796_send_command(CMD_CASET, caset, sizeof(caset));
    // Page addresses
    const uint8_t raset[] = {
        static_cast<uint8_t>(area->y1 >> 8),
        static_cast<uint8_t>(area->y1),
        static_cast<uint8_t>(area->y2 >> 8),
        static_cast<uint8_t>(area->y2)};
    lvgl_tft_st7796_send_command(CMD_RASET, raset, sizeof(raset));
    // Memory write
    const auto size = lv_area_get_width(area) * lv_area_get_height(area);
    lvgl_tft_st7796_send_pixels(CMD_RAMWR, color_map, size);
    lv_disp_flush_ready(drv);
}

void lvgl_tft_set_backlight(uint8_t value)
{
    analogWrite(TFT_PIN_BL, value);
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