#pragma once

#include <esp_lcd.h>
#include <esp_lcd_types.h>
#include <esp_lcd_panel_vendor.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct
    {
        const lcd_init_cmd_t *init_cmds;
        uint16_t init_cmds_size;
    } st7701_vendor_config_t;

    esp_err_t esp_lcd_new_panel_st7701(const esp_lcd_panel_io_handle_t io, const esp_lcd_panel_dev_config_t *config, esp_lcd_panel_handle_t *handle);

#ifdef __cplusplus
}
#endif
