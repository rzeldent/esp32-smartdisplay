#include <esp32_smartdisplay.h>

#ifdef USES_DIRECT_IO

#include <driver/spi_master.h>
#include <esp_lcd_panel_io.h>
#include <esp_lcd_panel_rgb.h>
#include <esp_lcd_panel_ops.h>

#define DIRECT_IO_SPI_HOST SPI2_HOST
#define DIRECT_IO_PIXEL_BUFFER_LINES 16

esp_lcd_panel_io_handle_t io_handle;
esp_lcd_panel_handle_t panel_handle;

bool direct_io_frame_trans_done(esp_lcd_panel_handle_t panel, esp_lcd_rgb_panel_event_data_t *edata, void *user_ctx)
{
    const auto disp_driver = (lv_disp_drv_t *)user_ctx;
    lv_disp_flush_ready(disp_driver);
    return false;
}

void direct_io_lv_flush(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_map)
{
    ESP_ERROR_CHECK(esp_lcd_panel_draw_bitmap(panel_handle, area->x1, area->y1, area->x2 + 1, area->y2 + 1, color_map));
};

void lvgl_tft_init(lv_disp_drv_t *drv)
{
    pinMode(PIN_BCKL, OUTPUT);
    ledcSetup(PWM_CHANNEL_BL, PWM_FREQ_BL, PWM_BITS_BL);
    ledcAttachPin(PIN_BCKL, PWM_CHANNEL_BL);
    digitalWrite(PIN_BCKL, LOW);

    drv->draw_buf = new lv_disp_draw_buf_t;
    auto drawBufferPixels = drv->hor_res * DIRECT_IO_PIXEL_BUFFER_LINES;
    auto drawBuffer = new lv_color_t[drawBufferPixels];
    lv_disp_draw_buf_init(drv->draw_buf, drawBuffer, nullptr, drawBufferPixels * sizeof(lv_color_t));

    // Create direct_io panel handle
    esp_lcd_rgb_panel_config_t tft_panel_config = {
        .clk_src = LCD_CLK_SRC_PLL160M,
        .timings = {
            .pclk_hz = SPI_MASTER_FREQ_16M,
            .h_res = TFT_WIDTH,
            .v_res = TFT_HEIGHT,
            .hsync_pulse_width = 4,
            .hsync_back_porch = 43,
            .hsync_front_porch = 8,
            .vsync_pulse_width = 4,
            .vsync_back_porch = 12,
            .vsync_front_porch = 8,
            .flags = {
                .hsync_idle_low = 1,
                .vsync_idle_low = 1,
                .de_idle_high = 0,
                .pclk_active_neg = 1,
                .pclk_idle_high = 0,
            }},
        .data_width = 16, // RGB565
        .sram_trans_align = 8,
        .psram_trans_align = 64,
        .hsync_gpio_num = 39 /*HSYNC*/,
        .vsync_gpio_num = 41 /*VSYNC*/,
        .de_gpio_num = 40 /*DE*/,
        .pclk_gpio_num = 42 /*PCLK*/,
        .data_gpio_nums = {8 /*B0*/, 3 /*B1*/, 46 /*B2*/, 9 /*B3*/, 1 /*B4*/, 5 /*G0*/, 6 /*G1*/, 7 /*G2*/, 15 /*G3*/, 16 /*G4*/, 4 /*G5*/, 45 /*R0*/, 48 /*R1*/, 47 /*R2*/, 14 /*R4*/},
        .disp_gpio_num = GPIO_NUM_NC,
        .on_frame_trans_done = direct_io_frame_trans_done,
        .user_ctx = nullptr,
        .flags = {.disp_active_low = 0, .relax_on_idle = 0, .fb_in_psram = 1}};

    ESP_ERROR_CHECK(esp_lcd_new_rgb_panel(&tft_panel_config, &panel_handle));
    drv->user_data = panel_handle;
    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));

    drv->flush_cb = direct_io_lv_flush;

    ledcWrite(PWM_CHANNEL_BL, PWM_MAX_BL);

    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, true));
}

void smartdisplay_tft_set_backlight(uint16_t duty)
{
    ledcWrite(PWM_CHANNEL_BL, duty);
}

void smartdisplay_tft_sleep()
{
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, false));
}

void smartdisplay_tft_wake()
{
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, true));
}

#endif