#pragma once

#include <esp_lcd.h>
#include <esp_lcd_types.h>
#include <esp_lcd_panel_vendor.h>

#ifdef __cplusplus
extern "C"
{
#endif

esp_err_t esp_lcd_new_panel_gc9a01(const esp_lcd_panel_io_handle_t io, const esp_lcd_panel_dev_config_t *config, esp_lcd_panel_handle_t *handle);

#ifdef __cplusplus
}
#endif
