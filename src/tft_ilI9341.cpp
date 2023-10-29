#include <esp32_smartdisplay.h>

#ifdef ILI9431

#define CMD_SLPIN 0x10       // Sleep in
#define CMD_SLPOUT 0x11      // Sleep out
#define CMD_GAMMASET 0x26    // Gamma set 26 01
#define CMD_DISPON 0x29      // Display On
#define CMD_CASET 0x2A       // Column Address Set
#define CMD_RASET 0x2B       // Row Address Set
#define CMD_RAMWR 0x2C       // Memory Write
#define CMD_MADCTL 0x36      // Memory Data Access Control
#define CMD_COLMOD 0x3A      // Interface Pixel Format
#define CMD_FRRATECTR 0xB1   // Frame rate control B1 00 1B
#define CMD_DISFNCTR 0xB6    // Function Function Control B6 0A 82 27 XX
#define CMD_ENTRYMODEST 0xB7 // Entry mode Set
#define CMD_PWRCTR1 0xC0     // Power Control 1
#define CMD_PWRCTR2 0xC1     // Power Control 2
#define CMD_VCOMVTRL1 0xC5   // VCOM Control 1
#define CMD_VCOMVTRL2 0xC7   // VCOM Control 2
#define CMD_PWRCTRB 0xCF     // Power Control B
#define CMD_PWRONSEQCTR 0xED // Power on Sequence Control
#define CMD_DRVTMCTR 0xE8    // Driver Timing control  E8 84 11 7A
#define CMD_PWRCTRA 0xCB     // Power Control A CB 39 2C 00 34 02
#define CMD_PGC 0E0          // Positive Gamma Control
#define CMD_NGC 0xE1         // Negative Gamma Control
#define CMD_DRVTMCTRB 0xEA   // Driver Timing control A  EA 66 00
#define CMD_EN3G 0xF2        // Enable 3 Gamma
#define CMD_PUMPRTCTR 0xF7   // Pump ratio control F7 10

#define MADCTL_MY 0x80  // Row Address Order - 0=Increment (Top to Bottom), 1=Decrement (Bottom to Top)
#define MADCTL_MX 0x40  // Column Address Order - 0=Increment (Left to Right), 1=Decrement (Right to Left)
#define MADCTL_MV 0x20  // Row/Column exchange - 0=Normal, 1=Row/Column exchanged
#define MADCTL_ML 0x10  // Vertical Refresh Order
#define MADCTL_BGR 0x08 // RGB/BGR Order - BGR
#define MADCTL_MH 0x10  // Horizontal Refresh Order
#define MADCTL_RGB 0x00 // RGB/BGR Order - RGB

#define COLMOD_RGB_16BIT 0x50
#define COLMOD_CTRL_16BIT 0x05
#define COLMOD_RGB656 (COLMOD_RGB_16BIT | COLMOD_CTRL_16BIT)

#ifdef TFT_PANEL_ORDER_RGB
#define MADCTL_PANEL_ORDER MADCTL_RGB
#else
#ifdef TFT_PANEL_ORDER_BGR
#define MADCTL_PANEL_ORDER MADCTL_BGR
#else
#error TFT_PANEL_ORDER not defined!
#endif
#endif

void ili9341_send_command(const uint8_t command, const uint8_t data[] = nullptr, const ushort length = 0)
{
  digitalWrite(ILI9341_PIN_DC, LOW); // Command mode => command
  spi_ili9431.beginTransaction(SPISettings(ILI9341_SPI_FREQ, MSBFIRST, SPI_MODE0));
  digitalWrite(ILI9341_PIN_CS, LOW); // Chip select => enable
  spi_ili9431.write(command);
  if (length > 0)
  {
    digitalWrite(ILI9341_PIN_DC, HIGH); // Command mode => data
    spi_ili9431.writeBytes(data, length);
  }
  digitalWrite(ILI9341_PIN_CS, HIGH); // Chip select => disable
  spi_ili9431.endTransaction();
}

void ili9341_send_pixels(const uint8_t command, const lv_color_t data[], const ushort length)
{
  digitalWrite(ILI9341_PIN_DC, LOW); // Command mode => command
  spi_ili9431.beginTransaction(SPISettings(ILI9341_SPI_FREQ, MSBFIRST, SPI_MODE0));
  digitalWrite(ILI9341_PIN_CS, LOW); // Chip select => enable
  spi_ili9431.write(command);
  if (length > 0)
  {
    digitalWrite(ILI9341_PIN_DC, HIGH); // Command mode => data
    spi_ili9431.writePixels(data, sizeof(lv_color_t) * length);
  }
  digitalWrite(ILI9341_PIN_CS, HIGH); // Chip select => disable
  spi_ili9431.endTransaction();
}

