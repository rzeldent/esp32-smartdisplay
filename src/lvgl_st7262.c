#ifdef LCD_USES_ST7262

#include <esp32_smartdisplay.h>
#include <esp_lcd_panel_rgb.h>
#include <esp_lcd_panel_ops.h>

static bool direct_io_frame_trans_done(esp_lcd_panel_handle_t panel, esp_lcd_rgb_panel_event_data_t *edata, void *user_ctx)
{
    lv_disp_drv_t *disp_driver = user_ctx;
    lv_disp_flush_ready(disp_driver);
    return false;
}

static void direct_io_lv_flush(lv_disp_drv_t *drv, const lv_area_t *area, lv_color16_t *color_map)
{
    esp_lcd_panel_handle_t panel_handle = drv->user_data;
    // LV_COLOR_16_SWAP is handled by mapping of the data
    ESP_ERROR_CHECK(esp_lcd_panel_draw_bitmap(panel_handle, area->x1, area->y1, area->x2 + 1, area->y2 + 1, color_map));
};

void lvgl_lcd_init(lv_disp_drv_t *drv)
{
    // Hardware rotation is NOT supported
    drv->sw_rotate = 1;
    drv->rotated = LV_DISP_ROT_NONE;

// Create direct_io panel handle
    const esp_lcd_rgb_panel_config_t tft_panel_config = {
        .clk_src = LCD_CLK_SRC_PLL160M,
        .timings = {
            .pclk_hz = 16000000,
            .h_res = LCD_WIDTH,
            .v_res = LCD_HEIGHT,
            .hsync_pulse_width = ST7262_HSYNC_PULSE_WIDTH,
            .hsync_back_porch = ST7262_HSYNC_BACK_PORCH,
            .hsync_front_porch = ST7262_HSYNC_FRONT_PORCH,
            .vsync_pulse_width = ST7262_VSYNC_PULSE_WIDTH,
            .vsync_back_porch = ST7262_VSYNC_BACK_PORCH,
            .vsync_front_porch = ST7262_VSYNC_FRONT_PORCH,
            .flags = {
                .hsync_idle_low = 1,
                .vsync_idle_low = 1,
                .pclk_active_neg = 1}},
        .data_width = 16,
        .psram_trans_align = 64,
        .hsync_gpio_num = ST7262_HSYNC,
        .vsync_gpio_num = ST7262_VSYNC,
        .de_gpio_num = ST7262_DE,
        .pclk_gpio_num = ST7262_PCLK,
        .disp_gpio_num = GPIO_NUM_NC,
#if LV_COLOR_16_SWAP == 0
        .data_gpio_nums = {ST7262_R0, ST7262_R1, ST7262_R2, ST7262_R3, ST7262_R4, ST7262_G0, ST7262_G1, ST7262_G2, ST7262_G3, ST7262_G4, ST7262_G5, ST7262_B0, ST7262_B1, ST7262_B2, ST7262_B3, ST7262_B4},
#else
        .data_gpio_nums = {ST7262_G3, ST7262_G4, ST7262_G5, ST7262_B0, ST7262_B1, ST7262_B2, ST7262_B3, ST7262_B4, ST7262_R0, ST7262_R1, ST7262_R2, ST7262_R3, ST7262_R4, ST7262_G0, ST7262_G1, ST7262_G2},
#endif
        .on_frame_trans_done = direct_io_frame_trans_done,
        .user_ctx = drv,
        .flags = {.fb_in_psram = 1}};

    esp_lcd_panel_handle_t panel_handle;
    ESP_ERROR_CHECK(esp_lcd_new_rgb_panel(&tft_panel_config, &panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));
    drv->user_data = panel_handle;
    drv->flush_cb = direct_io_lv_flush;
}

#endif