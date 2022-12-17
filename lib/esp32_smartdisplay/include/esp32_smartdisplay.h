#pragma once

#include <Arduino.h>
#include <lvgl.h>

#define ESP32_2432S028R // 2.8" 240x320 resistive ILI9341
// #define ESP32_3248S028R // 3.5" 320x480 resistive ST7796
// #define ESP32_3248S028C // 3.5" 320x480 capacitive ST7796

#define TFT_ORIENTATION_LANDSCAPE

#include <SPI.h>
#ifdef ESP32_3248S028C
#include <Wire.h>
#endif

extern void lvgl_init();
extern void setLedColor(uint8_t r, uint8_t g, uint8_t b);
extern int getLightIntensity();
extern void beep(unsigned int frequency, unsigned long duration);
extern void lvgl_tft_set_backlight(uint8_t value); // 0-255
extern void lvgl_tft_sleep();
extern void lvgl_tft_wake();

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
#define ILI9341_SPI_FREQ 80000000
#define ILI9341_PIN_BL 21
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

// ESP32_3248S028R
#ifdef ESP32_3248S028R
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

#ifdef ESP32_3248S028C
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
#define GT911
#define GT911_IIC_SDA 33
#define GT911_IIC_SCL 32
#define GT911_IIC_RST 25

extern SPIClass spi_st7796;
extern TwoWire i2c_gt911;
#endif

// Build in LED
#define LED_PIN_R 4
#define LED_PIN_G 16
#define LED_PIN_B 17

// Photo resistor
#define CDS_PIN 34 // ANALOG_PIN_0

// Audio out
#define AUDIO_PIN 26

// TF Card
#define TF_PIN_CS 5
#define TS_PIN_MOSI 23
#define TF_PIN_SCLK 18
#define TF_PIN_MISC 19