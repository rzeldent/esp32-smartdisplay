# ESP32-SmartDisplay

[![Platform IO CI](https://github.com/rzeldent/esp32-smartdisplay/actions/workflows/main.yml/badge.svg)](https://github.com/rzeldent/esp32-smartdisplay/actions/workflows/main.yml)

## LVGL drivers and peripheral interface for Chinese Sunton Smart display boards 2432S024R/C/N, 2432S028R, 3248S035R/C, 8048S035N/C and 8048S070N/C

This library supports these boards without any effort.

## Why this library

With the boards, there is a link supplied and there are a lot of examples present and this looks fine.
These examples for [LVGL](https://lvgl.io/) depend on external libraries ([TFT_eSPI](https://github.com/Bodmer/TFT_eSPI) or [LovyanGFX](https://github.com/lovyan03/LovyanGFX)).
However, when implementing the capacitive version, I found out that these libraries had their flaws using these boards:

- A lot of not unnecessary code is included (for other boards)
- No support for on the fly rotating
- No auto of the box support for touch
- A lot of configuring to do before it all works

This library uses the "official" drivers from the EspressIf repository. These drivers use the newly introduced esp_lcd_panel interfaces.
This should provide some support in the future for updates and new boards.

## How to obtain these boards

These boards are available on AliExpress for decent prices and offer a lot.
They can be bought in the [Sunton Store](https://www.aliexpress.com/store/1100192306) on AliExpress but saw them also from other sellers.

## Supported boards

| Type            | CPU       | Display | Size  | Controller                        | Rotate support  | LV_COLOR_16_SWAP | Touch                              | Audio  | Flash  | RGB LED  | CDS    | Link|
|---              |---        |---      |--     |---                                |---              |---               |---                                 |---     |---     |---       |---     |--- |
| ESP32-2432S024N | ESP32     | 240x320 | 2.4"  | [ILI9341](datasheets/ILI9341.pdf) | yes             | yes              | n/a                                | yes    | yes    | yes      | yes    | [Ali Express](https://www.aliexpress.com/item/1005005865107357.html) |
| ESP32-2432S024R | ESP32     | 240x320 | 2.4"  | [ILI9341](datasheets/ILI9341.pdf) | yes             | yes              | [XPT2046](datasheets/XPT2046.pdf)  | yes    | yes    | yes      | yes    | [Ali Express](https://www.aliexpress.com/item/1005005865107357.html) |
| ESP32-2432S024C | ESP32     | 240x320 | 2.4"  | [ILI9341](datasheets/ILI9341.pdf) | yes             | yes              | [CST816S](datasheets/CST816S.pdf)  | yes    | yes    | yes      | yes    | [Ali Express](https://www.aliexpress.com/item/1005005865107357.html) |
| ESP32-2432S028R | ESP32     | 240x320 | 2.8"  | [ILI9341](datasheets/ILI9341.pdf) | yes             | yes              | [XPT2046](datasheets/XPT2046.pdf)  | yes    | yes    | yes      | yes    | [Ali Express](https://www.aliexpress.com/item/1005004502250619.html) |
| ESP32-2432S032N | ESP32     | 240x320 | 3.2"  | [ST7796](datasheets/ST7796.pdf)   | yes             | yes              | n/a                                | yes    | yes    | yes      | yes    | [Ali Express](https://www.aliexpress.com/item/1005006224494145.html) |
| ESP32-2432S032R | ESP32     | 240x320 | 3.2"  | [ST7796](datasheets/ST7796.pdf)   | yes             | yes              | [XPT2046](datasheets/XPT2046.pdf)  | yes    | yes    | yes      | yes    | [Ali Express](https://www.aliexpress.com/item/1005006224494145.html) |
| ESP32-2432S032C | ESP32     | 240x320 | 3.2"  | [ST7796](datasheets/ST7796.pdf)   | yes             | yes              | [GT911](datesheets/GT911.pdf)      | yes    | yes    | yes      | yes    | [Ali Express](https://www.aliexpress.com/item/1005006224494145.html) |
| ESP32-3248S035R | ESP32     | 320x480 | 3.5"  | [ST7796](datasheets/ST7796.pdf)   | yes             | yes              | [XPT2046](datasheets/XPT2046.pdf)  | yes    | yes    | yes      | yes    | [Ali Express](https://www.aliexpress.com/item/1005004632953455.html) |
| ESP32-3248S035C | ESP32     | 320x480 | 3.5"  | [ST7796](datasheets/ST7796.pdf)   | yes             | yes              | [GT911](datesheets/GT911.pdf)      | yes    | yes    | yes      | yes    | [Ali Express](https://www.aliexpress.com/item/1005004632953455.html) |
| ESP32-4827S043R | ESP32-S3  | 480x272 | 4.3"  | n/a                               | no              | no               | [XPT2046](datasheets/XPT2046.pdf)  | no     | no     | no       | no     | [Ali Express](https://www.aliexpress.com/item/1005006110360174.html) |
| ESP32-4827S043C | ESP32-S3  | 480x272 | 4.3"  | n/a                               | no              | no               | [GT911](datesheets/GT911.pdf)      | no     | no     | no       | no     | [Ali Express](https://www.aliexpress.com/item/1005006110360174.html) |
| ESP32-8048S050N | ESP32-S3  | 800x480 | 5.0"  | n/a                               | no              | no               | n/a                                | no     | no     | no       | no     | [Ali Express](https://www.aliexpress.com/item/1005005938915207.html) |
| ESP32-8048S050C | ESP32-S3  | 800x480 | 5.0"  | n/a                               | no              | no               | [GT911](datesheets/GT911.pdf)      | no     | no     | no       | no     | [Ali Express](https://www.aliexpress.com/item/1005005938915207.html) |
| ESP32-8048S070N | ESP32-S3  | 800x480 | 7.0"  | n/a                               | no              | no               | n/a                                | no     | no     | no       | no     | [Ali Express](https://www.aliexpress.com/item/1005005928865239.html) |
| ESP32-8048S070C | ESP32-S3  | 800x480 | 7.0"  | n/a                               | no              | no               | [GT911](datesheets/GT911.pdf)      | no     | no     | no       | no     | [Ali Express](https://www.aliexpress.com/item/1005005928865239.html) |

![ESP32-3248S035R front](assets/images/esp32-3248S035-front.png)
![ESP32-3248S035R back](assets/images/esp32-3248S035-back.png)

## Dependencies

This library depends on:

- LVGL (version ^8.3.2)

To use the LVGL library, a `lv_conf.h` file is required to define the settings for LVGL.
This file needs to be provided by the application.
As this file is referenced from the build of LVGL, the path must be known.
Normally this file is included in the include directory of **your** project so the define must be

```ini
    -D LV_CONF_PATH=${PROJECT_INCLUDE_DIR}/lv_conf.h
```

The template for the `lv_conf.h` file can be found in the LVGL library at `.pio/libdeps/esp32dev/lvgl/lv_conf_template.h`.

## How to use

Basically there is only **ONE** define that need to be defined: The type of board assuming everything is default.
Be aware that the platform board must also match but this is a requirement for a platform io project.

- Type of board

  - esp32dev
    - ESP32_2432S024R
    - ESP32_2432S024C
    - ESP32_2432S024N
    - ESP32_2432S028R
    - ESP32_3248S035R
    - ESP32_3248S035C

  - esp32-s3-devkitc-1
    - ESP32_8048S043N
    - ESP32_8048S043R
    - ESP32_8048S043C
    - ESP32_8048S050N
    - ESP32_8048S050R
    - ESP32_8048S050C
    - ESP32_8048S070N
    - ESP32_8048S070R
    - ESP32_8048S070C

These can be defined in the `platformio.ini` file defining the settings:

```ini
; PlatformIO Project Configuration File
;
;   Build options: build flags, source filter
;   Upload options: custom upload port, speed and extra flags
;   Library options: dependencies, extra library storages
;   Advanced options: extra scripting
;
; Please visit documentation for the other options and examples
; https://docs.platformio.org/page/projectconf.html

[platformio]
default_envs = esp32-s3-devkitc-1

[env]
platform = espressif32
framework = arduino
monitor_speed = 115200

monitor_filters = esp32_exception_decoder

build_flags =
    -Ofast
    -Wall
    -D CORE_DEBUG_LEVEL=ARDUHAL_LOG_LEVEL_VERBOSE
    # LVGL settings
    -D LV_CONF_PATH=${PROJECT_INCLUDE_DIR}/lv_conf.h
    # EspressIf library defines
    -D ESP_LCD_ST7796_VER_MAJOR=1
    -D ESP_LCD_ST7796_VER_MINOR=2
    -D ESP_LCD_ST7796_VER_PATCH=0
    -D ESP_LCD_ILI9341_VER_MAJOR=1
    -D ESP_LCD_ILI9341_VER_MINOR=2
    -D ESP_LCD_ILI9341_VER_PATCH=0
    -D CONFIG_ESP_LCD_TOUCH_MAX_POINTS=1
    -D CONFIG_XPT2046_CONVERT_ADC_TO_COORDS
    -D CONFIG_XPT2046_Z_THRESHOLD=600

lib_deps = rzeldent/esp32_smartdisplay

[env:esp32dev]
board = esp32dev

build_flags =
    ${env.build_flags}
    # Smartdisplay selection
    #-D ESP32_2432S024N
    #-D ESP32_2432S024R
    #-D ESP32_2432S024C
    #-D ESP32_2432S028R
    #-D ESP32_3248S032N
    #-D ESP32_3248S032R
    #-D ESP32_3248S032C
    #-D ESP32_3248S035R
    #-D ESP32_3248S035C

lib_deps = ${env.lib_deps}

[env:esp32-s3-devkitc-1]
board = esp32-s3-devkitc-1

build_flags =
    ${env.build_flags}
    # Smartdisplay selection
    #-D ESP32_4827S043N
    #-D ESP32_4827S043R
    #-D ESP32_4827S043C
    #-D ESP32_8048S043N
    #-D ESP32_8048S043R
    #-D ESP32_8048S043C
    #-D ESP32_8048S050N
    #-D ESP32_8048S050R
    #-D ESP32_8048S050C
    #-D ESP32_8048S070N
    #-D ESP32_8048S070R
    #-D ESP32_8048S070C

lib_deps = ${env.lib_deps}
```

The path for the lv_conf.h above is `${PROJECT_INCLUDE_DIR}`.
This needs to be specified because the LVGL library included this header file.

## Demo application

An bare minimum application to demonstrate the library can be found at [esp32-smartdisplay-demo](https://github.com/rzeldent/esp32-smartdisplay-demo).

## Functions

### void smartdisplay_init()

This is the first function that needs to be called.
It initializes the display controller and touch controller.

### void smartdisplay_tft_set_backlight(uint16_t duty)

Set the brightness of the backlight display. The resolution is 12 bit so 0-1023.

## Version history

- November 2023
  - Rewrote library to also support the new ESP32-S3 panels
- October 2023
  - Added support for esp32_8048S034N/C
  - Added option for flipped/mirrored TFT's
  - Changed default RGB order to BGR
  - Version 1.0.8 and 1.0.9
- September 2023
  - Added support for ESP32_2432S024N/R/S
  - Version 1.0.7
- August 2023
  - Added support for esp32_8048S070N/C
  - Display buffer size configurable
- February 2023
  - Version 1.0.3
  - Added variable for the LCD panel RGB/BGR order
- February 2023
  - Version 1.0.3
  - Added mutex for access to lvgl, required for multithreading
  - Changed RGB led input to lv_color32_t
- December 2022
  - Initial version 1.0.2.
  - Drivers for ESP32_2432S028R, ESP32_3248S035R and ESP32_3248S035C displays working.
  - Sound output
  - RGB Led output
  - CDS light sensor input
- July 2022
  - Initial work started