void ili9341_send_init_commands()
{
  static const uint8_t pwrctrb[] = {0x00, 0x83, 0X30};                     // VCIx2 + VCIx6 + VCIx3, Discharge Path Enable
  ili9341_send_command(CMD_PWRCTRB, pwrctrb, sizeof(pwrctrb));             // Power Control B
  static const uint8_t pwronseqctr[] = {0x64, 0x03, 0X12, 0X81};           // CP1 Soft start control disable, 3rd frame enable
  ili9341_send_command(CMD_PWRONSEQCTR, pwronseqctr, sizeof(pwronseqctr)); // Power on Sequence Control
  static const uint8_t drvtmrctr[] = {0x85, 0x01, 0x79};                   //
  ili9341_send_command(CMD_DRVTMCTR, drvtmrctr, sizeof(drvtmrctr));        // Driver Timing Control
  static const uint8_t pwrctra[] = {0x39, 0x2C, 0x00, 0x34, 0x02};         //
  ili9341_send_command(CMD_PWRCTRA, pwrctra, sizeof(pwrctra));             // Power Control A
  static const uint8_t pumprtctrl[] = {0x20};                              //
  ili9341_send_command(CMD_PUMPRTCTR, pumprtctrl, sizeof(pumprtctrl));     // Pump ratio control
  static const uint8_t drvtmrctrb[] = {0x00, 0x00};                        //
  ili9341_send_command(CMD_DRVTMCTRB, drvtmrctrb, sizeof(drvtmrctrb));     // Driver Timing Control B
  static const uint8_t pwrctr1[] = {0x26};                                 //
  ili9341_send_command(CMD_PWRCTR1, pwrctr1, sizeof(pwrctr1));             // Power Control 1
  static const uint8_t pwrctr2[] = {0x11};                                 //
  ili9341_send_command(CMD_PWRCTR2, pwrctr2, sizeof(pwrctr2));             // Power Control 2
  static const uint8_t vcomctr1[] = {0x35, 0x3E};                          //
  ili9341_send_command(CMD_VCOMVTRL1, vcomctr1, sizeof(vcomctr1));         // VCOM Control 1
  static const uint8_t vcomctr2[] = {0xBE};                                //
  ili9341_send_command(CMD_VCOMVTRL2, vcomctr2, sizeof(vcomctr2));         // VCOM Control 2

#ifdef TFT_ORIENTATION_PORTRAIT
// Portrait 0 Degrees
#ifndef TFT_FLIPPEDMIRRORED
  static const uint8_t madctl[] = {MADCTL_MY | MADCTL_PANEL_ORDER};
#else
  static const uint8_t madctl[] = {MADCTL_MY | MADCTL_MV | MADCTL_PANEL_ORDER}; // Flipped/Mirrored anomaly
#endif
#else
#ifdef TFT_ORIENTATION_LANDSCAPE
// Landscape 90 Degrees
#ifndef TFT_FLIPPEDMIRRORED
  static const uint8_t madctl[] = {MADCTL_MV | MADCTL_PANEL_ORDER};
#else
  static const uint8_t madctl[] = {MADCTL_ML | MADCTL_PANEL_ORDER}; // Flipped/Mirrored anomaly
#endif
#else
#ifdef TFT_ORIENTATION_PORTRAIT_INV
// Portrait inverted 180 Degrees
#ifndef TFT_FLIPPEDMIRRORED
  static const uint8_t madctl[] = {MADCTL_MX | MADCTL_PANEL_ORDER};
#else
  static const uint8_t madctl[] = {MADCTL_MX | MADCTL_MV | MADCTL_PANEL_ORDER}; // Flipped/Mirrored anomaly
#endif
#else
#ifdef TFT_ORIENTATION_LANDSCAPE_INV
// Landscape inverted 270 Degrees
#ifndef TFT_FLIPPEDMIRRORED
  static const uint8_t madctl[] = {MADCTL_MY | MADCTL_MX | MADCTL_MV | MADCTL_PANEL_ORDER};
#else
  static const uint8_t madctl[] = {MADCTL_MY | MADCTL_MX | MADCTL_MH | MADCTL_PANEL_ORDER}; // Flipped/Mirrored anomaly
#endif
#else
#error TFT_ORIENTATION not defined!
#endif
#endif
#endif
#endif
  ili9341_send_command(CMD_MADCTL, madctl, sizeof(madctl));

  static const uint8_t colmod[] = {COLMOD_RGB656};          // 16 bits R5G6B5
  ili9341_send_command(CMD_COLMOD, colmod, sizeof(colmod)); // Set color mode
  static const uint8_t frratectrl[] = {0x00, 0x1B};
  ili9341_send_command(CMD_FRRATECTR, frratectrl, sizeof(frratectrl)); // Frame rate control
  static const uint8_t en3g[] = {0x08};
  ili9341_send_command(CMD_EN3G, en3g, sizeof(en3g)); // Enable 3 Gamma
  static const uint8_t gammaset[] = {0x01};
  ili9341_send_command(CMD_GAMMASET, gammaset, sizeof(gammaset)); // Gamma Set
  static const uint8_t pgc[] = {0x1F, 0x1A, 0x18, 0x0A, 0x0F, 0x06, 0x45, 0X87, 0x32, 0x0A, 0x07, 0x02, 0x07, 0x05, 0x00};
  ili9341_send_command(CMD_PGC, pgc, sizeof(pgc));
  static const uint8_t ngc[] = {0x00, 0x25, 0x27, 0x05, 0x10, 0x09, 0x3A, 0x78, 0x4D, 0x05, 0x18, 0x0D, 0x38, 0x3A, 0x1F};
  ili9341_send_command(CMD_NGC, ngc, sizeof(ngc));
  static const uint8_t entrymodest[] = {0x07};                             //
  ili9341_send_command(CMD_ENTRYMODEST, entrymodest, sizeof(entrymodest)); // Entry mode Set
  static const uint8_t disfnctr[] = {0x0A, 0x82, 0x27, 0x00};              //
  ili9341_send_command(CMD_DISFNCTR, disfnctr, sizeof(disfnctr));
  ili9341_send_command(CMD_SLPOUT); // Out of sleep mode
  ili9341_send_command(CMD_DISPON); // Main screen turn on
}

