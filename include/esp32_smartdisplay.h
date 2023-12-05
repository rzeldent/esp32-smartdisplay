#ifndef ESP32_SMARTDISPLAY_H
#define ESP32_SMARTDISPLAY_H

#include <Arduino.h>
#include <lvgl.h>

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
// Use last PWM_CHANNEL
#define PWM_CHANNEL_BCKL (SOC_LEDC_CHANNEL_NUM - 1)
#define PWM_FREQ_BCKL 5000
#define PWM_BITS_BCKL 8
#define PWM_MAX_BCKL ((1 << PWM_BITS_BCKL) - 1)

// ESP32_1732S019 N/C
#if defined(ESP32_1732S019N) || defined(ESP32_1732S019C)
#define TFT_WIDTH 170
#define TFT_HEIGHT 320
// Backlight
#define PIN_BCKL 14
// LCD
#define USES_ST7789
#define ST7789_SPI_HOST SPI2_HOST
#define ST7789_SPI_BUS_CONFIG {.mosi_io_num=13,.sclk_io_num=12,.quadwp_io_num=-1,.quadhd_io_num=-1}
#define ST7789_IO_SPI_CONFIG {.cs_gpio_num=10,.dc_gpio_num=11,.spi_mode=SPI_MODE0,.pclk_hz=24000000,.trans_queue_depth=10,.lcd_cmd_bits=8,.lcd_param_bits=8}
#define ST7789_PANEL_DEV_CONFIG {.reset_gpio_num=1,.color_space=ESP_LCD_COLOR_SPACE_RGB,.bits_per_pixel=16}
#define PANEL_ROT_NONE_SWAP_XY false
#define PANEL_ROT_NONE_MIRROR_X false
#define PANEL_ROT_NONE_MIRROR_Y false
#ifdef ESP32_1732S019C
#define USES_GT911
#include "esp_lcd_touch_gt911.h"
#define GT911_I2C_HOST 0
#define GT911_I2C_CONFIG {.mode=I2C_MODE_MASTER,.sda_io_num=9,.scl_io_num=46,.sda_pullup_en=GPIO_PULLUP_ENABLE,.scl_pullup_en=GPIO_PULLUP_ENABLE,.master={.clk_speed=400000}}
#define GT911_IO_I2C_CONFIG {.dev_addr=ESP_LCD_TOUCH_IO_I2C_GT911_ADDRESS,.control_phase_bytes=1,.lcd_cmd_bits=16,.flags={.disable_control_phase=1}}
#define GT911_TOUCH_CONFIG {.x_max=TFT_WIDTH,.y_max=TFT_HEIGHT,.rst_gpio_num=GPIO_NUM_3,.int_gpio_num=GPIO_NUM_8}
#define TOUCH_ROT_NONE_SWAP_X false
#define TOUCH_ROT_NONE_SWAP_Y false
#endif
#endif

// ESP32_2424S012 N/C
#if defined(ESP32_2424S012N) || defined(ESP32_2424S012C)
#define TFT_WIDTH 240
#define TFT_HEIGHT 240
// Backlight
#define PIN_BCKL 3
// LCD
#define USES_GC9A01
#define GC9A01_SPI_HOST SPI2_HOST
#define GC9A01_SPI_BUS_CONFIG {.mosi_io_num=7,.miso_io_num=-1,.sclk_io_num=6,.quadwp_io_num=-1,.quadhd_io_num=-1}
#define GC9A01_IO_SPI_CONFIG {.cs_gpio_num=10,.dc_gpio_num=2,.spi_mode=SPI_MODE0,.pclk_hz=80000000,.trans_queue_depth=10,.lcd_cmd_bits=8,.lcd_param_bits=8}
#define GC9A01_PANEL_DEV_CONFIG {.reset_gpio_num=-1,.color_space=ESP_LCD_COLOR_SPACE_BGR,.bits_per_pixel=16}
#define PANEL_ROT_NONE_SWAP_XY false
#define PANEL_ROT_NONE_MIRROR_X true
#define PANEL_ROT_NONE_MIRROR_Y false
// Touch
#ifdef ESP32_2424S012C
#define USES_CST816S
#include "esp_lcd_touch_cst816s.h"
#define CST816S_I2C_HOST 0
#define CST816S_I2C_CONFIG {.mode=I2C_MODE_MASTER,.sda_io_num=4,.scl_io_num=5,.sda_pullup_en=GPIO_PULLUP_ENABLE,.scl_pullup_en=GPIO_PULLUP_ENABLE,.master={ .clk_speed=400000}}
#define CST816S_IO_I2C_CONFIG {.dev_addr=ESP_LCD_TOUCH_IO_I2C_CST816S_ADDRESS,.control_phase_bytes=1,.lcd_cmd_bits=8,.flags={.disable_control_phase=1}}
#define CST816S_TOUCH_CONFIG {.x_max=TFT_WIDTH,.y_max=TFT_HEIGHT,.rst_gpio_num=GPIO_NUM_1,.int_gpio_num=GPIO_NUM_0}
#define TOUCH_ROT_NONE_SWAP_X false
#define TOUCH_ROT_NONE_SWAP_Y false
#endif
#endif

