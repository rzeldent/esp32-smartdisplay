#pragma once

#include <Arduino.h>
#include <SPI.h>
#include <lvgl.h>
#include <mutex>

// Mutex to access lvgl if multi-threaded
extern std::recursive_mutex lvgl_mutex;
// Initialize the display and touch
extern void smartdisplay_init();
// Set the color of the led
extern void smartdisplay_set_led_color(lv_color32_t rgb);
// Get the value of the CDS sensor
extern int smartdisplay_get_light_intensity();
// Beep with the specified frequency and duration
extern void smartdisplay_beep(unsigned int frequency, unsigned long duration);
// Set the brightness of the backlight display
extern void smartdisplay_tft_set_backlight(uint16_t duty); // 0-1023 (12 bits)
// Put the display to sleep
extern void smartdisplay_tft_sleep();
// Wake the display
extern void smartdisplay_tft_wake();

// Default orientation
#if !defined(TFT_ORIENTATION_PORTRAIT) && !defined(TFT_ORIENTATION_LANDSCAPE) && !defined(TFT_ORIENTATION_PORTRAIT_INV) && !defined(TFT_ORIENTATION_LANDSCAPE_INV)
#define TFT_ORIENTATION_PORTRAIT
#endif

// Default RGB order
#if !defined(TFT_PANEL_ORDER_RGB) && !defined(TFT_PANEL_ORDER_BGR)
#define TFT_PANEL_ORDER_BGR
#endif

#if defined(ESP32_2432S024N) || defined(ESP32_2432S024R) || defined(ESP32_2432S024C)
#define ESP32_SMARTDISPLAY_BOARD_VALID
#define TFT_WIDTH 240
#define TFT_HEIGHT 320
#define ILI9431
#define ILI9431_SPI_SCLK 14
#define ILI9431_SPI_MOSI 13
#define ILI9431_SPI_MISO 12
#define ILI9341_PIN_CS 15
#define ILI9341_PIN_DC 2
#define ILI9341_SPI_FREQ 50000000
#define ILI9341_PIN_BL 21
#define PWM_CHANNEL_BL 12
#define PWM_FREQ_BL 5000
#define PWM_BITS_BL 8
#define PWM_MAX_BL ((1 << PWM_BITS_BL) - 1)
extern SPIClass spi_ili9431;
// Push button
#define PUSHBUTTON_BOOT 0
// Build in RGB LED
#define LED_PIN_R 4
#define LED_PIN_G 16
#define LED_PIN_B 17
// PWM channels for RGB
#define LED_PWM_FREQ 5000
#define LED_PWM_CHANNEL_R 13
#define LED_PWM_CHANNEL_G 14
#define LED_PWM_CHANNEL_B 15
#define LED_PWM_BITS 8
#define LED_PWM_MAX ((1 << LED_PWM_BITS) - 1)
// Photo resistor
#define LIGHTSENSOR_IN 34 // ANALOG_PIN_0
// Audio out
#define SPEAKER_OUT 26
// TF Card
#define TF_PIN_CS 5
#define TF_PIN_MOSI 23
#define TF_PIN_SCLK 18
#define TF_PIN_MISO 19
#endif

// ESP32_2432S024N
#ifdef ESP32_2432S024R
#endif

// ESP32_2432S024R
#ifdef ESP32_2432S024R
// Touch
#define XPT2046
#define XPT2046_SPI_SCLK 25
#define XPT2046_SPI_MOSI 32
#define XPT2046_SPI_MISO 39
#define XPT2046_SPI_FREQ 2000000
#define XPT2046_PIN_INT 36
#define XPT2046_PIN_CS 33
// Calibration 240x320
#define XPT2046_MIN_X 349
#define XPT2046_MAX_X 3859
#define XPT2046_MIN_Y 247
#define XPT2046_MAX_Y 3871
extern SPIClass spi_xpt2046;
#endif

// ESP32_2432S024C
#ifdef ESP32_2432S024C
// Touch
#define CST820
#define CST820_IIC_SDA 33
#define CST820_IIC_SCL 32
#define CST820_IIC_RST 25
#include <Wire.h>
extern TwoWire i2c_cst820;
#endif

