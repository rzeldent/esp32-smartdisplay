#pragma once

#include <Arduino.h>
#include <lvgl.h>

#include <hal/gpio_types.h>

// Push button
#define PUSHBUTTON_BOOT 0
// TF Card
#define HAS_TF_CARD
#define TF_PIN_CS 5
#define TF_PIN_MOSI 23
#define TF_PIN_SCLK 18
#define TF_PIN_MISO 19

// LVGL lines buffered
#define LVGL_PIXEL_BUFFER_LINES 16

// Backlight PWM
#define PWM_FREQ_BCKL 5000
#define PWM_BITS_BCKL 8
#define PWM_MAX_BCKL ((1 << PWM_BITS_BCKL) - 1)

// ESP32_2432S024 N/R/C
#if defined(ESP32_2432S024N) || defined(ESP32_2432S024R) || defined(ESP32_2432S024C)
#define TFT_WIDTH 240
#define TFT_HEIGHT 320
// Backlight
#define PIN_BCKL 21
#define PWM_CHANNEL_BCKL 12
// LCD
#define USES_ILI9341
#include <driver/spi_common.h>
#include <esp_lcd_panel_io.h>
#include <esp_lcd_panel_vendor.h>
#define ILI9341_SPI_HOST SPI2_HOST
constexpr spi_bus_config_t ili9341_bus_config = {
    .mosi_io_num = 13,
    .miso_io_num = 12,
    .sclk_io_num = 14,
    .quadwp_io_num = -1,
    .quadhd_io_num = -1};
constexpr esp_lcd_panel_io_spi_config_t ili9341_io_spi_config = {
    .cs_gpio_num = 15,
    .dc_gpio_num = 2,
    .spi_mode = SPI_MODE0,
    .pclk_hz = 24000000,
    .trans_queue_depth = 10,
    .lcd_cmd_bits = 8,
    .lcd_param_bits = 8};
constexpr esp_lcd_panel_dev_config_t ili9341_panel_dev_config = {
    .reset_gpio_num = -1,
    .color_space = ESP_LCD_COLOR_SPACE_BGR,
    .bits_per_pixel = 16};
// Touch
#ifdef ESP32_2432S024R
#define USES_XPT2046
#include <driver/spi_master.h>
#include <esp_lcd_touch.h>
#define XPT2046_SPI_HOST SPI3_HOST
constexpr spi_bus_config_t xpt2046_spi_bus_config = {
    .mosi_io_num = 32,
    .miso_io_num = 39,
    .sclk_io_num = 25,
    .quadwp_io_num = -1,
    .quadhd_io_num = -1};
constexpr esp_lcd_panel_io_spi_config_t xpt2046_io_spi_config = {
    .cs_gpio_num = 33,
    .dc_gpio_num = -1,
    .spi_mode = SPI_MODE0,
    .pclk_hz = 2000000,
    .trans_queue_depth = 3,
    .lcd_cmd_bits = 8,
    .lcd_param_bits = 8};
constexpr esp_lcd_touch_config_t xpt2046_touch_config = {
    .x_max = TFT_WIDTH,
    .y_max = TFT_HEIGHT,
    .rst_gpio_num = GPIO_NUM_NC,
    .int_gpio_num = GPIO_NUM_36};
#else
#ifdef ESP32_2432S024C
#define USES_CST816S
#include <driver/i2c.h>
#include "esp_lcd_touch_cst816s.h"
constexpr i2c_config_t cst816s_i2c_config = {
    .mode = I2C_MODE_MASTER,
    .sda_io_num = 33,
    .scl_io_num = 32,
    .sda_pullup_en = GPIO_PULLUP_ENABLE,
    .scl_pullup_en = GPIO_PULLUP_ENABLE,
    .master = {
        .clk_speed = 400000}};