void lvgl_tft_init()
{
  pinMode(ILI9341_PIN_DC, OUTPUT); // Data or Command
  pinMode(ILI9341_PIN_CS, OUTPUT); // Chip Select
  digitalWrite(ILI9341_PIN_CS, HIGH);

  pinMode(ILI9341_PIN_BL, OUTPUT); // Backlight
  ledcSetup(PWM_CHANNEL_BL, PWM_FREQ_BL, PWM_BITS_BL);
  ledcAttachPin(ILI9341_PIN_BL, PWM_CHANNEL_BL);

  ili9341_send_init_commands();

  smartdisplay_tft_set_backlight(PWM_MAX_BL); // Backlight on
}

void lvgl_tft_flush(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_map)
{
  // Column addresses
  const uint8_t caset[] = {
      static_cast<uint8_t>(area->x1 >> 8),
      static_cast<uint8_t>(area->x1),
      static_cast<uint8_t>(area->x2 >> 8),
      static_cast<uint8_t>(area->x2)};
  ili9341_send_command(CMD_CASET, caset, sizeof(caset));
  // Page addresses
  const uint8_t raset[] = {
      static_cast<uint8_t>(area->y1 >> 8),
      static_cast<uint8_t>(area->y1),
      static_cast<uint8_t>(area->y2 >> 8),
      static_cast<uint8_t>(area->y2)};
  ili9341_send_command(CMD_RASET, raset, sizeof(raset));
  // Memory write
  const auto size = lv_area_get_width(area) * lv_area_get_height(area);
  ili9341_send_pixels(CMD_RAMWR, color_map, size);
  lv_disp_flush_ready(drv);
}

void smartdisplay_tft_set_backlight(uint16_t duty)
{
  ledcWrite(PWM_CHANNEL_BL, duty);
}

void smartdisplay_tft_sleep()
{
  static const uint8_t slpin[] = {0x08};
  ili9341_send_command(CMD_SLPIN, slpin, sizeof(slpin));
}

void smartdisplay_tft_wake()
{
  static const uint8_t splout[] = {0x08};
  ili9341_send_command(CMD_SLPOUT, splout, sizeof(splout));
}

#endif