#if defined(ESP32_2432S028R)
#define ESP32_SMARTDISPLAY_BOARD_VALID
#define TFT_WIDTH 240
#define TFT_HEIGHT 320
#define ILI9431
#define ILI9431_SPI_SCLK 14
#define ILI9431_SPI_MOSI 13
#define ILI9431_SPI_MISO 12
#define ILI9341_PIN_CS 15
#define ILI9341_PIN_DC 2
#define ILI9341_SPI_FREQ 50000000
#define ILI9341_PIN_BL 21
#define PWM_CHANNEL_BL 12
#define PWM_FREQ_BL 5000
#define PWM_BITS_BL 8
#define PWM_MAX_BL ((1 << PWM_BITS_BL) - 1)
extern SPIClass spi_ili9431;
// Push button
#define PUSHBUTTON_BOOT 0
// Build in RGB LED
#define LED_PIN_R 4
#define LED_PIN_G 16
#define LED_PIN_B 17
// PWM channels for RGB
#define LED_PWM_FREQ 5000
#define LED_PWM_CHANNEL_R 13
#define LED_PWM_CHANNEL_G 14
#define LED_PWM_CHANNEL_B 15
#define LED_PWM_BITS 8
#define LED_PWM_MAX ((1 << LED_PWM_BITS) - 1)
// Photo resistor
#define LIGHTSENSOR_IN 34 // ANALOG_PIN_0
// Audio out
#define SPEAKER_OUT 26
// TF Card
#define TF_PIN_CS 5
#define TF_PIN_MOSI 23
#define TF_PIN_SCLK 18
#define TF_PIN_MISO 19
// Touch
#define XPT2046
#define XPT2046_SPI_SCLK 25
#define XPT2046_SPI_MOSI 32
#define XPT2046_SPI_MISO 39
#define XPT2046_SPI_FREQ 2000000
#define XPT2046_PIN_INT 36
#define XPT2046_PIN_CS 33
// Calibration 240x320
#define XPT2046_MIN_X 349
#define XPT2046_MAX_X 3859
#define XPT2046_MIN_Y 247
#define XPT2046_MAX_Y 3871
extern SPIClass spi_xpt2046;
#endif

#if defined(ESP32_3248S035R) || defined(ESP32_3248S035C)
#define ESP32_SMARTDISPLAY_BOARD_VALID
#define TFT_WIDTH 320
#define TFT_HEIGHT 480
#define ST7796
#define ST7796_SPI_SCLK 14
#define ST7796_SPI_MOSI 13
#define ST7796_SPI_MISO 12
#define ST7796_PIN_CS 15
#define ST7796_PIN_DC 2
#define ST7796_SPI_FREQ 80000000
#define ST7796_PIN_BL 27
#define PWM_CHANNEL_BL 12
#define PWM_FREQ_BL 5000
#define PWM_BITS_BL 8
#define PWM_MAX_BL ((1 << PWM_BITS_BL) - 1)
extern SPIClass spi_st7796;
// Push button
#define PUSHBUTTON_BOOT 0
// Build in RGB LED
#define LED_PIN_R 4
#define LED_PIN_G 16
#define LED_PIN_B 17
// PWM channels for RGB
#define LED_PWM_FREQ 5000
#define LED_PWM_CHANNEL_R 13
#define LED_PWM_CHANNEL_G 14
#define LED_PWM_CHANNEL_B 15
#define LED_PWM_BITS 8
#define LED_PWM_MAX ((1 << LED_PWM_BITS) - 1)
// Photo resistor
#define LIGHTSENSOR_IN 34 // ANALOG_PIN_0
// Audio out
#define SPEAKER_OUT 26
// TF Card
#define TF_PIN_CS 5
#define TF_PIN_MOSI 23
#define TF_PIN_SCLK 18
#define TF_PIN_MISO 19
#endif

// ESP32_3248S035R
#ifdef ESP32_3248S035R
// Touch
#define XPT2046
#define XPT2046_SPI_SCLK 14
#define XPT2046_SPI_MOSI 13
#define XPT2046_SPI_MISO 12
#define XPT2046_SPI_FREQ 2000000
#define XPT2046_PIN_INT 36
#define XPT2046_PIN_CS 33
// Calibration 320x480
#define XPT2046_MIN_X 256
#define XPT2046_MAX_X 3860
#define XPT2046_MIN_Y 180
#define XPT2046_MAX_Y 3900
#define spi_xpt2046 spi_st7796
#endif