constexpr esp_lcd_panel_io_i2c_config_t cst816s_io_i2c_config = {
    .dev_addr = ESP_LCD_TOUCH_IO_I2C_CST816S_ADDRESS,
    .control_phase_bytes = 1,
    .dc_bit_offset = 0,
    .lcd_cmd_bits = 8,
    .flags = {
        .disable_control_phase = 1,
    }};
constexpr esp_lcd_touch_config_t cst816s_touch_config = {
    .x_max = TFT_WIDTH,
    .y_max = TFT_HEIGHT,
    .rst_gpio_num = GPIO_NUM_25,
    .int_gpio_num = GPIO_NUM_NC};
#endif
#endif
// Build in RGB LED
#define HAS_RGB_LED
#define LED_PIN_R 4
#define LED_PIN_G 16
#define LED_PIN_B 17
// Photo resistor
#define HAS_LIGHTSENSOR
#define LIGHTSENSOR_IN 34 // ANALOG_PIN_0
// Audio out
#define HAS_SPEAKER
#define SPEAKER_PIN 26
#endif

// ESP32_2432S028 R
#ifdef ESP32_2432S028R
#define TFT_WIDTH 240
#define TFT_HEIGHT 320
// Backlight
#define PIN_BCKL 21
#define PWM_CHANNEL_BCKL 12
// LCD
#define USES_ILI9341
#include <driver/spi_common.h>
#include <esp_lcd_panel_io.h>
#include <esp_lcd_panel_vendor.h>
#define ILI9341_SPI_HOST SPI2_HOST
constexpr spi_bus_config_t ili9341_bus_config = {
    .mosi_io_num = 13,
    .miso_io_num = 12,
    .sclk_io_num = 14,
    .quadwp_io_num = -1,
    .quadhd_io_num = -1};
constexpr esp_lcd_panel_io_spi_config_t ili9341_io_spi_config = {
    .cs_gpio_num = 15,
    .dc_gpio_num = 2,
    .spi_mode = SPI_MODE0,
    .pclk_hz = 24000000,
    .trans_queue_depth = 10,
    .lcd_cmd_bits = 8,
    .lcd_param_bits = 8};
constexpr esp_lcd_panel_dev_config_t ili9341_panel_dev_config = {
    .reset_gpio_num = -1,
    .color_space = ESP_LCD_COLOR_SPACE_BGR,
    .bits_per_pixel = 16};
// Touch
#define USES_XPT2046
#include <driver/spi_common.h>
#include <esp_lcd_touch.h>
#define XPT2046_SPI_HOST SPI3_HOST
constexpr spi_bus_config_t xpt2046_spi_bus_config = {
    .mosi_io_num = 32,
    .miso_io_num = 39,
    .sclk_io_num = 25,
    .quadwp_io_num = -1,
    .quadhd_io_num = -1};
constexpr esp_lcd_panel_io_spi_config_t xpt2046_io_spi_config = {
    .cs_gpio_num = 33,
    .dc_gpio_num = -1,
    .spi_mode = SPI_MODE0,
    .pclk_hz = 2000000,
    .trans_queue_depth = 3,
    .lcd_cmd_bits = 8,
    .lcd_param_bits = 8};
constexpr esp_lcd_touch_config_t xpt2046_touch_config = {
    .x_max = TFT_WIDTH,
    .y_max = TFT_HEIGHT,
    .rst_gpio_num = GPIO_NUM_NC,
    .int_gpio_num = GPIO_NUM_36};
// Build in RGB LED
#define HAS_RGB_LED
#define LED_PIN_R 4
#define LED_PIN_G 16
#define LED_PIN_B 17
// Photo resistor
#define HAS_LIGHTSENSOR
#define LIGHTSENSOR_IN 34 // ANALOG_PIN_0
// Audio out
#define HAS_SPEAKER
#define SPEAKER_PIN 26
#endif

