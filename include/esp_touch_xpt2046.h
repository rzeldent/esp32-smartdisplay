#pragma once

#include <esp_lcd_touch.h>

#ifdef __cplusplus
extern "C" {
#endif

    esp_err_t esp_lcd_touch_new_spi_xpt2046(const esp_lcd_panel_io_handle_t io, const esp_lcd_touch_config_t *config, esp_lcd_touch_handle_t *handle);
    esp_err_t esp_lcd_touch_xpt2046_read_battery_level(const esp_lcd_touch_handle_t tp, float *output);

#ifdef __cplusplus
}
#endif