#ifdef ESP32_3248S035C
#define GT911
#define GT911_IIC_SDA 33
#define GT911_IIC_SCL 32
#define GT911_IIC_RST 25
#include <Wire.h>
extern TwoWire i2c_gt911;
#endif

#if defined(ESP32_8048S043N) || defined(ESP32_8048S043R) || defined(ESP32_8048S043C)
#define ESP32_SMARTDISPLAY_BOARD_VALID
#define TFT_WIDTH 800
#define TFT_HEIGHT 480
#define TFT_DIRECT_IO
#define TFT_DE 40
#define TFT_VSYNC 41
#define TFT_HSYNC 39
#define TFT_PCLK 42
#define TFT_R0 45
#define TFT_R1 48
#define TFT_R2 47
#define TFT_R3 21
#define TFT_R4 14
#define TFT_G0 5
#define TFT_G1 6
#define TFT_G2 7
#define TFT_G3 15
#define TFT_G4 16
#define TFT_G5 4
#define TFT_B0 8
#define TFT_B1 3
#define TFT_B2 46
#define TFT_B3 9
#define TFT_B4 1
#define PIN_BL 2
#define PWM_CHANNEL_BL 12
#define PWM_FREQ_BL 5000
#define PWM_BITS_BL 8
#define PWM_MAX_BL ((1 << PWM_BITS_BL) - 1)
extern SPIClass spi_ili9431;
// Push button
#define PUSHBUTTON_BOOT 0
// Build in RGB LED
#define LED_PIN_R 4
#define LED_PIN_G 16
#define LED_PIN_B 17
// PWM channels for RGB
#define LED_PWM_FREQ 5000
#define LED_PWM_CHANNEL_R 13
#define LED_PWM_CHANNEL_G 14
#define LED_PWM_CHANNEL_B 15
#define LED_PWM_BITS 8
#define LED_PWM_MAX ((1 << LED_PWM_BITS) - 1)
// Photo resistor
#define LIGHTSENSOR_IN 34 // ANALOG_PIN_0
// TF Card
#define TF_PIN_CS 10
#define TF_PIN_MOSI 11
#define TF_PIN_SCLK 12
#define TF_PIN_MISO 13
#endif

#ifdef ESP32_8048S043N
#endif

#ifdef ESP32_8048S043R
// Touch
#define XPT2046
#define XPT2046_SPI_SCLK 12
#define XPT2046_SPI_MOSI 11
#define XPT2046_SPI_MISO 13
#define XPT2046_SPI_FREQ 2000000
#define XPT2046_PIN_INT 18
#define XPT2046_PIN_CS 38
// Calibration
#define XPT2046_MIN_X 349
#define XPT2046_MAX_X 3859
#define XPT2046_MIN_Y 247
#define XPT2046_MAX_Y 3871
extern SPIClass spi_xpt2046;
#endif

#ifdef ESP32_8048S043C
#define GT911
#define GT911_IIC_SDA 19
#define GT911_IIC_SCL 20
#define GT911_IIC_RST 38
#include <Wire.h>
extern TwoWire i2c_gt911;
#endif

#if defined(ESP32_8048S050N) || defined(ESP32_8048S050R) || defined(ESP32_8048S050C)
#define ESP32_SMARTDISPLAY_BOARD_VALID
#define TFT_WIDTH 800
#define TFT_HEIGHT 480
#define TFT_DIRECT_IO
#define TFT_DE 40
#define TFT_VSYNC 41
#define TFT_HSYNC 39
#define TFT_PCLK 42
#define TFT_R0 45
#define TFT_R1 48
#define TFT_R2 47
#define TFT_R3 21
#define TFT_R4 14
#define TFT_G0 5
#define TFT_G1 6
#define TFT_G2 7
#define TFT_G3 15
#define TFT_G4 16
#define TFT_G5 4
#define TFT_B0 8
#define TFT_B1 3
#define TFT_B2 46
#define TFT_B3 9
#define TFT_B4 1
#define ILI9341_PIN_BL 2
#define PWM_CHANNEL_BL 12
#define PWM_FREQ_BL 5000
#define PWM_BITS_BL 8
#define PWM_MAX_BL ((1 << PWM_BITS_BL) - 1)
extern SPIClass spi_ili9431;
// Push button
#define PUSHBUTTON_BOOT 0
// Build in RGB LED
#define LED_PIN_R 4
#define LED_PIN_G 16
#define LED_PIN_B 17
// PWM channels for RGB
#define LED_PWM_FREQ 5000
#define LED_PWM_CHANNEL_R 13
#define LED_PWM_CHANNEL_G 14
#define LED_PWM_CHANNEL_B 15
#define LED_PWM_BITS 8
#define LED_PWM_MAX ((1 << LED_PWM_BITS) - 1)
// Photo resistor
#define LIGHTSENSOR_IN 34 // ANALOG_PIN_0
// TF Card
#define TF_PIN_CS 10
#define TF_PIN_MOSI 11
#define TF_PIN_SCLK 12
#define TF_PIN_MISO 13
#endif

