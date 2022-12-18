# ESP32-SmartDisplay

LVGL drivers and peripheral interface for Chinese Smart display boards ESP32-2432S028R, ESP32-3248S035R, ESP32-3248S035C

These boards are available on AliExpress for decent prices but offer a lot.
With the boards, there is a link supplied and there are a lot of examples present and this looks fine.
These examples for LVGL depend on external libraries ([TFT_eSPI](https://github.com/Bodmer/TFT_eSPI) or [LovyanGFX](https://github.com/lovyan03/LovyanGFX)).
However, when implementing the capacitive version, I found out that these libraries had their flaws using these boards.
Additionally, There was a log of unnecessary code included in these libraries.
There is also a library for LVGL present [LVGL_ESP32_drivers](https://github.com/lvgl/lvgl_esp32_drivers) but this is not aimed at the Arduino framework but ESP-IDF framework.

So the aim is to create a library to support these boards without any effort.

These can be bought in the [Sunton Store](https://www.aliexpress.com/store/1100192306) on AliExpress.

- [ESP32-2432S028R - 2.8" 240x320 TFT Resistive touch](https://www.aliexpress.com/item/1005004502250619.html)
- [ESP32-3248S035R/C 3.5" 320x480 TFT Resistive/Capacitive touch](https://www.aliexpress.com/item/1005004632953455.html)

These boards offer:

- ESP32-WROOM
- 4MB Additional Flash
- Display with touch (IL9341/ST7796 and XPT2046/GT911)
- Light sensor (CDS)
- Audio amplifier
- RGB led (3.5" front, 2.8" back)
- Extension connectors
- USB Serial interface
- 4 Holes so can easily be attached

## How to use

Basically there are **two** defines that need to be present:

- Type of board
    - ESP32_2432S028R
    - ESP32_3248S035R
    - ESP32_3248S035C
- Orientation of the board
    - TFT_ORIENTATION_PORTRAIT
    - TFT_ORIENTATION_LANDSCAPE
    - TFT_ORIENTATION_PORTRAIT_INV
    - TFT_ORIENTATION_LANDSCAPE_INV

These can be defined in the ```platformio.ini``` file by (un)define the settings:

```ini
build_flags =
    ...
    ...
    # LVGL settings
    -D LV_LVGL_H_INCLUDE_SIMPLE
    # For lv_conf.h   
    -I ./include
    # Smartdisplay settings 
    -D TFT_ORIENTATION_PORTRAIT
    #-D TFT_ORIENTATION_LANDSCAPE
    #-D TFT_ORIENTATION_PORTRAIT_INV
    #-D TFT_ORIENTATION_LANDSCAPE_INV
    -D ESP32_2432S028R
    #-D ESP32_3248S035R
    #-D ESP32_3248S035C
```
