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
        .clk_src = ST7262_PANEL_CONFIG_CLK_SRC,
        .timings = {
            .pclk_hz = ST7262_PANEL_CONFIG_TIMINGS_PCLK_HZ,
            .h_res = ST7262_PANEL_CONFIG_TIMINGS_H_RES,
            .v_res = ST7262_PANEL_CONFIG_TIMINGS_V_RES,
            .hsync_pulse_width = ST7262_PANEL_CONFIG_TIMINGS_HSYNC_PULSE_WIDTH,
            .hsync_back_porch = ST7262_PANEL_CONFIG_TIMINGS_HSYNC_BACK_PORCH,
            .hsync_front_porch = ST7262_PANEL_CONFIG_TIMINGS_HSYNC_FRONT_PORCH,
            .vsync_pulse_width = ST7262_PANEL_CONFIG_TIMINGS_VSYNC_PULSE_WIDTH,
            .vsync_back_porch = ST7262_PANEL_CONFIG_TIMINGS_VSYNC_BACK_PORCH,
            .vsync_front_porch = ST7262_PANEL_CONFIG_TIMINGS_VSYNC_FRONT_PORCH,
            .flags = {
                .hsync_idle_low = ST7262_PANEL_CONFIG_TIMINGS_FLAGS_HSYNC_IDLE_LOW,
                .vsync_idle_low = ST7262_PANEL_CONFIG_TIMINGS_FLAGS_VSYNC_IDLE_LOW,
                .de_idle_high = ST7262_PANEL_CONFIG_TIMINGS_FLAGS_DE_IDLE_HIGH,
                .pclk_active_neg = ST7262_PANEL_CONFIG_TIMINGS_FLAGS_PCLK_ACTIVE_NEG,
                .pclk_idle_high = ST7262_PANEL_CONFIG_TIMINGS_FLAGS_PCLK_IDLE_HIGH}},
        .data_width = ST7262_PANEL_CONFIG_DATA_WIDTH,
        .sram_trans_align = ST7262_PANEL_CONFIG_SRAM_TRANS_ALIGN,
        .psram_trans_align = ST7262_PANEL_CONFIG_PSRAM_TRANS_ALIGN,
        .hsync_gpio_num = ST7262_PANEL_CONFIG_HSYNC_GPIO_NUM,
        .vsync_gpio_num = ST7262_PANEL_CONFIG_VSYNC_GPIO_NUM,
        .de_gpio_num = ST7262_PANEL_CONFIG_DE_GPIO_NUM,
        .pclk_gpio_num = ST7262_PANEL_CONFIG_PCLK_GPIO_NUM,
#if LV_COLOR_16_SWAP == 0
        .data_gpio_nums = {ST7262_PANEL_CONFIG_DATA_GPIO_R0, ST7262_PANEL_CONFIG_DATA_GPIO_R1, ST7262_PANEL_CONFIG_DATA_GPIO_R2, ST7262_PANEL_CONFIG_DATA_GPIO_R3, ST7262_PANEL_CONFIG_DATA_GPIO_R4, ST7262_PANEL_CONFIG_DATA_GPIO_G0, ST7262_PANEL_CONFIG_DATA_GPIO_G1, ST7262_PANEL_CONFIG_DATA_GPIO_G2, ST7262_PANEL_CONFIG_DATA_GPIO_G3, ST7262_PANEL_CONFIG_DATA_GPIO_G4, ST7262_PANEL_CONFIG_DATA_GPIO_G5, ST7262_PANEL_CONFIG_DATA_GPIO_B0, ST7262_PANEL_CONFIG_DATA_GPIO_B1, ST7262_PANEL_CONFIG_DATA_GPIO_B2, ST7262_PANEL_CONFIG_DATA_GPIO_B3, ST7262_PANEL_CONFIG_DATA_GPIO_B4},
#else
        .data_gpio_nums = {ST7262_PANEL_CONFIG_DATA_GPIO_G3, ST7262_PANEL_CONFIG_DATA_GPIO_G4, ST7262_PANEL_CONFIG_DATA_GPIO_G5, ST7262_PANEL_CONFIG_DATA_GPIO_B0, ST7262_PANEL_CONFIG_DATA_GPIO_B1, ST7262_PANEL_CONFIG_DATA_GPIO_B2, ST7262_PANEL_CONFIG_DATA_GPIO_B3, ST7262_PANEL_CONFIG_DATA_GPIO_B4, ST7262_PANEL_CONFIG_DATA_GPIO_R0, ST7262_PANEL_CONFIG_DATA_GPIO_R1, ST7262_PANEL_CONFIG_DATA_GPIO_R2, ST7262_PANEL_CONFIG_DATA_GPIO_R3, ST7262_PANEL_CONFIG_DATA_GPIO_R4, ST7262_PANEL_CONFIG_DATA_GPIO_G0, ST7262_PANEL_CONFIG_DATA_GPIO_G1, ST7262_PANEL_CONFIG_DATA_GPIO_G2},
#endif
        .disp_gpio_num = ST7262_PANEL_CONFIG_DISP_GPIO_NUM,
        .on_frame_trans_done = direct_io_frame_trans_done,
        .user_ctx = drv,
        .flags = {.disp_active_low = ST7262_PANEL_CONFIG_FLAGS_DISP_ACTIVE_LOW, .relax_on_idle = ST7262_PANEL_CONFIG_FLAGS_RELAX_ON_IDLE, .fb_in_psram = ST7262_PANEL_CONFIG_FLAGS_FB_IN_PSRAM}
    };

    esp_lcd_panel_handle_t panel_handle;
    ESP_ERROR_CHECK(esp_lcd_new_rgb_panel(&tft_panel_config, &panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));
    drv->user_data = panel_handle;
    drv->flush_cb = direct_io_lv_flush;
}

#endif