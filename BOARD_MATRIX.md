# Hardware Compatibility & Board Compatibility Matrix

This document provides a comprehensive mapping of all supported Sunton Cheap Yellow Display (CYD) and compatible smart display boards, their associated display drivers, touch controllers, interface types, and specific build configurations required when using `esp32-smartdisplay`.

---

## Overview Table

| Board ID | Display Resolution | Screen Size | Display Controller & Interface | Touch Controller & Interface | MCU / SoC | PSRAM / Flash | Key Features / Notes |
| :--- | :---: | :---: | :--- | :--- | :--- | :---: | :--- |
| **esp32-1732S019C** | $170\times320$ | 1.9" | ST7789 (SPI) | GT911 (I2C) | ESP32-S3 | 8MB / 16MB | Capacitive Touch, USB-C |
| **esp32-1732S019N** | $170\times320$ | 1.9" | ST7789 (SPI) | *None* (No Touch) | ESP32-S3 | 8MB / 16MB | No Touch variant |
| **esp32-2424S012C** | $240\times240$ | 1.2" | GC9A01A (SPI) | CST816S (I2C) | ESP32-C3 | None / 4MB | Round Display, Capacitive Touch |
| **esp32-2424S012N** | $240\times240$ | 1.2" | GC9A01A (SPI) | *None* (No Touch) | ESP32-C3 | None / 4MB | Round Display, No Touch |
| **esp32-2432S022C** | $240\times320$ | 2.2" | ST7789 (SPI) | CST816S (I2C) | ESP32 | None / 4MB | Audio (FM8002A), Capacitive Touch |
| **esp32-2432S022N** | $240\times320$ | 2.2" | ST7789 (SPI) | *None* (No Touch) | ESP32 | None / 4MB | Audio (FM8002A), No Touch |
| **esp32-2432S024C** | $240\times320$ | 2.4" | ILI9341 (SPI) | CST820 / CST816S (SPI/I2C) | ESP32 | None / 4MB | Micro-USB, Audio, RGB LEDs, Capacitive Touch |
| **esp32-2432S024N** | $240\times320$ | 2.4" | ILI9341 (SPI) | *None* (No Touch) | ESP32 | None / 4MB | Micro-USB, Audio, RGB LEDs, No Touch |
| **esp32-2432S024R** | $240\times320$ | 2.4" | ILI9341 (SPI) | XPT2046 (SPI) | ESP32 | None / 4MB | Micro-USB, Audio, RGB LEDs, Resistive Touch |
| **esp32-2432S028R** | $240\times320$ | 2.8" | ILI9341 (SPI) | XPT2046 (SPI) | ESP32 | None / 4MB | Classic CYD, Resistive Touch |
| **esp32-2432S028Rv2**| $240\times320$ | 2.8" | ILI9341 (SPI) | XPT2046 (SPI) | ESP32 | None / 4MB | USB-C Revision 2 |
| **esp32-2432S028Rv3**| $240\times320$ | 2.8" | ST7789 (SPI) | XPT2046 (SPI) | ESP32 | None / 4MB | USB-C / Micro-USB Revision 3 (ST7789 display) |
| **esp32-2432S032C** | $240\times320$ | 3.2" | ST7789 (SPI) | GT911 (I2C) | ESP32 | None / 4MB | Capacitive Touch |
| **esp32-2432S032N** | $240\times320$ | 3.2" | ST7789 (SPI) | *None* (No Touch) | ESP32 | None / 4MB | No Touch |
| **esp32-2432S032R** | $240\times320$ | 3.2" | ST7789 (SPI) | XPT2046 (SPI) | ESP32 | None / 4MB | Resistive Touch |
| **JC2432W328C** | $240\times320$ | 2.8" | ST7789 (SPI) | CST816S (I2C) | ESP32 | None / 4MB | Capacitive Touch variant |
| **esp32-3248S035C** | $320\times480$ | 3.5" | ST7796 (SPI) | GT911 (I2C) | ESP32 | None / 4MB | 3.5" Capacitive Touch |
| **esp32-3248S035R** | $320\times480$ | 3.5" | ST7796 (SPI) | XPT2046 (SPI) | ESP32 | None / 4MB | 3.5" Resistive Touch |
| **esp32-4827S043C** | $480\times272$ | 4.3" | ST7262 (Direct 16-bit RGB) | GT911 (I2C) | ESP32-S3 | 8MB / 16MB | RGB Parallel Display, Capacitive Touch |
| **esp32-4827S043N** | $480\times272$ | 4.3" | ST7262 (Direct 16-bit RGB) | *None* (No Touch) | ESP32-S3 | 8MB / 16MB | RGB Parallel Display, No Touch |
| **esp32-4827S043R** | $480\times272$ | 4.3" | ST7262 (Direct 16-bit RGB) | XPT2046 (SPI) | ESP32-S3 | 8MB / 16MB | RGB Parallel Display, Resistive Touch |
| **esp32-8048S043C** | $800\times480$ | 4.3" | ST7262 (Direct 16-bit RGB) | GT911 (I2C) | ESP32-S3 | 8MB / 16MB | High-res 4.3", Capacitive Touch |
| **esp32-8048S043N** | $800\times480$ | 4.3" | ST7262 (Direct 16-bit RGB) | *None* (No Touch) | ESP32-S3 | 8MB / 16MB | High-res 4.3", No Touch |
| **esp32-8048S043R** | $800\times480$ | 4.3" | ST7262 (Direct 16-bit RGB) | XPT2046 (SPI) | ESP32-S3 | 8MB / 16MB | High-res 4.3", Resistive Touch |
| **esp32-4848S040CIY1**| $480\times480$ | 4.0" | ST7701 (16-bit + 3-wire SPI) | GT911 (I2C) | ESP32-S3 | 8MB / 16MB | Round/Square RGB panel, Relay 1 |
| **esp32-4848S040CIY3**| $480\times480$ | 4.0" | ST7701 (16-bit + 3-wire SPI) | GT911 (I2C) | ESP32-S3 | 8MB / 16MB | Round/Square RGB panel, Relays 1-3 |
| **esp32-8048S050C** | $800\times480$ | 5.0" | ST7262 (Direct 16-bit RGB) | GT911 (I2C) | ESP32-S3 | 8MB / 16MB | 5.0" Capacitive Touch |
| **esp32-8048S050N** | $800\times480$ | 5.0" | ST7262 (Direct 16-bit RGB) | *None* (No Touch) | ESP32-S3 | 8MB / 16MB | 5.0" No Touch |
| **esp32-8048S050R** | $800\times480$ | 5.0" | ST7262 (Direct 16-bit RGB) | XPT2046 (SPI) | ESP32-S3 | 8MB / 16MB | 5.0" Resistive Touch |
| **esp32-8048S070C** | $800\times480$ | 7.0" | ST7262 (Direct 16-bit RGB) | GT911 (I2C) | ESP32-S3 | 8MB / 16MB | 7.0" Capacitive Touch |
| **esp32-8048S070N** | $800\times480$ | 7.0" | ST7262 (Direct 16-bit RGB) | *None* (No Touch) | ESP32-S3 | 8MB / 16MB | 7.0" No Touch |
| **esp32-8048S070R** | $800\times480$ | 7.0" | ST7262 (Direct 16-bit RGB) | XPT2046 (SPI) | ESP32-S3 | 8MB / 16MB | 7.0" Resistive Touch |
| **esp32-8048S550C** | $800\times480$ | 5.5" | ST7262 (Direct 16-bit RGB) | GT911 (I2C) | ESP32-S3 | 8MB / 16MB | 5.5" Capacitive Touch |
| **JC3248W535N** | $320\times480$ | 3.5" | ST7796 (SPI) | *None* (No Touch) | ESP32-S3 | 8MB / 16MB | ESP32-S3 3.5" Display |
| **JC4827W543C** | $480\times272$ | 4.3" | ST7262 (Direct 16-bit RGB) | GT911 (I2C) | ESP32-S3 | 8MB / 16MB | JC Series 4.3" |
| **JC8048W550C** | $800\times480$ | 5.0" | ST7262 (Direct 16-bit RGB) | GT911 (I2C) | ESP32-S3 | 8MB / 16MB | JC Series 5.0" |
| **esp32-s3touchlcd2p8**| $240\times320$ | 2.8" | ILI9341 / ST7789 | Capacitive / Resistive | ESP32-S3 | 8MB / 16MB | Waveshare / Generic S3 2.8" |
| **esp32-s3touchlcd4p3**| $480\times272$ | 4.3" | ST7262 (RGB) | GT911 (I2C) | ESP32-S3 | 8MB / 16MB | Waveshare S3 4.3" |
| **esp32-s3touchlcd7** | $800\times480$ | 7.0" | ST7262 (RGB) | GT911 (I2C) | ESP32-S3 | 8MB / 16MB | Waveshare S3 7.0" |

---

## Driver Architecture Mapping

The library automatically configures the appropriate underlying `esp_lcd` panel driver and touch driver based on the selected PlatformIO environment board definition:

1. **SPI Displays (`ST7789`, `ILI9341`, `ST7796`, `GC9A01`)**:
   - Initialized via high-speed SPI with dedicated `esp_lcd_panel_io` handles.
   - Touch interfaces use either SPI (`XPT2046`) or I2C (`CST816S`, `GT911`).

2. **Parallel RGB Displays (`ST7262`, `ST7701`)**:
   - Uses ESP32-S3 RGB peripheral interface (`esp_lcd_dpi_panel`) for high-throughput frame updates directly from PSRAM framebuffers.
   - Requires external PSRAM (8MB) enabled in the board configuration.

---

## Quick Build Selection Example

To build and flash for a specific board (e.g., the classic 2.8" resistive CYD):

```ini
[env:esp32-2432S028R]
board = esp32-2432S028R
```
