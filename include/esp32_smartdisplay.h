#ifndef ESP32_SMARTDISPLAY_H
#define ESP32_SMARTDISPLAY_H

#include <Arduino.h>
#include <lvgl.h>

// LVGL lines buffered
#define LVGL_PIXEL_BUFFER_LINES 16

// Backlight PWM
// Use last PWM_CHANNEL
#define PWM_CHANNEL_BCKL (SOC_LEDC_CHANNEL_NUM - 1)
#define PWM_FREQ_BCKL 5000
#define PWM_BITS_BCKL 8
#define PWM_MAX_BCKL ((1 << PWM_BITS_BCKL) - 1)

// Exported functions
#ifdef __cplusplus
extern "C"
{
#endif
    // Initialize the display and touch
    void smartdisplay_init();
    // Set the brightness of the backlight display
    void smartdisplay_tft_set_backlight(float duty); // [0, 1]
#ifdef __cplusplus
}
#endif

#endif