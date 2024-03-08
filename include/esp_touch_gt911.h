#pragma once

#include <esp_lcd_touch.h>

// Two I2C Addresses possible
#define GT911_IO_I2C_CONFIG_DEV_ADDRESS_5D 0x5D // 0x5D (0xBA/0xBB): INT High during reset or
#define GT911_IO_I2C_CONFIG_DEV_ADDRESS_14 0x14 // 0x14 (0x28/0x29): INT Low during reset

#ifdef __cplusplus
extern "C"
{
#endif

    esp_err_t esp_lcd_touch_new_i2c_gt911(const esp_lcd_panel_io_handle_t io, const esp_lcd_touch_config_t *config, esp_lcd_touch_handle_t *handle);

#ifdef __cplusplus
}
#endif
