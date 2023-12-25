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
#define PWM_BITS_BCKL 13
#define PWM_MAX_BCKL ((1 << PWM_BITS_BCKL) - 1)

// Structure to store the data from the three point calibration data
typedef struct
{
    bool valid;
    float alphaX;
    float betaX;
    float deltaX;
    float alphaY;
    float betaY;
    float deltaY;
} touch_calibration_data_t;

// Exported functions
#ifdef __cplusplus
extern "C"
{
#endif
    // Initialize the display and touch
    void smartdisplay_init();
    // Set the brightness of the backlight display
    void smartdisplay_lcd_set_backlight(float duty); // [0, 1]
    // Set the brightness automatically based on the CdS sensor
#ifdef BOARD_HAS_CDS
    void smartdisplay_lcd_set_auto_brightness(bool enable);
#endif
#ifdef BOARD_HAS_RGB_LED
    void smartdisplay_led_set_rgb(bool r, bool g, bool b);
#endif
#ifdef BOARD_HAS_TOUCH
    // Touch calibration
    extern touch_calibration_data_t smartdisplay_touch_calibration;
    void smartdisplay_compute_touch_calibration(const lv_point_t screen[3], const lv_point_t touch[3]);
    void smartdisplay_touch_calibrate();
#endif

#ifdef __cplusplus
}
#endif

#endif