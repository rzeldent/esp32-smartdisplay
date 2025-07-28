#ifndef ESP32_SMARTDISPLAY_H
#define ESP32_SMARTDISPLAY_H

#include <Arduino.h>
#include <lvgl.h>
#include <display/lv_display_private.h>
#include <misc/lv_timer_private.h>
#include <indev/lv_indev_private.h>

// Use last PWM_CHANNEL for backlight
#define PWM_CHANNEL_BCKL (SOC_LEDC_CHANNEL_NUM - 1)
#define PWM_FREQ_BCKL 400
#define PWM_BITS_BCKL 8
#define PWM_MAX_BCKL ((1 << PWM_BITS_BCKL) - 1)

// Exported functions
#ifdef __cplusplus
extern "C"
{
#endif
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

    // Initialize the display and touch
    void smartdisplay_init();
#ifdef BOARD_HAS_TOUCH
    // Touch calibration
    extern touch_calibration_data_t touch_calibration_data;
    touch_calibration_data_t smartdisplay_compute_touch_calibration(const lv_point_t screen[3], const lv_point_t touch[3]);
#endif    
    // Set the brightness of the backlight display
    void smartdisplay_lcd_set_backlight(float duty); // [0, 1]
    // Set the adaptive brightness callback
    typedef float (*smartdisplay_lcd_adaptive_brightness_cb_t)();
    void smartdisplay_lcd_set_brightness_cb(smartdisplay_lcd_adaptive_brightness_cb_t cb, uint32_t interval);
#ifdef BOARD_HAS_CDS
    // Read CdS sensor and return a value for the screen brightness (to be used in smartdisplay_lcd_set_brightness_cb)
    float smartdisplay_lcd_adaptive_brightness_cds();
#endif    
#ifdef BOARD_HAS_RGB_LED
    void smartdisplay_led_set_rgb(bool r, bool g, bool b);
#endif
#ifdef __cplusplus
}
#endif

#endif