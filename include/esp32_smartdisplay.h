#pragma once

#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <esp_lcd_panel_rgb.h>

#include <lvgl.h>

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
#define HAS_ILI9431
#define TFT_SPI_SCLK 14
#define TFT_SPI_MOSI 13
#define TFT_SPI_MISO 12
#define ILI9341_PIN_CS 15
#define ILI9341_PIN_DC 2
#define ILI9341_SPI_FREQ 50000000
#define ILI9341_PIN_BL 21
#define PWM_CHANNEL_BL 12
#define PWM_FREQ_BL 5000
#define PWM_BITS_BL 8
#define PWM_MAX_BL ((1 << PWM_BITS_BL) - 1)
// Push button
#define PUSHBUTTON_BOOT 0
// Build in RGB LED
#define HAS_RGB_LED
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
#define HAS_LIGHTSENSOR
#define LIGHTSENSOR_IN 34 // ANALOG_PIN_0
// Audio out
#define HAS_SPEAKER
#define SPEAKER_PIN 26
// TF Card
#define HAS_TF_CARD
#define TF_PIN_CS 5
#define TF_PIN_MOSI 23
#define TF_PIN_SCLK 18
#define TF_PIN_MISO 19
#endif

// ESP32_2432S024R
#ifdef ESP32_2432S024R
// Touch
#define HAS_XPT2046
#define TOUCH_SPI_SCLK 25
#define TOUCH_SPI_MOSI 32
#define TOUCH_SPI_MISO 39
#define TOUCH_SPI_FREQ 2000000
#define XPT2046_PIN_INT 36
#define XPT2046_PIN_CS 33
// Calibration 240x320
#define XPT2046_MIN_X 349
#define XPT2046_MAX_X 3859
#define XPT2046_MIN_Y 247
#define XPT2046_MAX_Y 3871
#endif

// ESP32_2432S024C
#ifdef ESP32_2432S024C
// Touch
#define HAS_CST820
#define TOUCH_IIC_SDA 33
#define TOUCH_IIC_SCL 32
#define TOUCH_IIC_RST 25
#endif

#ifdef ESP32_2432S028R
#define ESP32_SMARTDISPLAY_BOARD_VALID
#define TFT_WIDTH 240
#define TFT_HEIGHT 320
#define HAS_ILI9431
#define TFT_SPI_SCLK 14
#define TFT_SPI_MOSI 13
#define TFT_SPI_MISO 12
#define ILI9341_PIN_CS 15
#define ILI9341_PIN_DC 2
#define ILI9341_SPI_FREQ 50000000
#define ILI9341_PIN_BL 21
#define PWM_CHANNEL_BL 12
#define PWM_FREQ_BL 5000
#define PWM_BITS_BL 8
#define PWM_MAX_BL ((1 << PWM_BITS_BL) - 1)
// Push button
#define PUSHBUTTON_BOOT 0
// Build in RGB LED
#define HAS_RGB_LED
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
#define HAS_LIGHTSENSOR
#define LIGHTSENSOR_IN 34 // ANALOG_PIN_0
// Audio out
#define HAS_SPEAKER
#define SPEAKER_PIN 26
// TF Card
#define HAS_TF_CARD
#define TF_PIN_CS 5
#define TF_PIN_MOSI 23
#define TF_PIN_SCLK 18
#define TF_PIN_MISO 19
// Touch
#define HAS_XPT2046
#define TOUCH_SPI_SCLK 25
#define TOUCH_SPI_MOSI 32
#define TOUCH_SPI_MISO 39
#define TOUCH_SPI_FREQ 2000000
#define XPT2046_PIN_INT 36
#define XPT2046_PIN_CS 33
// Calibration 240x320
#define XPT2046_MIN_X 349
#define XPT2046_MAX_X 3859
#define XPT2046_MIN_Y 247
#define XPT2046_MAX_Y 3871
#endif

#if defined(ESP32_3248S035R) || defined(ESP32_3248S035C)
#define ESP32_SMARTDISPLAY_BOARD_VALID
#define TFT_WIDTH 320
#define TFT_HEIGHT 480
#define HAS_ST7796
#define TFT_SPI_SCLK 14
#define TFT_SPI_MOSI 13
#define TFT_SPI_MISO 12
#define ST7796_PIN_CS 15
#define ST7796_PIN_DC 2
#define TFT_SPI_FREQ 80000000
#define ST7796_PIN_BL 27
#define PWM_CHANNEL_BL 12
#define PWM_FREQ_BL 5000
#define PWM_BITS_BL 8
#define PWM_MAX_BL ((1 << PWM_BITS_BL) - 1)
// Push button
#define PUSHBUTTON_BOOT 0
// Build in RGB LED
#define HAS_RGB_LED
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
#define HAS_LIGHTSENSOR
#define LIGHTSENSOR_IN 34 // ANALOG_PIN_0
// Audio out
#define HAS_SPEAKER
#define SPEAKER_PIN 26
// TF Card
#define HAS_TF_CARD
#define TF_PIN_CS 5
#define TF_PIN_MOSI 23
#define TF_PIN_SCLK 18
#define TF_PIN_MISO 19
#endif