// ESP32_2432S024 N/R/C
#if defined(ESP32_2432S024N) || defined(ESP32_2432S024R) || defined(ESP32_2432S024C)
#define TFT_WIDTH 240
#define TFT_HEIGHT 320
// Backlight
#define PIN_BCKL 27
// LCD
#define USES_ILI9341
#define ILI9341_SPI_HOST SPI2_HOST
#define ILI9341_SPI_BUS_CONFIG {.mosi_io_num=13,.miso_io_num=12,.sclk_io_num=14,.quadwp_io_num=-1,.quadhd_io_num=-1}
#define ILI9341_IO_SPI_CONFIG  {.cs_gpio_num=15,.dc_gpio_num=2,.spi_mode=SPI_MODE0,.pclk_hz=24000000,.trans_queue_depth=10,.lcd_cmd_bits=8,.lcd_param_bits=8}
#define ILI9341_PANEL_DEV_CONFIG {.reset_gpio_num=-1,.color_space=ESP_LCD_COLOR_SPACE_BGR,.bits_per_pixel=16}
#define PANEL_ROT_NONE_SWAP_XY false
#define PANEL_ROT_NONE_MIRROR_X true
#define PANEL_ROT_NONE_MIRROR_Y false
// Touch
#ifdef ESP32_2432S024R
#define USES_XPT2046
#define XPT2046_SPI_HOST SPI2_HOST
#define XPT2046_SPI_BUS_CONFIG {.mosi_io_num=13,.miso_io_num=12,.sclk_io_num=14,.quadwp_io_num=-1,.quadhd_io_num=-1}
#define XPT2046_IO_SPI_CONFIG {.cs_gpio_num=33,.dc_gpio_num=-1,.spi_mode=SPI_MODE0,.pclk_hz=2000000,.trans_queue_depth=3,.lcd_cmd_bits=8,.lcd_param_bits=8}
#define XPT2046_TOUCH_CONFIG {.x_max=TFT_WIDTH,.y_max=TFT_HEIGHT,.rst_gpio_num=GPIO_NUM_NC,.int_gpio_num=GPIO_NUM_36}
#define TOUCH_ROT_NONE_SWAP_X true
#define TOUCH_ROT_NONE_SWAP_Y false
#else
#ifdef ESP32_2432S024C
#define USES_CST816S
#include "esp_lcd_touch_cst816s.h"
#define CST816S_I2C_HOST 0
#define CST816S_I2C_CONFIG {.mode=I2C_MODE_MASTER,.sda_io_num=33,.scl_io_num=32,.sda_pullup_en=GPIO_PULLUP_ENABLE,.scl_pullup_en=GPIO_PULLUP_ENABLE,.master={.clk_speed=400000}}
#define CST816S_IO_I2C_CONFIG {.dev_addr=ESP_LCD_TOUCH_IO_I2C_CST816S_ADDRESS,.control_phase_bytes=1,.lcd_cmd_bits=8,.flags={.disable_control_phase=1}}
#define CST816S_TOUCH_CONFIG {.x_max=TFT_WIDTH,.y_max=TFT_HEIGHT,.rst_gpio_num=GPIO_NUM_25,.int_gpio_num=GPIO_NUM_NC}
#define TOUCH_ROT_NONE_SWAP_X false
#define TOUCH_ROT_NONE_SWAP_Y false
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
// LCD
#define USES_ILI9341
#define ILI9341_SPI_HOST SPI2_HOST
#define ILI9341_SPI_BUS_CONFIG {.mosi_io_num=13,.miso_io_num=12,.sclk_io_num=14,.quadwp_io_num=-1,.quadhd_io_num=-1}
#define ILI9341_IO_SPI_CONFIG {.cs_gpio_num=15,.dc_gpio_num=2,.spi_mode=SPI_MODE0,.pclk_hz=24000000,.trans_queue_depth=10,.lcd_cmd_bits=8,.lcd_param_bits=8}
#define ILI9341_PANEL_DEV_CONFIG {.reset_gpio_num=-1,.color_space=ESP_LCD_COLOR_SPACE_BGR,.bits_per_pixel=16}
#define PANEL_ROT_NONE_SWAP_XY false
#define PANEL_ROT_NONE_MIRROR_X true
#define PANEL_ROT_NONE_MIRROR_Y false
// Touch
#define USES_XPT2046
#define XPT2046_SPI_HOST SPI3_HOST
#define XPT2046_SPI_BUS_CONFIG {.mosi_io_num=32,.miso_io_num=39,.sclk_io_num=25,.quadwp_io_num=-1,.quadhd_io_num=-1}
#define XPT2046_IO_SPI_CONFIG {.cs_gpio_num=33,.dc_gpio_num=-1,.spi_mode=SPI_MODE0,.pclk_hz=2000000,.trans_queue_depth=3,.lcd_cmd_bits=8,.lcd_param_bits=8}
#define XPT2046_TOUCH_CONFIG {.x_max=TFT_WIDTH,.y_max=TFT_HEIGHT,.rst_gpio_num=GPIO_NUM_NC,.int_gpio_num=GPIO_NUM_36}
#define TOUCH_ROT_NONE_SWAP_X true
#define TOUCH_ROT_NONE_SWAP_Y false
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

