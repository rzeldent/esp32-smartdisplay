#pragma once

#include <esp_lcd_touch.h>


#ifdef __cplusplus
extern "C"
{
#endif

    esp_err_t esp_lcd_touch_new_i2c_cst328(const esp_lcd_panel_io_handle_t io, const esp_lcd_touch_config_t *config, esp_lcd_touch_handle_t *handle);

#ifdef __cplusplus
}
#endif