// ESP32_3248S035R
#ifdef ESP32_3248S035R
// Touch
#define HAS_XPT2046
#define TOUCH_SPI_SCLK 14
#define TOUCH_SPI_MOSI 13
#define TOUCH_SPI_MISO 12
#define TOUCH_SPI_FREQ 2000000
#define XPT2046_PIN_INT 36
#define XPT2046_PIN_CS 33
// Calibration 320x480
#define XPT2046_MIN_X 256
#define XPT2046_MAX_X 3860
#define XPT2046_MIN_Y 180
#define XPT2046_MAX_Y 3900
#endif

#ifdef ESP32_3248S035C
#define HAS_GT911
#define TOUCH_IIC_SDA 33
#define TOUCH_IIC_SCL 32
#define TOUCH_IIC_RST 25
#endif

#if defined(ESP32_4827S043N) || defined(ESP32_4827S043R) || defined(ESP32_4827S043C)
#define ESP32_SMARTDISPLAY_BOARD_VALID
#define TFT_WIDTH 480
#define TFT_HEIGHT 272
#define TFT_ESP_LCD
constexpr esp_lcd_rgb_panel_config_t tft_panel_config = {
    .clk_src = LCD_CLK_SRC_PLL160M,
    .timings = {
        .pclk_hz = 12000000L,
        .h_res = TFT_WIDTH,
        .v_res = TFT_HEIGHT,
        .hsync_pulse_width = 4,
        .hsync_back_porch = 43,
        .hsync_front_porch = 8,
        .vsync_pulse_width = 4,
        .vsync_back_porch = 12,
        .vsync_front_porch = 8,
        .flags = {
            .hsync_idle_low = 1,
            .vsync_idle_low = 1,
            .de_idle_high = 0,
            .pclk_active_neg = 1,
            .pclk_idle_high = 0,
        }},
    .data_width = 16, // RGB565
    .sram_trans_align = 8,
    .psram_trans_align = 64,
    .hsync_gpio_num = 39 /*HSYNC*/,
    .vsync_gpio_num = 41 /*VSYNC*/,
    .de_gpio_num = 40 /*DE*/,
    .pclk_gpio_num = 42 /*PCLK*/,
    .data_gpio_nums = {8 /*B0*/, 3 /*B1*/, 46 /*B2*/, 9 /*B3*/, 1 /*B4*/, 5 /*G0*/, 6 /*G1*/, 7 /*G2*/, 15 /*G3*/, 16 /*G4*/, 4 /*G5*/, 45 /*R0*/, 48 /*R1*/, 47 /*R2*/, 14 /*R4*/},
    .disp_gpio_num = GPIO_NUM_NC,
    .on_frame_trans_done = nullptr,
    .user_ctx = nullptr,
    .flags = {.disp_active_low = 0, .relax_on_idle = 0, .fb_in_psram = 1}};
#define PIN_BL 2
#define PWM_CHANNEL_BL 12
#define PWM_FREQ_BL 5000
#define PWM_BITS_BL 8
#define PWM_MAX_BL ((1 << PWM_BITS_BL) - 1)
// Push button
#define PUSHBUTTON_BOOT 0
// TF Card
#define TF_PIN_CS 10
#define TF_PIN_MOSI 11
#define TF_PIN_SCLK 12
#define TF_PIN_MISO 13
#endif

#ifdef ESP32_4827S043R
// Touch
#define HAS_XPT2046
#define TOUCH_SPI_SCLK 12
#define TOUCH_SPI_MOSI 11
#define TOUCH_SPI_MISO 13
#define TOUCH_SPI_FREQ 2000000
#define XPT2046_PIN_INT 18
#define XPT2046_PIN_CS 38
// Calibration
#define XPT2046_MIN_X 349
#define XPT2046_MAX_X 3859
#define XPT2046_MIN_Y 247
#define XPT2046_MAX_Y 3871
#endif

#ifdef ESP32_4827S043C
#define HAS_GT911
#define TOUCH_IIC_SDA 19
#define TOUCH_IIC_SCL 20
#define TOUCH_IIC_RST 38
#endif