// ESP32_3248S035 R/C
#if defined(ESP32_3248S035R) || defined(ESP32_3248S035C)
#define TFT_WIDTH 320
#define TFT_HEIGHT 480
// Backlight
#define PIN_BCKL 27
#define PWM_CHANNEL_BCKL 12
// LCD
#define USES_ST7796
#include <driver/spi_common.h>
#include <esp_lcd_panel_io.h>
#include <esp_lcd_panel_vendor.h>
#define ST7796_SPI_HOST SPI2_HOST
constexpr spi_bus_config_t st7796_bus_config = {
    .mosi_io_num = 13,
    .miso_io_num = 12,
    .sclk_io_num = 14,
    .quadwp_io_num = -1,
    .quadhd_io_num = -1};
constexpr esp_lcd_panel_io_spi_config_t st7796_spi_bus_config = {
    .cs_gpio_num = 15,
    .dc_gpio_num = 2,
    .spi_mode = SPI_MODE0,
    .pclk_hz = 24000000,
    .trans_queue_depth = 10,
    .lcd_cmd_bits = 8,
    .lcd_param_bits = 8};
constexpr esp_lcd_panel_dev_config_t panel_dev_config = {
    .reset_gpio_num = -1,
    .color_space = ESP_LCD_COLOR_SPACE_BGR,
    .bits_per_pixel = 16};
// Touch
#ifdef ESP32_3248S035R
#define USES_XPT2046
#include <driver/spi_common.h>
#include <esp_lcd_touch.h>
#define XPT2046_SPI_HOST SPI2_HOST
// Do not initialize the bus; already done by the ST7796
constexpr spi_bus_config_t xpt2046_spi_bus_config = {
    .mosi_io_num = 13,
    .miso_io_num = 12,
    .sclk_io_num = 14,
    .quadwp_io_num = -1,
    .quadhd_io_num = -1};
constexpr esp_lcd_panel_io_spi_config_t xpt2046_io_spi_config = {
    .cs_gpio_num = 33,
    .dc_gpio_num = -1,
    .spi_mode = SPI_MODE0,
    .pclk_hz = 2000000,
    .trans_queue_depth = 3,
    .lcd_cmd_bits = 8,
    .lcd_param_bits = 8};
constexpr esp_lcd_touch_config_t xpt2046_touch_config = {
    .x_max = TFT_WIDTH,
    .y_max = TFT_HEIGHT,
    .rst_gpio_num = GPIO_NUM_NC,
    .int_gpio_num = GPIO_NUM_36};
#else
#ifdef ESP32_3248S035C
#define USES_GT911
#include <driver/i2c.h>
#include "esp_lcd_touch_gt911.h"
const i2c_config_t gt911_i2c_config = {
    .mode = I2C_MODE_MASTER,
    .sda_io_num = 33,
    .scl_io_num = 32,
    .sda_pullup_en = GPIO_PULLUP_ENABLE,
    .scl_pullup_en = GPIO_PULLUP_ENABLE,
    .master = {
        .clk_speed = 400000}};
constexpr esp_lcd_panel_io_i2c_config_t gt911_io_i2c_config = {
    .dev_addr = ESP_LCD_TOUCH_IO_I2C_GT911_ADDRESS,
    .control_phase_bytes = 1,
    .dc_bit_offset = 0,
    .lcd_cmd_bits = 16,
    .flags = {
        .disable_control_phase = 1,
    }};
constexpr esp_lcd_touch_config_t gt911_touch_config = {
    .x_max = TFT_WIDTH,
    .y_max = TFT_HEIGHT,
    .rst_gpio_num = GPIO_NUM_25,
    .int_gpio_num = GPIO_NUM_NC};
#endif
#endif
// Build in RGB LED
#define HAS_RGB_LED
#define LED_PIN_R 4
#define LED_PIN_G 16
#define LED_PIN_B 17
// Photo resistor
#define HAS_LIGHTSENSOR
#define LIGHTSENSOR_IN 34 // ANALOG_PIN_0
// Audio out
#define HAS_SPEAKER
#define SPEAKER_PIN 26
#endif

