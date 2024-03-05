#pragma once

#include <esp_lcd_touch.h>

// I2C Address
#define CST816S_IO_I2C_CONFIG_DEV_ADDRESS_15 0x15 // 0x15 (0x2A/0x2B): INT High during reset or

#ifdef __cplusplus
extern "C"
{
#endif

    esp_err_t esp_lcd_touch_new_i2c_cst816s(const esp_lcd_panel_io_handle_t io, const esp_lcd_touch_config_t *config, esp_lcd_touch_handle_t *handle);

#ifdef __cplusplus
}
#endif
