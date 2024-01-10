#ifdef LCD_USES_ST7701

#include <esp32_smartdisplay.h>
#include <esp_lcd_st7701.h>
#include <esp_lcd_panel_io_additions.h>
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
    // Install 3-wire SPI panel IO
    spi_line_config_t line_config = {
        .cs_io_type = IO_TYPE_GPIO,
        .cs_gpio_num = ST7701_SPI_CS,
        .scl_io_type = IO_TYPE_GPIO,
        .scl_gpio_num = ST7701_SPI_SCL,
        .sda_io_type = IO_TYPE_GPIO,
        .sda_gpio_num = ST7701_SPI_SDA,
        .io_expander = NULL, // Set to NULL if not using IO expander
    };
    esp_lcd_panel_io_3wire_spi_config_t io_config = ST7701_PANEL_IO_3WIRE_SPI_CONFIG(line_config, 0);
    esp_lcd_panel_io_handle_t io_handle;
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_3wire_spi(&io_config, &io_handle));

// Create direct_io panel handle
#if LV_COLOR_16_SWAP == 0
    esp_lcd_rgb_panel_config_t tft_panel_config = {.clk_src = LCD_CLK_SRC_PLL160M, .timings = {.pclk_hz = 16000000, .h_res = LCD_WIDTH, .v_res = LCD_HEIGHT, .hsync_pulse_width = ST7701_HSYNC_PULSE_WIDTH, .hsync_back_porch = ST7701_HSYNC_BACK_PORCH, .hsync_front_porch = ST7701_HSYNC_FRONT_PORCH, .vsync_pulse_width = ST7701_VSYNC_PULSE_WIDTH, .vsync_back_porch = ST7701_VSYNC_BACK_PORCH, .vsync_front_porch = ST7701_VSYNC_FRONT_PORCH, .flags = {.hsync_idle_low = 1, .vsync_idle_low = 1, .pclk_active_neg = 1}}, .data_width = 16, .psram_trans_align = 64, .hsync_gpio_num = ST7701_HSYNC, .vsync_gpio_num = ST7701_VSYNC, .de_gpio_num = ST7701_DE, .pclk_gpio_num = ST7701_PCLK, .disp_gpio_num = -1, .data_gpio_nums = {ST7701_R0, ST7701_R1, ST7701_R2, ST7701_R3, ST7701_R4, ST7701_G0, ST7701_G1, ST7701_G2, ST7701_G3, ST7701_G4, ST7701_G5, ST7701_B0, ST7701_B1, ST7701_B2, ST7701_B3, ST7701_B4}, .flags = {.fb_in_psram = 1}};
#else
    esp_lcd_rgb_panel_config_t tft_panel_config = {.clk_src = LCD_CLK_SRC_PLL160M, .timings = {.pclk_hz = 16000000, .h_res = LCD_WIDTH, .v_res = LCD_HEIGHT, .hsync_pulse_width = ST7701_HSYNC_PULSE_WIDTH, .hsync_back_porch = ST7701_HSYNC_BACK_PORCH, .hsync_front_porch = ST7701_HSYNC_FRONT_PORCH, .vsync_pulse_width = ST7701_VSYNC_PULSE_WIDTH, .vsync_back_porch = ST7701_VSYNC_BACK_PORCH, .vsync_front_porch = ST7701_VSYNC_FRONT_PORCH, .flags = {.hsync_idle_low = 1, .vsync_idle_low = 1, .pclk_active_neg = 1}}, .data_width = 16, .psram_trans_align = 64, .hsync_gpio_num = ST7701_HSYNC, .vsync_gpio_num = ST7701_VSYNC, .de_gpio_num = ST7701_DE, .pclk_gpio_num = ST7701_PCLK, .disp_gpio_num = -1, .data_gpio_nums = {ST7701_G3, ST7701_G4, ST7701_G5, ST7701_B0, ST7701_B1, ST7701_B2, ST7701_B3, ST7701_B4, ST7701_R0, ST7701_R1, ST7701_R2, ST7701_R3, ST7701_R4, ST7701_G0, ST7701_G1, ST7701_G2}, .flags = {.fb_in_psram = 1}};
#endif
    tft_panel_config.on_frame_trans_done = direct_io_frame_trans_done;
    tft_panel_config.user_ctx = drv;

    st7701_vendor_config_t vendor_config = {
        .rgb_config = &tft_panel_config};

    const esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = -1,
#if ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(5, 0, 0)
        .color_space = ESP_LCD_COLOR_SPACE_RGB, // Implemented by LCD command `36h`
#else
        .rgb_endian = LCD_RGB_ENDIAN_RGB, // Implemented by LCD command `36h`
#endif
        .bits_per_pixel = 16, // Implemented by LCD command `3Ah` (16/18/24)
        .vendor_config = &vendor_config
    };

    esp_lcd_panel_handle_t panel_handle;
    ESP_ERROR_CHECK(esp_lcd_new_panel_st7701(io_handle, &panel_config, &panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));
    drv->user_data = panel_handle;
    drv->flush_cb = direct_io_lv_flush;
}

#endif