// ESP32_2432S032 N/R/C
#if defined(ESP32_2432S032N) || defined(ESP32_2432S032R) || defined(ESP32_2432S032C)
#define TFT_WIDTH 240
#define TFT_HEIGHT 320
// Backlight
#define PIN_BCKL 27
// LCD
#define USES_ST7796
#define ST7796_SPI_HOST SPI2_HOST
#define ST7796_SPI_BUS_CONFIG {.mosi_io_num=13,.miso_io_num=12,.sclk_io_num=14,.quadwp_io_num=-1,.quadhd_io_num=-1}
#define ST7796_IO_SPI_CONFIG {.cs_gpio_num=15,.dc_gpio_num=2,.spi_mode=SPI_MODE0,.pclk_hz=24000000,.trans_queue_depth=10,.lcd_cmd_bits=8,.lcd_param_bits=8}
#define ST7796_PANEL_DEV_CONFIG {.reset_gpio_num=-1,.color_space=ESP_LCD_COLOR_SPACE_BGR,.bits_per_pixel=16}
#define PANEL_ROT_NONE_SWAP_XY false
#define PANEL_ROT_NONE_MIRROR_X true
#define PANEL_ROT_NONE_MIRROR_Y false
// Touch
#ifdef ESP32_2432S032R
#define USES_XPT2046
#define XPT2046_SPI_HOST SPI2_HOST
// Do not initialize the bus: already done by the ST7796
#define XPT2046_SPI_BUS_CONFIG {.mosi_io_num=13,.miso_io_num=12,.sclk_io_num=14,.quadwp_io_num=-1,.quadhd_io_num=-1}
#define XPT2046_IO_SPI_CONFIG {.cs_gpio_num=33,.dc_gpio_num=-1,.spi_mode=SPI_MODE0,.pclk_hz=2000000,.trans_queue_depth=3,.lcd_cmd_bits=8,.lcd_param_bits=8}
#define XPT2046_TOUCH_CONFIG {.x_max=TFT_WIDTH,.y_max=TFT_HEIGHT,.rst_gpio_num=GPIO_NUM_NC,.int_gpio_num=GPIO_NUM_36}
#define TOUCH_ROT_NONE_SWAP_X true
#define TOUCH_ROT_NONE_SWAP_Y false
#else
#ifdef ESP32_2432S032C
#define USES_GT911
#include "esp_lcd_touch_gt911.h"
#define GT911_I2C_HOST 0
#define GT911_I2C_CONFIG {.mode=I2C_MODE_MASTER,.sda_io_num=33,.scl_io_num=32,.sda_pullup_en=GPIO_PULLUP_ENABLE,.scl_pullup_en=GPIO_PULLUP_ENABLE,.master={.clk_speed=400000}}
#define GT911_IO_I2C_CONFIG {.dev_addr=ESP_LCD_TOUCH_IO_I2C_GT911_ADDRESS,.control_phase_bytes=1,.lcd_cmd_bits=16,.flags={.disable_control_phase=1}}
#define GT911_TOUCH_CONFIG {.x_max=TFT_WIDTH,.y_max=TFT_HEIGHT,.rst_gpio_num=GPIO_NUM_25,.int_gpio_num=GPIO_NUM_NC}
#define TOUCH_ROT_NONE_SWAP_X false
#define TOUCH_ROT_NONE_SWAP_Y false
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