// ESP32_4827S043 N/R/C
#if defined(ESP32_4827S043N) || defined(ESP32_4827S043R) || defined(ESP32_4827S043C)
#define TFT_WIDTH 480
#define TFT_HEIGHT 272
// Backlight
#define PIN_BCKL 2
#define PWM_CHANNEL_BCKL 7
// LCD ILI6485 480x272
#define USES_LCD_RGB
#include <esp_lcd_panel_rgb.h>
constexpr esp_lcd_rgb_panel_config_t esp_lcd_rgb_panel_config = {
    .clk_src = LCD_CLK_SRC_PLL160M,
    .timings = {
        .pclk_hz = 9000000,
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
    .data_width = 16, // R5G6B5
    .sram_trans_align = 8,
    .hsync_gpio_num = 39,
    .vsync_gpio_num = 41,
    .de_gpio_num = 40,
    .pclk_gpio_num = 42,
    .data_gpio_nums = {8, 3, 46, 9, 1, 5, 6, 7, 15, 16, 4, 45, 48, 47, 21, 14},
    .disp_gpio_num = -1,
    .flags = {.disp_active_low = 0, .relax_on_idle = 0, .fb_in_psram = 0}};
// Touch
#ifdef ESP32_4827S043R
#define USES_XPT2046
#include <driver/spi_common.h>
#include <esp_lcd_touch.h>
#define XPT2046_SPI_HOST SPI2_HOST
constexpr spi_bus_config_t xpt2046_spi_bus_config = {
    .mosi_io_num = 11,
    .miso_io_num = 13,
    .sclk_io_num = 12,
    .quadwp_io_num = -1,
    .quadhd_io_num = -1};
constexpr esp_lcd_panel_io_spi_config_t xpt2046_io_spi_config = {
    .cs_gpio_num = 38,
    .dc_gpio_num = -1,
    .spi_mode = SPI_MODE0,
    .pclk_hz = 2000000,
    .trans_queue_depth = 3,
    .lcd_cmd_bits = 8,
    .lcd_param_bits = 8};
constexpr esp_lcd_touch_config_t xpt2046_touch_config = {
    .x_max = TFT_WIDTH,
    .y_max = TFT_HEIGHT,
    .rst_gpio_num = GPIO_NUM_NC,
    .int_gpio_num = GPIO_NUM_18};
#else
#ifdef ESP32_4827S043C
#define USES_GT911
#include <driver/i2c.h>
#include "esp_lcd_touch_gt911.h"
const i2c_config_t gt911_i2c_config = {
    .mode = I2C_MODE_MASTER,
    .sda_io_num = 19,
    .scl_io_num = 20,
    .sda_pullup_en = GPIO_PULLUP_ENABLE,
    .scl_pullup_en = GPIO_PULLUP_ENABLE,
    .master = {
        .clk_speed = 400000}};
constexpr esp_lcd_panel_io_i2c_config_t gt911_io_i2c_config = {
    .dev_addr = ESP_LCD_TOUCH_IO_I2C_GT911_ADDRESS,
    .control_phase_bytes = 1,
    .dc_bit_offset = 0,
    .lcd_cmd_bits = 16,
    .flags = {
        .disable_control_phase = 1,
    }};
constexpr esp_lcd_touch_config_t gt911_touch_config = {
    .x_max = TFT_WIDTH,
    .y_max = TFT_HEIGHT,
    .rst_gpio_num = GPIO_NUM_38,
    .int_gpio_num = GPIO_NUM_NC};
#endif
#endif
#endif

// ESP32_8048S043 N/R/C
#if defined(ESP32_8048S043N) || defined(ESP32_8048S043R) || defined(ESP32_8048S043C)
#define TFT_WIDTH 800
#define TFT_HEIGHT 480
// Backlight
#define PIN_BCKL 2
#define PWM_CHANNEL_BCKL 7
// LCD 800x480
#define USES_LCD_RGB
#include <esp_lcd_panel_rgb.h>
constexpr esp_lcd_rgb_panel_config_t esp_lcd_rgb_panel_config = {
    .clk_src = LCD_CLK_SRC_PLL160M,
    .timings = {
        .pclk_hz = 8000000,
        .h_res = TFT_WIDTH,
        .v_res = TFT_HEIGHT,
        .hsync_pulse_width = 4,
        .hsync_back_porch = 8,
        .hsync_front_porch = 8,
        .vsync_pulse_width = 4,
        .vsync_back_porch = 8,
        .vsync_front_porch = 8,
        .flags = {
            .hsync_idle_low = 1,
            .vsync_idle_low = 1,
            .de_idle_high = 0,
            .pclk_active_neg = 1,
            .pclk_idle_high = 0,
        }},
    .data_width = 16, // R5G6B5
    .sram_trans_align = 8,
    .hsync_gpio_num = 39,
    .vsync_gpio_num = 41,
    .de_gpio_num = 40,
    .pclk_gpio_num = 42,
    .data_gpio_nums = {8, 3, 46, 9, 1, 5, 6, 7, 15, 16, 4, 45, 48, 47, 21, 14},
    .disp_gpio_num = -1,
    .flags = {.disp_active_low = 0, .relax_on_idle = 0, .fb_in_psram = 0}};
// Touch
#ifdef ESP32_8048S043R
#define USES_XPT2046
#include <driver/spi_common.h>
#include <esp_lcd_touch.h>
#define XPT2046_SPI_HOST SPI2_HOST
constexpr spi_bus_config_t xpt2046_spi_bus_config = {
    .mosi_io_num = 11,
    .miso_io_num = 13,
    .sclk_io_num = 12,
    .quadwp_io_num = -1,
    .quadhd_io_num = -1};
constexpr esp_lcd_panel_io_spi_config_t xpt2046_io_spi_config = {
    .cs_gpio_num = 38,
    .dc_gpio_num = -1,
    .spi_mode = SPI_MODE0,
    .pclk_hz = 2000000,
    .trans_queue_depth = 3,
    .lcd_cmd_bits = 8,
    .lcd_param_bits = 8};
constexpr esp_lcd_touch_config_t xpt2046_touch_config = {
    .x_max = TFT_WIDTH,
    .y_max = TFT_HEIGHT,
    .rst_gpio_num = GPIO_NUM_NC,
    .int_gpio_num = GPIO_NUM_18};
#else
#ifdef ESP32_8048S043C
#define USES_GT911
#include <driver/i2c.h>
#include "esp_lcd_touch_gt911.h"
const i2c_config_t gt911_i2c_config = {
    .mode = I2C_MODE_MASTER,
    .sda_io_num = 19,
    .scl_io_num = 20,
    .sda_pullup_en = GPIO_PULLUP_ENABLE,
    .scl_pullup_en = GPIO_PULLUP_ENABLE,
    .master = {
        .clk_speed = 400000}};
constexpr esp_lcd_panel_io_i2c_config_t gt911_io_i2c_config = {
    .dev_addr = ESP_LCD_TOUCH_IO_I2C_GT911_ADDRESS,
    .control_phase_bytes = 1,
    .dc_bit_offset = 0,
    .lcd_cmd_bits = 16,
    .flags = {
        .disable_control_phase = 1,
    }};
constexpr esp_lcd_touch_config_t gt911_touch_config = {
    .x_max = TFT_WIDTH,
    .y_max = TFT_HEIGHT,
    .rst_gpio_num = GPIO_NUM_38,
    .int_gpio_num = GPIO_NUM_NC};
#endif
#endif
#endif

// ESP32_8048S050 N/R/C
#if defined(ESP32_8048S050N) || defined(ESP32_8048S050R) || defined(ESP32_8048S050C)
#define TFT_WIDTH 800
#define TFT_HEIGHT 480
// Backlight
#define PIN_BCKL 2
#define PWM_CHANNEL_BCKL 7
// LCD 800x480
#define USES_LCD_RGB
#include <esp_lcd_panel_rgb.h>
constexpr esp_lcd_rgb_panel_config_t esp_lcd_rgb_panel_config = {
    .clk_src = LCD_CLK_SRC_PLL160M,
    .timings = {
        .pclk_hz = 16000000,
        .h_res = TFT_WIDTH,
        .v_res = TFT_HEIGHT,
        .hsync_pulse_width = 4,
        .hsync_back_porch = 8,
        .hsync_front_porch = 8,
        .vsync_pulse_width = 4,
        .vsync_back_porch = 8,
        .vsync_front_porch = 8,
        .flags = {
            .hsync_idle_low = 1,
            .vsync_idle_low = 1,
            .de_idle_high = 0,
            .pclk_active_neg = 1,
            .pclk_idle_high = 0,
        }},
    .data_width = 16, // R5G6B5
    .sram_trans_align = 8,
    .hsync_gpio_num = 39,
    .vsync_gpio_num = 41,
    .de_gpio_num = 40,
    .pclk_gpio_num = 42,
    .data_gpio_nums = {8, 3, 46, 9, 1, 5, 6, 7, 15, 16, 4, 45, 48, 47, 21, 14},
    .disp_gpio_num = -1,
    .flags = {.disp_active_low = 0, .relax_on_idle = 0, .fb_in_psram = 0}};
// Touch
#ifdef ESP32_8048S050R
#define USES_XPT2046
#include <driver/spi_common.h>
#include <esp_lcd_touch.h>
#define XPT2046_SPI_HOST SPI2_HOST
constexpr spi_bus_config_t xpt2046_spi_bus_config = {
    .mosi_io_num = 11,
    .miso_io_num = 13,
    .sclk_io_num = 12,
    .quadwp_io_num = -1,
    .quadhd_io_num = -1};
constexpr esp_lcd_panel_io_spi_config_t xpt2046_io_spi_config = {
    .cs_gpio_num = 38,
    .dc_gpio_num = -1,
    .spi_mode = SPI_MODE0,
    .pclk_hz = 2000000,
    .trans_queue_depth = 3,
    .lcd_cmd_bits = 8,
    .lcd_param_bits = 8};
constexpr esp_lcd_touch_config_t xpt2046_touch_config = {
    .x_max = TFT_WIDTH,
    .y_max = TFT_HEIGHT,
    .rst_gpio_num = GPIO_NUM_NC,
    .int_gpio_num = GPIO_NUM_18};
#else
#ifdef ESP32_8048S050C
#define USES_GT911
#include <driver/i2c.h>
#include "esp_lcd_touch_gt911.h"
const i2c_config_t gt911_i2c_config = {
    .mode = I2C_MODE_MASTER,
    .sda_io_num = 19,
    .scl_io_num = 20,
    .sda_pullup_en = GPIO_PULLUP_ENABLE,
    .scl_pullup_en = GPIO_PULLUP_ENABLE,
    .master = {
        .clk_speed = 400000}};
constexpr esp_lcd_panel_io_i2c_config_t gt911_io_i2c_config = {
    .dev_addr = ESP_LCD_TOUCH_IO_I2C_GT911_ADDRESS,
    .control_phase_bytes = 1,
    .dc_bit_offset = 0,
    .lcd_cmd_bits = 16,
    .flags = {
        .disable_control_phase = 1,
    }};
constexpr esp_lcd_touch_config_t gt911_touch_config = {
    .x_max = TFT_WIDTH,
    .y_max = TFT_HEIGHT,
    .rst_gpio_num = GPIO_NUM_38,
    .int_gpio_num = GPIO_NUM_NC};
#endif
#endif
#endif

// ESP32_8048S070 N/R/C
#if defined(ESP32_8048S070N) || defined(ESP32_8048S070R) || defined(ESP32_8048S070C)
#define TFT_WIDTH 800
#define TFT_HEIGHT 480
// Backlight
#define PIN_BCKL 2
#define PWM_CHANNEL_BCKL 7
// LCD 800x480
#define USES_LCD_RGB
#include <esp_lcd_panel_rgb.h>
constexpr esp_lcd_rgb_panel_config_t esp_lcd_rgb_panel_config = {
    .clk_src = LCD_CLK_SRC_PLL160M,
    .timings = {
        .pclk_hz = 12000000,
        .h_res = TFT_WIDTH,
        .v_res = TFT_HEIGHT,
        .hsync_pulse_width = 30,
        .hsync_back_porch = 16,
        .hsync_front_porch = 210,
        .vsync_pulse_width = 13,
        .vsync_back_porch = 10,
        .vsync_front_porch = 22,
        .flags = {
            .hsync_idle_low = 1,
            .vsync_idle_low = 1,
            .de_idle_high = 0,
            .pclk_active_neg = 1,
            .pclk_idle_high = 0,
        }},
    .data_width = 16, // R5G6B5
    .sram_trans_align = 8,
    .hsync_gpio_num = 39,
    .vsync_gpio_num = 40,
    .de_gpio_num = 41,
    .pclk_gpio_num = 42,
    .data_gpio_nums = {15, 7, 6, 5, 4, 9, 46, 3, 8, 16, 1, 14, 21, 47, 48, 45},
    .disp_gpio_num = -1,
    .flags = {.disp_active_low = 0, .relax_on_idle = 0, .fb_in_psram = 0}};
// Touch
#ifdef ESP32_8048S070R
#define USES_XPT2046
#include <driver/spi_common.h>
#include <esp_lcd_touch.h>
#define XPT2046_SPI_HOST SPI2_HOST
constexpr spi_bus_config_t xpt2046_spi_bus_config = {
    .mosi_io_num = 11,
    .miso_io_num = 13,
    .sclk_io_num = 12,
    .quadwp_io_num = -1,
    .quadhd_io_num = -1};
constexpr esp_lcd_panel_io_spi_config_t xpt2046_io_spi_config = {
    .cs_gpio_num = 38,
    .dc_gpio_num = -1,
    .spi_mode = SPI_MODE0,
    .pclk_hz = 2000000,
    .trans_queue_depth = 3,
    .lcd_cmd_bits = 8,
    .lcd_param_bits = 8};
constexpr esp_lcd_touch_config_t xpt2046_touch_config = {
    .x_max = TFT_WIDTH,
    .y_max = TFT_HEIGHT,
    .rst_gpio_num = GPIO_NUM_NC,
    .int_gpio_num = GPIO_NUM_18};
#else
#ifdef ESP32_8048S070C
#define USES_GT911
#include <driver/i2c.h>
#include "esp_lcd_touch_gt911.h"
const i2c_config_t gt911_i2c_config = {
    .mode = I2C_MODE_MASTER,
    .sda_io_num = 19,
    .scl_io_num = 20,
    .sda_pullup_en = GPIO_PULLUP_ENABLE,
    .scl_pullup_en = GPIO_PULLUP_ENABLE,
    .master = {
        .clk_speed = 400000}};
constexpr esp_lcd_panel_io_i2c_config_t gt911_io_i2c_config = {
    .dev_addr = ESP_LCD_TOUCH_IO_I2C_GT911_ADDRESS,
    .control_phase_bytes = 1,
    .dc_bit_offset = 0,
    .lcd_cmd_bits = 16,
    .flags = {
        .disable_control_phase = 1,
    }};
constexpr esp_lcd_touch_config_t gt911_touch_config = {
    .x_max = TFT_WIDTH,
    .y_max = TFT_HEIGHT,
    .rst_gpio_num = GPIO_NUM_38,
    .int_gpio_num = GPIO_NUM_NC};
#endif
#endif
#endif

// Initialize the display and touch
extern void smartdisplay_init();
// Set the brightness of the backlight display
extern void smartdisplay_tft_set_backlight(uint16_t duty); // 0-1023 (12 bits)