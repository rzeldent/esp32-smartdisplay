#ifdef DISPLAY_ST7262_PAR

#include <esp32_smartdisplay.h>
#include <esp_lcd_panel_rgb.h>
#include <esp_lcd_panel_ops.h>
#include <lvgl_panel_common.h>

lv_display_t *lvgl_lcd_init()
{
    lv_display_t *display = lvgl_create_display();
    log_v("display: %p", display);

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
        .bits_per_pixel = ST7262_PANEL_CONFIG_BITS_PER_PIXEL,
        .num_fbs = ST7262_PANEL_CONFIG_NUM_FBS,
        .bounce_buffer_size_px = ST7262_PANEL_CONFIG_BOUNCE_BUFFER_SIZE_PX,
        .dma_burst_size = ST7262_PANEL_CONFIG_DMA_BURST_SIZE,
        .hsync_gpio_num = ST7262_PANEL_CONFIG_HSYNC,
        .vsync_gpio_num = ST7262_PANEL_CONFIG_VSYNC,
        .de_gpio_num = ST7262_PANEL_CONFIG_DE,
        .pclk_gpio_num = ST7262_PANEL_CONFIG_PCLK,
        .disp_gpio_num = ST7262_PANEL_CONFIG_DISP,
        // LV_COLOR_16_SWAP is handled by mapping of the data
        .data_gpio_nums = {ST7262_PANEL_CONFIG_DATA_R0, ST7262_PANEL_CONFIG_DATA_R1, ST7262_PANEL_CONFIG_DATA_R2, ST7262_PANEL_CONFIG_DATA_R3, ST7262_PANEL_CONFIG_DATA_R4, ST7262_PANEL_CONFIG_DATA_G0, ST7262_PANEL_CONFIG_DATA_G1, ST7262_PANEL_CONFIG_DATA_G2, ST7262_PANEL_CONFIG_DATA_G3, ST7262_PANEL_CONFIG_DATA_G4, ST7262_PANEL_CONFIG_DATA_G5, ST7262_PANEL_CONFIG_DATA_B0, ST7262_PANEL_CONFIG_DATA_B1, ST7262_PANEL_CONFIG_DATA_B2, ST7262_PANEL_CONFIG_DATA_B3, ST7262_PANEL_CONFIG_DATA_B4},
        .flags = {.disp_active_low = ST7262_PANEL_CONFIG_FLAGS_DISP_ACTIVE_LOW, .refresh_on_demand = ST7262_PANEL_CONFIG_FLAGS_REFRESH_ON_DEMAND, .fb_in_psram = ST7262_PANEL_CONFIG_FLAGS_FB_IN_PSRAM, .double_fb = ST7262_PANEL_CONFIG_FLAGS_DOUBLE_FB, .no_fb = ST7262_PANEL_CONFIG_FLAGS_NO_FB, .bb_invalidate_cache = ST7262_PANEL_CONFIG_FLAGS_BB_INVALIDATE_CACHE}};
    log_d("rgb_panel_config: clk_src: %d, timings:{pclk_hz: %d, h_res: %d, v_res: %d, hsync_pulse_width: %d, hsync_back_porch: %d, hsync_front_porch: %d, vsync_pulse_width: %d, vsync_back_porch: %d, vsync_front_porch: %d, flags:{hsync_idle_low: %d, vsync_idle_low: %d, de_idle_high: %d, pclk_active_neg: %d, pclk_idle_high: %d}}, data_width: %d, bits_per_pixel: %d, num_fbs: %d, bounce_buffer_size_px: %d, dma_burst_size: %d, hsync_gpio_num: %d, vsync_gpio_num: %d, de_gpio_num: %d, pclk_gpio_num: %d, data_gpio_nums:[%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,], disp_gpio_num: %d, flags:{disp_active_low: %d, refresh_on_demand: %d, fb_in_psram: %d, double_fb: %d, no_fb: %d, bb_invalidate_cache: %d}", rgb_panel_config.clk_src, rgb_panel_config.timings.pclk_hz, rgb_panel_config.timings.h_res, rgb_panel_config.timings.v_res, rgb_panel_config.timings.hsync_pulse_width, rgb_panel_config.timings.hsync_back_porch, rgb_panel_config.timings.hsync_front_porch, rgb_panel_config.timings.vsync_pulse_width, rgb_panel_config.timings.vsync_back_porch, rgb_panel_config.timings.vsync_front_porch, rgb_panel_config.timings.flags.hsync_idle_low, rgb_panel_config.timings.flags.vsync_idle_low, rgb_panel_config.timings.flags.de_idle_high, rgb_panel_config.timings.flags.pclk_active_neg, rgb_panel_config.timings.flags.pclk_idle_high, rgb_panel_config.data_width, rgb_panel_config.bits_per_pixel, rgb_panel_config.num_fbs, rgb_panel_config.bounce_buffer_size_px, rgb_panel_config.dma_burst_size, rgb_panel_config.hsync_gpio_num, rgb_panel_config.vsync_gpio_num, rgb_panel_config.de_gpio_num, rgb_panel_config.pclk_gpio_num, rgb_panel_config.data_gpio_nums[0], rgb_panel_config.data_gpio_nums[1], rgb_panel_config.data_gpio_nums[2], rgb_panel_config.data_gpio_nums[3], rgb_panel_config.data_gpio_nums[4], rgb_panel_config.data_gpio_nums[5], rgb_panel_config.data_gpio_nums[6], rgb_panel_config.data_gpio_nums[7], rgb_panel_config.data_gpio_nums[8], rgb_panel_config.data_gpio_nums[9], rgb_panel_config.data_gpio_nums[10], rgb_panel_config.data_gpio_nums[11], rgb_panel_config.data_gpio_nums[12], rgb_panel_config.data_gpio_nums[13], rgb_panel_config.data_gpio_nums[14], rgb_panel_config.data_gpio_nums[15], rgb_panel_config.disp_gpio_num, rgb_panel_config.flags.disp_active_low, rgb_panel_config.flags.refresh_on_demand, rgb_panel_config.flags.fb_in_psram, rgb_panel_config.flags.double_fb, rgb_panel_config.flags.no_fb, rgb_panel_config.flags.bb_invalidate_cache);
    log_d("refresh rate: %d Hz", (ST7262_PANEL_CONFIG_TIMINGS_PCLK_HZ * 16) / (ST7262_PANEL_CONFIG_TIMINGS_H_RES + ST7262_PANEL_CONFIG_TIMINGS_HSYNC_PULSE_WIDTH + ST7262_PANEL_CONFIG_TIMINGS_HSYNC_BACK_PORCH + ST7262_PANEL_CONFIG_TIMINGS_HSYNC_FRONT_PORCH) / (ST7262_PANEL_CONFIG_TIMINGS_V_RES + ST7262_PANEL_CONFIG_TIMINGS_VSYNC_PULSE_WIDTH + ST7262_PANEL_CONFIG_TIMINGS_VSYNC_BACK_PORCH + ST7262_PANEL_CONFIG_TIMINGS_VSYNC_FRONT_PORCH) / SOC_LCD_RGB_DATA_WIDTH);
    esp_lcd_panel_handle_t panel_handle;
    ESP_ERROR_CHECK(esp_lcd_new_rgb_panel(&rgb_panel_config, &panel_handle));

    // Initialize DMA for this panel (async_color_trans = false for RGB panels which use built-in DMA)
    ESP_ERROR_CHECK(smartdisplay_dma_init_with_logging(panel_handle, "ST7262", false));

    lvgl_setup_panel(panel_handle);
    display->user_data = panel_handle;
    display->flush_cb = lv_flush_software;
    return display;
}

#endif