// ESP32_3248S035 R/C
#if defined(ESP32_3248S035R) || defined(ESP32_3248S035C)
#define TFT_WIDTH 320
#define TFT_HEIGHT 480
// Backlight
#define PIN_BCKL 27
// LCD
#define USES_ST7796
#define ST7796_SPI_HOST SPI2_HOST
#define ST7796_SPI_BUS_CONFIG {.mosi_io_num=13,.miso_io_num=12,.sclk_io_num=14,.quadwp_io_num=-1,.quadhd_io_num=-1}
#define ST7796_IO_SPI_CONFIG {.cs_gpio_num=15,.dc_gpio_num=2,.spi_mode=SPI_MODE0,.pclk_hz=24000000,.trans_queue_depth=10,.lcd_cmd_bits=8,.lcd_param_bits=8}
#define ST7796_PANEL_DEV_CONFIG {.reset_gpio_num=-1,.color_space=ESP_LCD_COLOR_SPACE_BGR,.bits_per_pixel=16}
#define PANEL_ROT_NONE_SWAP_XY false
#define PANEL_ROT_NONE_MIRROR_X true
#define PANEL_ROT_NONE_MIRROR_Y false
// Touch
#ifdef ESP32_3248S035R
#define USES_XPT2046
#define XPT2046_SPI_HOST SPI2_HOST
// Do not initialize the bus; already done by the ST7796
#define XPT2046_SPI_BUS_CONFIG {.mosi_io_num=13,.miso_io_num=12,.sclk_io_num=14,.quadwp_io_num=-1,.quadhd_io_num=-1}
#define XPT2046_IO_SPI_CONFIG {.cs_gpio_num=33,.dc_gpio_num=-1,.spi_mode=SPI_MODE0,.pclk_hz=2000000,.trans_queue_depth=3,.lcd_cmd_bits=8,.lcd_param_bits=8}
#define XPT2046_TOUCH_CONFIG {.x_max=TFT_WIDTH,.y_max=TFT_HEIGHT,.rst_gpio_num=GPIO_NUM_NC,.int_gpio_num=GPIO_NUM_36}
#define TOUCH_ROT_NONE_SWAP_X true
#define TOUCH_ROT_NONE_SWAP_Y false
#else
#ifdef ESP32_3248S035C
#define USES_GT911
#include "esp_lcd_touch_gt911.h"
#define GT911_I2C_HOST 0
#define GT911_I2C_CONFIG {.mode=I2C_MODE_MASTER,.sda_io_num=33,.scl_io_num=32,.sda_pullup_en=GPIO_PULLUP_ENABLE,.scl_pullup_en=GPIO_PULLUP_ENABLE,.master={.clk_speed=400000}}
#define GT911_IO_I2C_CONFIG {.dev_addr=ESP_LCD_TOUCH_IO_I2C_GT911_ADDRESS,.control_phase_bytes=1,.lcd_cmd_bits=16,.flags={.disable_control_phase=1}}
#define GT911_TOUCH_CONFIG {.x_max=TFT_WIDTH,.y_max=TFT_HEIGHT,.rst_gpio_num=GPIO_NUM_25,.int_gpio_num=GPIO_NUM_NC}
#define TOUCH_ROT_NONE_SWAP_X false
#define TOUCH_ROT_NONE_SWAP_Y false
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
// LCD ILI6485 480x272
#define USES_LCD_RGB
#define RBG_PANEL_CONFIG {.clk_src=LCD_CLK_SRC_PLL160M,.timings={.pclk_hz=9000000,.h_res=TFT_WIDTH,.v_res=TFT_HEIGHT,.hsync_pulse_width=4,.hsync_back_porch=43,.hsync_front_porch=8,.vsync_pulse_width=4,.vsync_back_porch=12,.vsync_front_porch=8,.flags={.hsync_idle_low=1,.vsync_idle_low=1,.pclk_active_neg=1}},.data_width=16,.sram_trans_align=8,.hsync_gpio_num=39,.vsync_gpio_num=41,.de_gpio_num=40,.pclk_gpio_num=42,.data_gpio_nums={8, 3, 46, 9, 1, 5, 6, 7, 15, 16, 4, 45, 48, 47, 21, 14},.disp_gpio_num=-1}
// Touch
#ifdef ESP32_4827S043R
#define USES_XPT2046
#define XPT2046_SPI_HOST SPI2_HOST
#define XPT2046_SPI_BUS_CONFIG {.mosi_io_num=11,.miso_io_num=13,.sclk_io_num=12,.quadwp_io_num=-1,.quadhd_io_num=-1}
#define XPT2046_IO_SPI_CONFIG {.cs_gpio_num=38,.dc_gpio_num=-1,.spi_mode=SPI_MODE0,.pclk_hz=2000000,.trans_queue_depth=3,.lcd_cmd_bits=8,.lcd_param_bits=8}
#define XPT2046_TOUCH_CONFIG {.x_max=TFT_WIDTH,.y_max=TFT_HEIGHT,.rst_gpio_num=GPIO_NUM_NC,.int_gpio_num=GPIO_NUM_18}
#else
#ifdef ESP32_4827S043C
#define USES_GT911
#include "esp_lcd_touch_gt911.h"
#define GT911_I2C_HOST 0
#define GT911_I2C_CONFIG {.mode=I2C_MODE_MASTER,.sda_io_num=19,.scl_io_num=20,.sda_pullup_en=GPIO_PULLUP_ENABLE,.scl_pullup_en=GPIO_PULLUP_ENABLE,.master={.clk_speed=400000}}
#define GT911_IO_I2C_CONFIG {.dev_addr=ESP_LCD_TOUCH_IO_I2C_GT911_ADDRESS,.control_phase_bytes=1,.lcd_cmd_bits=16,.flags={.disable_control_phase=1}}
#define GT911_TOUCH_CONFIG {.x_max=TFT_WIDTH,.y_max=TFT_HEIGHT,.rst_gpio_num=GPIO_NUM_38,.int_gpio_num=GPIO_NUM_NC}
#endif
#endif
#endif

