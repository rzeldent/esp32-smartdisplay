#ifdef DISPLAY_ST7262_PAR

#include <esp32_smartdisplay.h>
#include <esp_lcd_panel_rgb.h>
#include <esp_lcd_panel_ops.h>
#include <esp32_smartdisplay_dma_helpers.h>

bool direct_io_frame_trans_done(esp_lcd_panel_handle_t panel, esp_lcd_rgb_panel_event_data_t *edata, void *user_ctx)
{
    // Note: When using DMA, lv_display_flush_ready() is called by DMA callbacks
    // This callback is only used for direct transfers (non-DMA fallback)
    // We return false to indicate we're not handling the flush completion here
    return false;
}

void direct_io_lv_flush(lv_display_t *display, const lv_area_t *area, uint8_t *px_map)
{
    // Hardware rotation is not supported - use optimized helper function with software rotation
    const esp_lcd_panel_handle_t panel_handle = display->user_data;
    smartdisplay_dma_flush_with_rotation(display, area, px_map, panel_handle, "ST7262 Parallel");
};

lv_display_t *lvgl_lcd_init()
{
    lv_display_t *display = lv_display_create(DISPLAY_WIDTH, DISPLAY_HEIGHT);
    log_v("display:0x%08x", display);
    //  Create drawBuffer
    lv_color_format_t cf = lv_display_get_color_format(display);
    uint32_t px_size = lv_color_format_get_size(cf);
    uint32_t drawBufferSize = px_size * LVGL_BUFFER_PIXELS;
    void *drawBuffer = heap_caps_malloc(drawBufferSize, LVGL_BUFFER_MALLOC_FLAGS);
    lv_display_set_buffers(display, drawBuffer, NULL, drawBufferSize, LV_DISPLAY_RENDER_MODE_PARTIAL);

    // Create direct_io panel handle
    const esp_lcd_rgb_panel_config_t rgb_panel_config = {
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
        .hsync_gpio_num = ST7262_PANEL_CONFIG_HSYNC,
        .vsync_gpio_num = ST7262_PANEL_CONFIG_VSYNC,
        .de_gpio_num = ST7262_PANEL_CONFIG_DE,
        .pclk_gpio_num = ST7262_PANEL_CONFIG_PCLK,
        // LV_COLOR_16_SWAP is handled by mapping of the data
        .data_gpio_nums = {ST7262_PANEL_CONFIG_DATA_R0, ST7262_PANEL_CONFIG_DATA_R1, ST7262_PANEL_CONFIG_DATA_R2, ST7262_PANEL_CONFIG_DATA_R3, ST7262_PANEL_CONFIG_DATA_R4, ST7262_PANEL_CONFIG_DATA_G0, ST7262_PANEL_CONFIG_DATA_G1, ST7262_PANEL_CONFIG_DATA_G2, ST7262_PANEL_CONFIG_DATA_G3, ST7262_PANEL_CONFIG_DATA_G4, ST7262_PANEL_CONFIG_DATA_G5, ST7262_PANEL_CONFIG_DATA_B0, ST7262_PANEL_CONFIG_DATA_B1, ST7262_PANEL_CONFIG_DATA_B2, ST7262_PANEL_CONFIG_DATA_B3, ST7262_PANEL_CONFIG_DATA_B4},
        .disp_gpio_num = ST7262_PANEL_CONFIG_DISP,
        .on_frame_trans_done = direct_io_frame_trans_done,
        .user_ctx = display,
        .flags = {.disp_active_low = ST7262_PANEL_CONFIG_FLAGS_DISP_ACTIVE_LOW, .relax_on_idle = ST7262_PANEL_CONFIG_FLAGS_RELAX_ON_IDLE, .fb_in_psram = ST7262_PANEL_CONFIG_FLAGS_FB_IN_PSRAM}};
    log_d("rgb_panel_config: clk_src:%d, timings:{pclk_hz:%d, h_res:%d, v_res:%d, hsync_pulse_width:%d, hsync_back_porch:%d, hsync_front_porch:%d, vsync_pulse_width:%d, vsync_back_porch:%d, vsync_front_porch:%d, flags:{hsync_idle_low:%d, vsync_idle_low:%d, de_idle_high:%d, pclk_active_neg:%d, pclk_idle_high:%d}}, data_width:%d, sram_trans_align:%d, psram_trans_align:%d, hsync_gpio_num:%d, vsync_gpio_num:%d, de_gpio_num:%d, pclk_gpio_num:%d, data_gpio_nums:[%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,], disp_gpio_num:%d, on_frame_trans_done:0x%08x, user_ctx:0x%08x, flags:{disp_active_low:%d, relax_on_idle:%d, fb_in_psram:%d}", rgb_panel_config.clk_src, rgb_panel_config.timings.pclk_hz, rgb_panel_config.timings.h_res, rgb_panel_config.timings.v_res, rgb_panel_config.timings.hsync_pulse_width, rgb_panel_config.timings.hsync_back_porch, rgb_panel_config.timings.hsync_front_porch, rgb_panel_config.timings.vsync_pulse_width, rgb_panel_config.timings.vsync_back_porch, rgb_panel_config.timings.vsync_front_porch, rgb_panel_config.timings.flags.hsync_idle_low, rgb_panel_config.timings.flags.vsync_idle_low, rgb_panel_config.timings.flags.de_idle_high, rgb_panel_config.timings.flags.pclk_active_neg, rgb_panel_config.timings.flags.pclk_idle_high, rgb_panel_config.data_width, rgb_panel_config.sram_trans_align, rgb_panel_config.psram_trans_align, rgb_panel_config.hsync_gpio_num, rgb_panel_config.vsync_gpio_num, rgb_panel_config.de_gpio_num, rgb_panel_config.pclk_gpio_num, rgb_panel_config.data_gpio_nums[0], rgb_panel_config.data_gpio_nums[1], rgb_panel_config.data_gpio_nums[2], rgb_panel_config.data_gpio_nums[3], rgb_panel_config.data_gpio_nums[4], rgb_panel_config.data_gpio_nums[5], rgb_panel_config.data_gpio_nums[6], rgb_panel_config.data_gpio_nums[7], rgb_panel_config.data_gpio_nums[8], rgb_panel_config.data_gpio_nums[9], rgb_panel_config.data_gpio_nums[10], rgb_panel_config.data_gpio_nums[11], rgb_panel_config.data_gpio_nums[12], rgb_panel_config.data_gpio_nums[13], rgb_panel_config.data_gpio_nums[14], rgb_panel_config.data_gpio_nums[15], rgb_panel_config.disp_gpio_num, rgb_panel_config.on_frame_trans_done, rgb_panel_config.user_ctx, rgb_panel_config.flags.disp_active_low, rgb_panel_config.flags.relax_on_idle, rgb_panel_config.flags.fb_in_psram);
    log_d("refresh rate: %d Hz", (ST7262_PANEL_CONFIG_TIMINGS_PCLK_HZ * ST7262_PANEL_CONFIG_DATA_WIDTH) / (ST7262_PANEL_CONFIG_TIMINGS_H_RES + ST7262_PANEL_CONFIG_TIMINGS_HSYNC_PULSE_WIDTH + ST7262_PANEL_CONFIG_TIMINGS_HSYNC_BACK_PORCH + ST7262_PANEL_CONFIG_TIMINGS_HSYNC_FRONT_PORCH) / (ST7262_PANEL_CONFIG_TIMINGS_V_RES + ST7262_PANEL_CONFIG_TIMINGS_VSYNC_PULSE_WIDTH + ST7262_PANEL_CONFIG_TIMINGS_VSYNC_BACK_PORCH + ST7262_PANEL_CONFIG_TIMINGS_VSYNC_FRONT_PORCH) / SOC_LCD_RGB_DATA_WIDTH);
    esp_lcd_panel_handle_t panel_handle;
    ESP_ERROR_CHECK(esp_lcd_new_rgb_panel(&rgb_panel_config, &panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));
    
    // Initialize DMA for optimized transfers
    smartdisplay_dma_init_with_logging(panel_handle, "ST7262 Parallel");
    
#ifdef DISPLAY_IPS
    // If LCD is IPS invert the colors
    ESP_ERROR_CHECK(esp_lcd_panel_invert_color(panel_handle, true));
#endif
#if defined(DISPLAY_GAP_X) || defined(DISPLAY_GAP_Y)
    ESP_ERROR_CHECK(esp_lcd_panel_set_gap(panel_handle, DISPLAY_GAP_X, DISPLAY_GAP_Y));
#endif
    display->user_data = panel_handle;
    display->flush_cb = direct_io_lv_flush;

    return display;
}

#endif