#if defined(ESP32_8048S043N) || defined(ESP32_8048S043R) || defined(ESP32_8048S043C)
#define ESP32_SMARTDISPLAY_BOARD_VALID
#define TFT_WIDTH 800
#define TFT_HEIGHT 480
#define TFT_ESP_LCD
#define PIN_BL 2
#define PWM_CHANNEL_BL 12
#define PWM_FREQ_BL 5000
#define PWM_BITS_BL 8
#define PWM_MAX_BL ((1 << PWM_BITS_BL) - 1)
// Push button
#define PUSHBUTTON_BOOT 0
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
#define HAS_XPT2046
#define TOUCH_SPI_SCLK 12
#define TOUCH_SPI_MOSI 11
#define TOUCH_SPI_MISO 13
#define TOUCH_SPI_FREQ 2000000
#define XPT2046_PIN_INT 18
#define XPT2046_PIN_CS 38
// Calibration
#define XPT2046_MIN_X 349
#define XPT2046_MAX_X 3859
#define XPT2046_MIN_Y 247
#define XPT2046_MAX_Y 3871
#endif

#ifdef ESP32_8048S043C
#define HAS_GT911
#define TOUCH_IIC_SDA 19
#define TOUCH_IIC_SCL 20
#define TOUCH_IIC_RST 38
#endif

#if defined(ESP32_8048S050N) || defined(ESP32_8048S050R) || defined(ESP32_8048S050C)
#define ESP32_SMARTDISPLAY_BOARD_VALID
#define TFT_WIDTH 800
#define TFT_HEIGHT 480
#define TFT_ESP_LCD
#define PIN_BL 2
#define PWM_CHANNEL_BL 12
#define PWM_FREQ_BL 5000
#define PWM_BITS_BL 8
#define PWM_MAX_BL ((1 << PWM_BITS_BL) - 1)
// Push button
#define PUSHBUTTON_BOOT 0
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
#define HAS_XPT2046
#define TOUCH_SPI_SCLK 12
#define TOUCH_SPI_MOSI 11
#define TOUCH_SPI_MISO 13
#define TOUCH_SPI_FREQ 2000000
#define XPT2046_PIN_INT 18
#define XPT2046_PIN_CS 38
// Calibration
#define XPT2046_MIN_X 349
#define XPT2046_MAX_X 3859
#define XPT2046_MIN_Y 247
#define XPT2046_MAX_Y 3871
#endif

#ifdef ESP32_8048S050C
#define HAS_GT911
#define TOUCH_IIC_SDA 19
#define TOUCH_IIC_SCL 20
#define TOUCH_IIC_RST 38
#endif

#if defined(ESP32_8048S070N) || defined(ESP32_8048S070R) || defined(ESP32_8048S070C)
#define ESP32_SMARTDISPLAY_BOARD_VALID
#define TFT_WIDTH 800
#define TFT_HEIGHT 480
#define TFT_ESP_LCD
#define PIN_BL 2
#define PWM_CHANNEL_BL 12
#define PWM_FREQ_BL 5000
#define PWM_BITS_BL 8
#define PWM_MAX_BL ((1 << PWM_BITS_BL) - 1)
// Push button
#define PUSHBUTTON_BOOT 0
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
#define HAS_XPT2046
#define TOUCH_SPI_SCLK 12
#define TOUCH_SPI_MOSI 11
#define TOUCH_SPI_MISO 13
#define TOUCH_SPI_FREQ 2000000
#define XPT2046_PIN_INT 18
#define XPT2046_PIN_CS 38
// Calibration
#define XPT2046_MIN_X 349
#define XPT2046_MAX_X 3859
#define XPT2046_MIN_Y 247
#define XPT2046_MAX_Y 3871
#endif

#ifdef ESP32_8048S070C
#define HAS_GT911
#define TOUCH_IIC_SDA 19
#define TOUCH_IIC_SCL 20
#define TOUCH_IIC_RST 38
#endif

// Initialize the display and touch
extern void smartdisplay_init();
#ifdef HAS_RGB_LED
// Set the color of the led
extern void smartdisplay_set_led_color(lv_color32_t rgb);
#endif
#ifdef HAS_LIGHTSENSOR
// Get the value of the CDS sensor
extern int smartdisplay_get_light_intensity();
#endif
#ifdef HAS_SPEAKER
// Beep with the specified frequency and duration
extern void smartdisplay_beep(unsigned int frequency, unsigned long duration);
#endif
// Set the brightness of the backlight display
extern void smartdisplay_tft_set_backlight(uint16_t duty); // 0-1023 (12 bits)
// Put the display to sleep
extern void smartdisplay_tft_sleep();
// Wake the display
extern void smartdisplay_tft_wake();

// Buffer size for drawing
#define DRAW_BUFFER_SIZE (TFT_WIDTH * 10)