// ESP32_8048S043 N/R/C
#if defined(ESP32_8048S043N) || defined(ESP32_8048S043R) || defined(ESP32_8048S043C)
#define TFT_WIDTH 800
#define TFT_HEIGHT 480
// Backlight
#define PIN_BCKL 2
// LCD 800x480
#define USES_LCD_RGB
#define RBG_PANEL_CONFIG {.clk_src=LCD_CLK_SRC_PLL160M,.timings={.pclk_hz=8000000,.h_res=TFT_WIDTH,.v_res=TFT_HEIGHT,.hsync_pulse_width=4,.hsync_back_porch=8,.hsync_front_porch=8,.vsync_pulse_width=4,.vsync_back_porch=8,.vsync_front_porch=8,.flags={.hsync_idle_low=1,.vsync_idle_low=1,.pclk_active_neg=1}},.data_width=16,.sram_trans_align=8,.hsync_gpio_num=39,.vsync_gpio_num=41,.de_gpio_num=40,.pclk_gpio_num=42,.data_gpio_nums={8, 3, 46, 9, 1, 5, 6, 7, 15, 16, 4, 45, 48, 47, 21, 14},.disp_gpio_num=-1}
// Touch
#ifdef ESP32_8048S043R
#define USES_XPT2046
#define XPT2046_SPI_HOST SPI2_HOST
#define XPT2046_SPI_BUS_CONFIG {.mosi_io_num=11,.miso_io_num=13,.sclk_io_num=12,.quadwp_io_num=-1,.quadhd_io_num=-1}
#define XPT2046_IO_SPI_CONFIG {.cs_gpio_num=38,.dc_gpio_num=-1,.spi_mode=SPI_MODE0,.pclk_hz=2000000,.trans_queue_depth=3,.lcd_cmd_bits=8,.lcd_param_bits=8}
#define XPT2046_TOUCH_CONFIG {.x_max=TFT_WIDTH,.y_max=TFT_HEIGHT,.rst_gpio_num=GPIO_NUM_NC,.int_gpio_num=GPIO_NUM_18}
#else
#ifdef ESP32_8048S043C
#define USES_GT911
#include "esp_lcd_touch_gt911.h"
#define GT911_I2C_HOST 0
#define GT911_I2C_CONFIG {.mode=I2C_MODE_MASTER,.sda_io_num=19,.scl_io_num=20,.sda_pullup_en=GPIO_PULLUP_ENABLE,.scl_pullup_en=GPIO_PULLUP_ENABLE,.master={.clk_speed=400000}}
#define GT911_IO_I2C_CONFIG {.dev_addr=ESP_LCD_TOUCH_IO_I2C_GT911_ADDRESS,.control_phase_bytes=1,.lcd_cmd_bits=16,.flags={.disable_control_phase=1}}
#define GT911_TOUCH_CONFIG {.x_max=TFT_WIDTH,.y_max=TFT_HEIGHT,.rst_gpio_num=GPIO_NUM_38,.int_gpio_num=GPIO_NUM_NC}
#endif
#endif
#endif

