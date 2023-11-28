# ESP32-SmartDisplay

[![Platform IO CI](https://github.com/rzeldent/esp32-smartdisplay/actions/workflows/main.yml/badge.svg)](https://github.com/rzeldent/esp32-smartdisplay/actions/workflows/main.yml)

## LVGL drivers and peripheral interface for Chinese Sunton Smart display boards, alias CYD (Cheap Yellow Display)

These boards have an LCD display and most of them have a touch interface; N = No touch, R = Resistive touch, C = Capacitive touch.
Currently this library supports: 2424S012N/C, 2432S024R/C/N, 2432S028R, 2432S032N/R/C, 3248S035R/C, 4827S043R/C, 8048S050N/C and 8048S070N/C.

This library supports these boards without any effort.

## Why this library

With the boards, there is a link supplied and there are a lot of examples present and this looks fine.
These examples for [LVGL](https://lvgl.io/) depend on external libraries ([TFT_eSPI](https://github.com/Bodmer/TFT_eSPI) or [LovyanGFX](https://github.com/lovyan03/LovyanGFX)).
However, when working with these libraries, I found out that these libraries had their flaws using these boards:

- A lot of configuring to do before it all works
- A lot of not unnecessary code is included (for other boards)
- No support for on the fly rotating
- No auto of the box support for touch

This library uses the "official" drivers from Espressif's component service. These drivers use the newly introduced esp_lcd_panel interfaces.
This should provide some support in the future for updates and new boards. These drivers are copied to this library.

## How to obtain these boards

These boards are available on AliExpress for decent prices and offer a lot.
They can be bought in the [Sunton Store](https://www.aliexpress.com/store/1100192306) on AliExpress but saw them also from other sellers.

## Supported boards

| Type            | CPU       | Display | Size  | Controller                                | Rotate support  | Color16 swap  | Touch                                     | Audio  | Flash                                          | RGB LED  | CDS    | Link|
|---              |---        |---      |--     |---                                        |---              |---            |---                                        |---     |---                                             |---       |---     |---  |
| ESP32-2424S012N | ESP32-C3  | 240x240 | 1.2"  | [GC9A01A](assets/datasheets/GC9A01A.pdf)  | yes             | yes           | n/a                                       | no     | no                                             | no       | no     | [Ali Express](https://www.aliexpress.com/item/1005005453515690.html) |
| ESP32-2424S012C | ESP32-C3  | 240x240 | 1.2"  | [GC9A01A](assets/datasheets/GC9A01A.pdf)  | yes             | yes           | [CST816S](assets/datasheets/CST816S.pdf)  | no     | no                                             | no       | no     | [Ali Express](https://www.aliexpress.com/item/1005005453515690.html) |
| ESP32-2432S024N | ESP32     | 240x320 | 2.4"  | [ILI9341](assets/datasheets/ILI9341.pdf)  | yes             | yes           | n/a                                       | yes    | [W25Q32JV](assets/datasheets/25Q32JVSSIQ.pdf)  | yes      | yes    | [Ali Express](https://www.aliexpress.com/item/1005005865107357.html) |
| ESP32-2432S024R | ESP32     | 240x320 | 2.4"  | [ILI9341](assets/datasheets/ILI9341.pdf)  | yes             | yes           | [XPT2046](assets/datasheets/XPT2046.pdf)  | yes    | [W25Q32JV](assets/datasheets/25Q32JVSSIQ.pdf)  | yes      | yes    | [Ali Express](https://www.aliexpress.com/item/1005005865107357.html) |
| ESP32-2432S024C | ESP32     | 240x320 | 2.4"  | [ILI9341](assets/datasheets/ILI9341.pdf)  | yes             | yes           | [CST816S](assets/datasheets/CST816S.pdf)  | yes    | [W25Q32JV](assets/datasheets/25Q32JVSSIQ.pdf)  | yes      | yes    | [Ali Express](https://www.aliexpress.com/item/1005005865107357.html) |
| ESP32-2432S028R | ESP32     | 240x320 | 2.8"  | [ILI9341](assets/datasheets/ILI9341.pdf)  | yes             | yes           | [XPT2046](assets/datasheets/XPT2046.pdf)  | yes    | [W25Q32JV](assets/datasheets/25Q32JVSSIQ.pdf)  | yes      | yes    | [Ali Express](https://www.aliexpress.com/item/1005004502250619.html) |
| ESP32-2432S032N | ESP32     | 240x320 | 3.2"  | [ST7796](assets/datasheets/ST7796.pdf)    | yes             | yes           | n/a                                       | yes    | [W25Q32JV](assets/datasheets/25Q32JVSSIQ.pdf)  | yes      | yes    | [Ali Express](https://www.aliexpress.com/item/1005006224494145.html) |
| ESP32-2432S032R | ESP32     | 240x320 | 3.2"  | [ST7796](assets/datasheets/ST7796.pdf)    | yes             | yes           | [XPT2046](assets/datasheets/XPT2046.pdf)  | yes    | [W25Q32JV](assets/datasheets/25Q32JVSSIQ.pdf)  | yes      | yes    | [Ali Express](https://www.aliexpress.com/item/1005006224494145.html) |
| ESP32-2432S032C | ESP32     | 240x320 | 3.2"  | [ST7796](assets/datasheets/ST7796.pdf)    | yes             | yes           | [GT911](assets/datasheets//GT911.pdf)     | yes    | [W25Q32JV](assets/datasheets/25Q32JVSSIQ.pdf)  | yes      | yes    | [Ali Express](https://www.aliexpress.com/item/1005006224494145.html) |
| ESP32-3248S035R | ESP32     | 320x480 | 3.5"  | [ST7796](assets/datasheets/ST7796.pdf)    | yes             | yes           | [XPT2046](assets/datasheets/XPT2046.pdf)  | yes    | [W25Q32JV](assets/datasheets/25Q32JVSSIQ.pdf)  | yes      | yes    | [Ali Express](https://www.aliexpress.com/item/1005004632953455.html) |
| ESP32-3248S035C | ESP32     | 320x480 | 3.5"  | [ST7796](assets/datasheets/ST7796.pdf)    | yes             | yes           | [GT911](assets/datasheets//GT911.pdf)     | yes    | [W25Q32JV](assets/datasheets/25Q32JVSSIQ.pdf)  | yes      | yes    | [Ali Express](https://www.aliexpress.com/item/1005004632953455.html) |
| ESP32-4827S043R | ESP32-S3  | 480x272 | 4.3"  | n/a                                       | no              | no            | [XPT2046](assets/datasheets/XPT2046.pdf)  | no     | no                                             | no       | no     | [Ali Express](https://www.aliexpress.com/item/1005006110360174.html) |
| ESP32-4827S043C | ESP32-S3  | 480x272 | 4.3"  | n/a                                       | no              | no            | [GT911](assets/datasheets//GT911.pdf)     | no     | no                                             | no       | no     | [Ali Express](https://www.aliexpress.com/item/1005006110360174.html) |
| ESP32-8048S050N | ESP32-S3  | 800x480 | 5.0"  | n/a                                       | no              | no            | n/a                                       | no     | no                                             | no       | no     | [Ali Express](https://www.aliexpress.com/item/1005005938915207.html) |
| ESP32-8048S050C | ESP32-S3  | 800x480 | 5.0"  | n/a                                       | no              | no            | [GT911](assets/datasheets//GT911.pdf)     | no     | no                                             | no       | no     | [Ali Express](https://www.aliexpress.com/item/1005005938915207.html) |
| ESP32-8048S070N | ESP32-S3  | 800x480 | 7.0"  | n/a                                       | no              | no            | n/a                                       | no     | no                                             | no       | no     | [Ali Express](https://www.aliexpress.com/item/1005005928865239.html) |
| ESP32-8048S070C | ESP32-S3  | 800x480 | 7.0"  | n/a                                       | no              | no            | [GT911](assets/datasheets//GT911.pdf)     | no     | no                                             | no       | no     | [Ali Express](https://www.aliexpress.com/item/1005005928865239.html) |

Note: the additional flash chip (W25Q32JV) is not always mounted on the board.

## Dependencies

This library depends on:

- LVGL (version ^8.3.2)

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

  - esp32-c3-devkitm-1
    - ESP32_2424S012N
    - ESP32_2424S012C

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

These can be defined in the `platformio.ini` file defining the settings. There also additional settings present for the EspressIf libraries. They should also be present.

```ini
[env]
platform = espressif32
framework = arduino

build_flags =
    -Ofast
    -Wall
    -D CORE_DEBUG_LEVEL=ARDUHAL_LOG_LEVEL_VERBOSE
    # LVGL settings. Point to your lv_conf.h file
    -D LV_CONF_PATH="${PROJECT_DIR}/example/lv_conf.h"
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

[env:esp32-c3-devkitm-1]
board = esp32-c3-devkitm-1
build_flags =
    ${env.build_flags}
    # Smartdisplay selection
    #-D ESP32_2424S012N
    #-D ESP32_2424S012C

[env:esp32-s3-devkitc-1]
board = esp32-s3-devkitc-1
build_flags =
    ${env.build_flags}
    # Smartdisplay selection
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
```

## LVGL.h

To use the LVGL library, a `lv_conf.h` file is required to define the settings for LVGL.
This file needs to be provided by the application.
As this file is referenced from the build of LVGL, the path must be known.
Normally this file is included in the include directory of **your** project and used by the LVGL library.
To include it globally, the define must be (for the include directory):

```ini
    -D LV_CONF_PATH=${PROJECT_INCLUDE_DIR}/lv_conf.h
```

The template for the `lv_conf.h` file can be found in the LVGL library at `.pio/libdeps/esp32dev/lvgl/lv_conf_template.h`.

## LV_COLOR_16_SWAP

The LVGL library has a define called **LV_COLOR_16_SWAP**. The value can be 1 (yes) or 0 (no).
This variable will swap the byte order of the lv_color16_t. This is required because the SPI is by default MSB first.
Swapping these bytes will undo this.
Setting this variable to true is required for the GC9A01A, ILI9341 and ST7796 controllers.

Additionally, when using the [SquareLine Studio](https://squareline.io/) for designing the user interface, the display properties (under the project settings) must match this variable.
It needs to be set both in `lv_conf.h` configuration file and the corresponding display properties (16 bit swap or 16 bit) in [SquareLine Studio](https://squareline.io/). When using squareline, the UI has to be regenerated.

![SquareLine display properties](assets/images/Squareline-display-properties.png)

## LVGL initialization Functions

The library exposes the following functions:

### void smartdisplay_init()

This is the first function that needs to be called.
It initializes the display controller and touch controller and will turn on the display.

### void smartdisplay_tft_set_backlight(uint16_t duty)

Set the brightness of the backlight display.
The resolution is 12 bit so valid values are from 0-1023.

## Controlling the RGB led

If the board has an RGB led, the define ```HAS_RGB_LED``` is defined.
Additionally, the following defines are present for the definition of the GPIO pins:

- LED_PIN_R
- LED_PIN_G
- LED_PIN_B

Before using the RGB LEDs, the GPIOs must be defined as output

```c++
  pinmode(LED_PIN_R, OUTPUT);
  pinmode(LED_PIN_G, OUTPUT);
  pinmode(LED_PIN_B, OUTPUT);
```

The LEDs are connected between the GPIO pin and the 3.3V. So the LED will light up if the GPIO is set to LOW.
For example: set the RGB led to red is done by the following code:

```c++
  digitalWrite(LED_PIN_R, false);
  digitalWrite(LED_PIN_G, true);
  digitalWrite(LED_PIN_B, true);
```

To have more colors than the 8 RGB combinations, PWM can be used to mix the colors.
To do this, attach a PWM channel to each GPIO pin to modulate the intensity.

## Reading the light sensor (CDS)

If the board has a light sensor, the define ```HAS_LIGHTSENSOR``` is defined.
The light sensor is a light sensitive resistor. It is attached to the analogue input of the ESP32.

To use the sensor, the define ```LIGHTSENSOR_IN``` indicates the analogue port to read.

```c++
  analogSetAttenuation(ADC_0db); // 0dB(1.0x) 0~800mV
  pinMode(LIGHTSENSOR_IN, INPUT);
```

The value read is between 0 (light) - 1023 (dark). To read the value, use:

```c++
  auto value = analogRead(LIGHTSENSOR_IN);
```

## Controlling the speaker

An 8 Ohm speaker can be connected to the Speak pin. This is a 1.25 JST connector.

Beeps can be generated by a PWM signal on the Speaker pin:

```c++
pinmode(SPEAKER_PIN, OUTPUT)
```

To produce real audio connect the internal D2A converter in the ESP32. Because the speaker is connected to GPIO26, this is the DAC2 (Left channel).
After that, audio can be streamed to the i2s.

The audio is a bit distorted. HexeguitarDIY has a fix for that by changing the resistor values to prevent distortion.
[![HexeguitarDIY Audio mod](https://img.youtube.com/vi/6JCLHIXXVus/0.jpg)](https://www.youtube.com/watch?v=6JCLHIXXVus)

## Demo application

An bare minimum application to demonstrate the library can be found at [esp32-smartdisplay-demo](https://github.com/rzeldent/esp32-smartdisplay-demo).
This application uses this library and the SquareLine Studio GUI generator.

## Board details

### ESP32-2424S012 N/C

- USB-C
- I2C: 1 x SH1.0 4p
- Power: JST1.25 2p

![ESP32-2424S012 front](assets/images/esp32-2424S012-front.png)
![ESP32-2424S012 back](assets/images/esp32-2424S012-back.png)

### ESP32-2432S024 N/R/C

- Micro USB
- I2C: 2 x JST1.25 4p
- Power + Serial: JST1.25 4p
- Speaker: JST1.25 2p

![ESP32-2432S024 front](assets/images/esp32-2432S024-front.png)

### ESP32-2432S028 R

- Micro USB
- I2C: 2 x JST1.25 4p
- Power + Serial: JST1.25 4p
- Speaker: JST1.25 2p

![ESP32-2432S028R back](assets/images/esp32-2432S028R-back.png)

### ESP32-3248S035

- Micro USB
- I2C: 2 x JST1.25 4p
- Power + Serial: JST1.25 4p
- Speaker: JST1.25 2p

![ESP32-3248S035 front](assets/images/esp32-3248S035-front.png)
![ESP32-3248S035 back](assets/images/esp32-3248S035-back.png)

### ESP32-4827S043

- USB-C
- I2C: JST1.25 4p
- Power + Serial: JST1.25 4p

![ESP32-4827S043 front](assets/images/esp32-4827S043-front.png)
![ESP32-4827S043 back](assets/images/esp32-4827S043-back.png)

### ESP32-8048S050

- USB-C
- I2C: JST1.25 4p
- Power + Serial: JST1.25 4p

![ESP32-8048S050 front](assets/images/esp32-8048S050-front.png)
![ESP32-8048S050 back](assets/images/esp32-8048S050-back.png)

### ESP32-8048S070

- USB-C
- I2C: JST1.25 4p
- Power + Serial: JST1.25 4p

![ESP32-8048S070 front](assets/images/esp32-8048S070-front.png)
![ESP32-8048S070 back](assets/images/esp32-8048S070-back.png)

## External dependencies

The following libraries are used from the EspressIf registry:

| Name                  | Version |
|---                    |---      |
| [ESP LCD ILI9341](https://components.espressif.com/api/download/?object_type=component&object_id=98a2daed-183c-47d8-b1a8-b7aa9a198bb5)        | v1.2    |
| [ESP LCD ST7796](https://components.espressif.com/api/download/?object_type=component&object_id=bdf53b24-6f59-4ab5-b92e-89ff0e94d307)         | v1.2    |
| [ESP LCD CG9A01](https://components.espressif.com/api/download/?object_type=component&object_id=6f06ecdf-97a6-4eea-ad4f-c00d11bd970a)         | v1.2    |
| [ESP LCD Touch](https://components.espressif.com/api/download/?object_type=component&object_id=bb4a4d94-2827-4695-84d1-1b53383b8001)          | v1.1.1  |
| [ESP LCD Touch CST816S](https://components.espressif.com/api/download/?object_type=component&object_id=cc8ef108-15e8-48cf-9be8-3c7e89ca493e)  | v1.0.3  |
| [ESP LCD Touch GT911](https://components.espressif.com/api/download/?object_type=component&object_id=4f44d570-8a04-466e-b4bb-429f1df7a9a1)    | v1.1.0  |
| [esp_lcd_touch driver](https://components.espressif.com/api/download/?object_type=component&object_id=225971c2-051f-4619-9f91-0080315ee8b8)   | v1.2.0  |

## Version history

- November 2023
  - Major version update: 2.0.0
  - Rewrite of the library to support the new ESP32-C3 and ESP32-S3 panels
  - Use the new Espressif esp_lcd interface
  - Use c instead of cpp
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
