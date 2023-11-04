#include <esp32_smartdisplay.h>

#ifdef TFT_ESP_LCD

#include <esp_lcd_panel_ops.h>

esp_lcd_panel_handle_t panel_handle;

void lvgl_tft_init()
{
  esp_lcd_new_rgb_panel(&tft_panel_config, &panel_handle);
  esp_lcd_panel_reset(panel_handle);
  esp_lcd_panel_init(panel_handle);
  pinMode(PIN_BL, OUTPUT);
  digitalWrite(PIN_BL, HIGH);
}

void lvgl_tft_flush(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_map)
{
  esp_lcd_panel_draw_bitmap(panel_handle, area->x1,area->y1, area->x2+1, area->y2+1, color_map);
  lv_disp_flush_ready(drv);
}

void smartdisplay_tft_set_backlight(uint16_t duty)
{
  ledcWrite(PWM_CHANNEL_BL, duty);
}

void smartdisplay_tft_sleep()
{
  //panel_handle->disp_off(panel_handle, true);
}

void smartdisplay_tft_wake()
{
  //panel_handle->disp_off(panel_handle, false);
}

#endif