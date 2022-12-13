#pragma once

#include <lvgl.h>

extern void lvgl_init();
extern void lvgl_tft_set_backlight(uint8_t value);  // 0-255
extern void lvgl_tft_sleep();
extern void lvgl_tft_wake();

// ST7796
// #define LVGL_TFT_ST7796
/*
    # LCD
    -D LVGL_TFT_ST7796
    -D TFT_WIDTH=320
    -D TFT_HEIGHT=480
    -D TFT_ORIENTATION_PORTRAIT
    # TFT ST7796 SPI
    -D TFT_PIN_SCLK=14
    -D TFT_PIN_MOSI=13
    -D TFT_PIN_MISO=12
    -D TFT_PIN_CS=15
    -D TFT_SPI_FREQ=80000000
    # TFT Panel
    -D TFT_PIN_DC=2
    # TFT backlight
    -D TFT_PIN_BL=27
*/