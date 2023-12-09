#include <esp32_smartdisplay.h>

#ifdef USES_LCD_RGB

#include <esp_lcd_panel_rgb.h>
#include <esp_lcd_panel_ops.h>

static bool direct_io_frame_trans_done(esp_lcd_panel_handle_t panel, esp_lcd_rgb_panel_event_data_t *edata, void *user_ctx)
{
    lv_disp_drv_t *disp_driver = user_ctx;
    lv_disp_flush_ready(disp_driver);
    return false;
}

static void direct_io_lv_flush(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_map)
{
    esp_lcd_panel_handle_t panel_handle = drv->user_data;
#if LV_COLOR_16_SWAP != 1
#warning "LV_COLOR_16_SWAP should be 1 for max performance"
    ushort pixels = lv_area_get_size(area);
    lv_color16_t *p = color_map;
    while (pixels--)
        p++->full = (uint16_t)((p->full >> 8) | (p->full << 8));
#endif
    ESP_ERROR_CHECK(esp_lcd_panel_draw_bitmap(panel_handle, area->x1, area->y1, area->x2 + 1, area->y2 + 1, color_map));
};

void lvgl_tft_init(lv_disp_drv_t *drv)
{
    // Create direct_io panel handle
    esp_lcd_rgb_panel_config_t tft_panel_config = RBG_PANEL_CONFIG;
    tft_panel_config.on_frame_trans_done = direct_io_frame_trans_done;
    tft_panel_config.user_ctx = drv;

    esp_lcd_panel_handle_t panel_handle;
    ESP_ERROR_CHECK(esp_lcd_new_rgb_panel(&tft_panel_config, &panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));
    drv->user_data = panel_handle;
    drv->flush_cb = direct_io_lv_flush;
}

#endif