#ifdef ESP32_8048S050N
#endif

#ifdef ESP32_8048S050R
// Touch
#define XPT2046
#define XPT2046_SPI_SCLK 12
#define XPT2046_SPI_MOSI 11
#define XPT2046_SPI_MISO 13
#define XPT2046_SPI_FREQ 2000000
#define XPT2046_PIN_INT 18
#define XPT2046_PIN_CS 38
// Calibration
#define XPT2046_MIN_X 349
#define XPT2046_MAX_X 3859
#define XPT2046_MIN_Y 247
#define XPT2046_MAX_Y 3871
extern SPIClass spi_xpt2046;
#endif

#ifdef ESP32_8048S050C
#define GT911
#define GT911_IIC_SDA 19
#define GT911_IIC_SCL 20
#define GT911_IIC_RST 38
#include <Wire.h>
extern TwoWire i2c_gt911;
#endif

#if defined(ESP32_8048S070N) || defined(ESP32_8048S070R) || defined(ESP32_8048S070C)
#define ESP32_SMARTDISPLAY_BOARD_VALID
#define TFT_WIDTH 800
#define TFT_HEIGHT 480
#define TFT_DIRECT_IO
#define TFT_DE 40
#define TFT_VSYNC 41
#define TFT_HSYNC 39
#define TFT_PCLK 42
#define TFT_R0 45
#define TFT_R1 48
#define TFT_R2 47
#define TFT_R3 21
#define TFT_R4 14
#define TFT_G0 5
#define TFT_G1 6
#define TFT_G2 7
#define TFT_G3 15
#define TFT_G4 16
#define TFT_G5 4
#define TFT_B0 8
#define TFT_B1 3
#define TFT_B2 46
#define TFT_B3 9
#define TFT_B4 1
#define PWM_CHANNEL_BL 12
#define PWM_FREQ_BL 5000
#define PWM_BITS_BL 8
#define PWM_MAX_BL ((1 << PWM_BITS_BL) - 1)
extern SPIClass spi_ili9431;
// Push button
#define PUSHBUTTON_BOOT 0
// Build in RGB LED
#define LED_PIN_R 4
#define LED_PIN_G 16
#define LED_PIN_B 17
// PWM channels for RGB
#define LED_PWM_FREQ 5000
#define LED_PWM_CHANNEL_R 13
#define LED_PWM_CHANNEL_G 14
#define LED_PWM_CHANNEL_B 15
#define LED_PWM_BITS 8
#define LED_PWM_MAX ((1 << LED_PWM_BITS) - 1)
// Photo resistor
#define LIGHTSENSOR_IN 34 // ANALOG_PIN_0
// TF Card
#define TF_PIN_CS 10
#define TF_PIN_MOSI 11
#define TF_PIN_SCLK 12
#define TF_PIN_MISO 13
#endif

#ifdef ESP32_8048S070N
#endif

#ifdef ESP32_8048S070R
// Touch
#define XPT2046
#define XPT2046_SPI_SCLK 12
#define XPT2046_SPI_MOSI 11
#define XPT2046_SPI_MISO 13
#define XPT2046_SPI_FREQ 2000000
#define XPT2046_PIN_INT 18
#define XPT2046_PIN_CS 38
// Calibration
#define XPT2046_MIN_X 349
#define XPT2046_MAX_X 3859
#define XPT2046_MIN_Y 247
#define XPT2046_MAX_Y 3871
extern SPIClass spi_xpt2046;
#endif

#ifdef ESP32_8048S070C
#define GT911
#define GT911_IIC_SDA 19
#define GT911_IIC_SCL 20
#define GT911_IIC_RST 38
#include <Wire.h>
extern TwoWire i2c_gt911;
#endif

// Buffer size for drawing
#define DRAW_BUFFER_SIZE (TFT_WIDTH * 10)
