# ESP32-SmartDisplay

LVGL Drivers for Chinese Smart display boards ESP32-2432S035R, ESP32-3248S035R, ESP32-3248S035C

These boards are available on AliExpress for decent prices.
With the boards, there is a link supplied and there are a lot of examples present and this looks fine.
These examples for LVGL depend on external libraries ([TFT_eSPI](https://github.com/Bodmer/TFT_eSPI) or [LovyanGFX](https://github.com/lovyan03/LovyanGFX)).
However, when implementing the capacitive version, I found out that these libraries had their flaws using these boards.
Additionally, There was a log of unnecesary code included in these libraries.
There is also a library for LVGL present [LVGL_ESP32_drivers](https://github.com/lvgl/lvgl_esp32_drivers) but this is not aimed at the Arduino framework but ESP-IDF framework

So the aim is to create a library to support these boards without any effort.

These can be bought in the [Sunton Store](https://www.aliexpress.com/store/1100192306)

- [2.8" 240x320 TFT Resistive touch](https://www.aliexpress.com/item/1005004502250619.html)
- [3.5" 320x480 TFT Resistive/Capacitive touch](https://www.aliexpress.com/item/1005004632953455.html)

They offer:

- ESP32
- Additional Flash
- Display with touch (IL9341/ST7796 and XPT2046/GT911)
- Light sensor (CDS)
- Audio amplifier
- RGB led
- Extension connectors
- USB Serial interface