// ESP32_8048S050 N/R/C
#if defined(ESP32_8048S050N) || defined(ESP32_8048S050R) || defined(ESP32_8048S050C)
#define TFT_WIDTH 800
#define TFT_HEIGHT 480
// Backlight
#define PIN_BCKL 2
// LCD 800x480
#define USES_LCD_RGB
#define RBG_PANEL_CONFIG {.clk_src=LCD_CLK_SRC_PLL160M,.timings={.pclk_hz=16000000,.h_res=TFT_WIDTH,.v_res=TFT_HEIGHT,.hsync_pulse_width=4,.hsync_back_porch=8,.hsync_front_porch=8,.vsync_pulse_width=4,.vsync_back_porch=8,.vsync_front_porch=8,.flags={.hsync_idle_low=1,.vsync_idle_low=1,.pclk_active_neg=1,}},.data_width=16,.sram_trans_align=8,.hsync_gpio_num=39,.vsync_gpio_num=41,.de_gpio_num=40,.pclk_gpio_num=42,.data_gpio_nums={8, 3, 46, 9, 1, 5, 6, 7, 15, 16, 4, 45, 48, 47, 21, 14},.disp_gpio_num=-1}
// Touch
#ifdef ESP32_8048S050R
#define USES_XPT2046
#define XPT2046_SPI_HOST SPI2_HOST
#define XPT2046_SPI_BUS_CONFIG {.mosi_io_num=11,.miso_io_num=13,.sclk_io_num=12,.quadwp_io_num=-1,.quadhd_io_num=-1}
#define XPT2046_IO_SPI_CONFIG {.cs_gpio_num=38,.dc_gpio_num=-1,.spi_mode=SPI_MODE0,.pclk_hz=2000000,.trans_queue_depth=3,.lcd_cmd_bits=8,.lcd_param_bits=8}
#define XPT2046_TOUCH_CONFIG {.x_max=TFT_WIDTH,.y_max=TFT_HEIGHT,.rst_gpio_num=GPIO_NUM_NC,.int_gpio_num=GPIO_NUM_18}
#else
#ifdef ESP32_8048S050C
#define USES_GT911
#include "esp_lcd_touch_gt911.h"
#define GT911_I2C_HOST 0
#define GT911_I2C_CONFIG {.mode=I2C_MODE_MASTER,.sda_io_num=19,.scl_io_num=20,.sda_pullup_en=GPIO_PULLUP_ENABLE,.scl_pullup_en=GPIO_PULLUP_ENABLE,.master={.clk_speed=400000}}
#define GT911_IO_I2C_CONFIG {.dev_addr=ESP_LCD_TOUCH_IO_I2C_GT911_ADDRESS,.control_phase_bytes=1,.lcd_cmd_bits=16,.flags={.disable_control_phase=1}}
#define GT911_TOUCH_CONFIG {.x_max=TFT_WIDTH,.y_max=TFT_HEIGHT,.rst_gpio_num=GPIO_NUM_38,.int_gpio_num=GPIO_NUM_NC}
#endif
#endif
#endif

