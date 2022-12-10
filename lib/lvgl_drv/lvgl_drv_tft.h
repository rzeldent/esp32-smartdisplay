#pragma once

#include <lvgl.h>

enum lvgl_tft_orientation
{
    portrait = 0,          // 0 degrees
    portrait_inverted = 1, // 90 degrees
    landscape = 2,         // 180 degrees
    landscape_inverted = 3 // 270 degrees
};

extern void lvgl_tft_init();
extern void lvgl_tft_flush(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_map);

extern void lvgl_tft_set_orientation(enum lvgl_tft_orientation orientation);
extern void lvgl_tft_set_backlight(uint8_t percent);
extern void vgl_tft_sleep();
extern void vgl_tft_wake();

// ST7796
// #define LVGL_TFT_ST7796
// LCD
// #define TFT_PIN_DC 2    // Data or Command
// #define TFT_PIN_BL 27   // Backlight
// #define TFT_WIDTH 320
// #define TFT_HEIGHT 480
// SPI
// #define TFT_PIN_SCLK 14
// #define TFT_PIN_MOSI 13
// #define TFT_PIN_MISO 12
// #define TFT_PIN_CS 15
// #define SPI_FREQ 16000000