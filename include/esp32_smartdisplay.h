#pragma once

#include <Arduino.h>
#include <SPI.h>
#include <lvgl.h>
#ifdef ESP32_3248S035C
#include <Wire.h>
#endif

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

// Buffer size for drawing
#define DRAW_BUFFER_SIZE (TFT_WIDTH * 10)

// ESP32_2432S028R
#ifdef ESP32_2432S028R
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
#define ILI9341_PWM_CHANNEL_BL 12
#define ILI9341_PWM_FREQ_BL 5000
#define ILI9341_PWM_BITS_BL 8
#define ILI9341_PWM_MAX_BL ((1 << ILI9341_PWM_BITS_BL) - 1)
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

extern SPIClass spi_ili9431;
extern SPIClass spi_xpt2046;
#endif

// ESP32_3248S035R
#ifdef ESP32_3248S035R
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
#define ST7796_PWM_CHANNEL_BL 12
#define ST7796_PWM_FREQ_BL 5000
#define ST7796_PWM_BITS_BL 8
#define ST7796_PWM_MAX_BL ((1 << ST7796_PWM_BITS_BL) - 1)
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

extern SPIClass spi_st7796;
#define spi_xpt2046 spi_st7796
#endif

#ifdef ESP32_3248S035C
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
#define ST7796_PWM_CHANNEL_BL 12
#define ST7796_PWM_FREQ_BL 5000
#define ST7796_PWM_BITS_BL 8
#define ST7796_PWM_MAX_BL ((1 << ST7796_PWM_BITS_BL) - 1)
#define GT911
#define GT911_IIC_SDA 33
#define GT911_IIC_SCL 32
#define GT911_IIC_RST 25

extern SPIClass spi_st7796;
extern TwoWire i2c_gt911;
#endif

#ifdef ESP32_8048S070C
#define TFT_WIDTH 800
#define TFT_HEIGHT 480
#define ILI9431
#define ILI9431_SPI_SCLK 14
#define ILI9431_SPI_MOSI 13
#define ILI9431_SPI_MISO 12
#define ILI9341_PIN_CS 15
#define ILI9341_PIN_DC 2
#define ILI9341_SPI_FREQ 50000000
#define ILI9341_PIN_BL 21
#define ILI9341_PWM_CHANNEL_BL 12
#define ILI9341_PWM_FREQ_BL 5000
#define ILI9341_PWM_BITS_BL 8
#define ILI9341_PWM_MAX_BL ((1 << ILI9341_PWM_BITS_BL) - 1)
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

extern SPIClass spi_ili9431;
extern SPIClass spi_xpt2046;
#endif

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
#define CDS_PIN 34 // ANALOG_PIN_0

// Audio out
#define AUDIO_PIN 26

// TF Card
#define TF_PIN_CS 5
#define TF_PIN_MOSI 23
#define TF_PIN_SCLK 18
#define TF_PIN_MISC 19