// ESP32_8048S070 N/R/C
#if defined(ESP32_8048S070N) || defined(ESP32_8048S070R) || defined(ESP32_8048S070C)
#define TFT_WIDTH 800
#define TFT_HEIGHT 480
// Backlight
#define PIN_BCKL 2
// LCD 800x480
#define USES_LCD_RGB
#define RBG_PANEL_CONFIG {.clk_src=LCD_CLK_SRC_PLL160M,.timings={.pclk_hz=12000000,.h_res=TFT_WIDTH,.v_res=TFT_HEIGHT,.hsync_pulse_width=30,.hsync_back_porch=16,.hsync_front_porch=210,.vsync_pulse_width=13,.vsync_back_porch=10,.vsync_front_porch=22,.flags={.hsync_idle_low=1,.vsync_idle_low=1,.pclk_active_neg=1,}},.data_width=16,.sram_trans_align=8,.hsync_gpio_num=39,.vsync_gpio_num=40,.de_gpio_num=41,.pclk_gpio_num=42,.data_gpio_nums={15, 7, 6, 5, 4, 9, 46, 3, 8, 16, 1, 14, 21, 47, 48, 45},.disp_gpio_num=-1}
// Touch
#ifdef ESP32_8048S070R
#define USES_XPT2046
#define XPT2046_SPI_HOST SPI2_HOST
#define XPT2046_SPI_BUS_CONFIG {.mosi_io_num=11,.miso_io_num=13,.sclk_io_num=12,.quadwp_io_num=-1,.quadhd_io_num=-1}
#define XPT2046_IO_SPI_CONFIG {.cs_gpio_num=38,.dc_gpio_num=-1,.spi_mode=SPI_MODE0,.pclk_hz=2000000,.trans_queue_depth=3,.lcd_cmd_bits=8,.lcd_param_bits=8}
#define XPT2046_TOUCH_CONFIG {.x_max=TFT_WIDTH,.y_max=TFT_HEIGHT,.rst_gpio_num=GPIO_NUM_NC,.int_gpio_num=GPIO_NUM_18}
#else
#ifdef ESP32_8048S070C
#define USES_GT911
#include "esp_lcd_touch_gt911.h"
#define GT911_I2C_HOST 0
#define GT911_I2C_CONFIG {.mode=I2C_MODE_MASTER,.sda_io_num=19,.scl_io_num=20,.sda_pullup_en=GPIO_PULLUP_ENABLE,.scl_pullup_en=GPIO_PULLUP_ENABLE,.master={.clk_speed=400000}}
#define GT911_IO_I2C_CONFIG {.dev_addr=ESP_LCD_TOUCH_IO_I2C_GT911_ADDRESS,.control_phase_bytes=1,.lcd_cmd_bits=16,.flags={.disable_control_phase=1}}
#define GT911_TOUCH_CONFIG {.x_max=TFT_WIDTH,.y_max=TFT_HEIGHT,.rst_gpio_num=GPIO_NUM_38,.int_gpio_num=GPIO_NUM_NC}
#endif
#endif
#endif

#if defined(USES_ILI9341) || defined(USES_ST7796)
// These use an SPI interface. Because display is LSB first the option LV_COLOR_16_SWAP must be set
#if LV_COLOR_16_SWAP == 0
#error "LV_COLOR_16_SWAP should be set to 1 in lv_conf.h because of SPI interface"
#endif
#endif

#if defined(USES_CST816S) || defined(USES_XPT2046) || defined(USES_GT911)
#define USES_TOUCH
#endif

// Exported functions

#ifdef __cplusplus
extern "C"
{
#endif
    // Initialize the display and touch
    void smartdisplay_init();
    // Set the brightness of the backlight display
    void smartdisplay_tft_set_backlight(uint16_t duty); // 0-1023 (12 bits)
#ifdef __cplusplus
}
#endif

#endif