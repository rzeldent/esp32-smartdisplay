#pragma once

#include <lvgl.h>

// Color definitions RGB565
#define RGB565_BLACK 0x0000
#define RGB565_BLUE 0x001F
#define RGB565_RED 0xF800
#define RGB565_GREEN 0x07E0
#define RGB565_CYAN 0x07FF
#define RGB565_MAGENTA 0xF81F
#define RGB565_YELLOW 0xFFE0
#define RGB565_WHITE 0xFFFF

extern void lvgl_init();
extern void lvgl_tft_set_backlight(uint8_t value);  // 0-255
extern void vgl_tft_sleep();
extern void vgl_tft_wake();

// ST7796
// #define LVGL_TFT_ST7796
// LCD
// #define TFT_PIN_DC 2    // Data or Command
// #define TFT_PIN_BL 27   // Backlight
// #define TFT_WIDTH 320
// #define TFT_HEIGHT 480
// define TFT_ORIENTATION_PORTRAIT / TFT_ORIENTATION_LANDSCAPE / TFT_ORIENTATION_PORTRAIT_INV / TFT_ORIENTATION_LANDSCAPE_INV
// SPI
// #define TFT_PIN_SCLK 14
// #define TFT_PIN_MOSI 13
// #define TFT_PIN_MISO 12
// #define TFT_PIN_CS 15
// #define SPI_FREQ 16000000