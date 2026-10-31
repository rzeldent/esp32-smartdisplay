#ifdef DISPLAY_ST7701_PAR

#include <esp32_smartdisplay.h>
#include <esp_panel_st7701.h>
#include <esp_lcd_panel_io_additions.h>
#include <esp_lcd_panel_rgb.h>
#include <esp_lcd_panel_ops.h>
#include <lvgl_panel_common.h>

lv_display_t *lvgl_lcd_init()
{
    lv_display_t *display = lvgl_create_display();
    log_v("display: %p", display);

    // Install 3-wire SPI panel IO
    esp_lcd_panel_io_3wire_spi_config_t io_3wire_spi_config = {
        .line_config = {
            .cs_io_type = IO_TYPE_GPIO,
            .cs_gpio_num = ST7701_IO_3WIRE_SPI_LINE_CONFIG_CS,
            .scl_io_type = IO_TYPE_GPIO,
            .scl_gpio_num = ST7701_IO_3WIRE_SPI_LINE_CONFIG_SCL,
            .sda_io_type = IO_TYPE_GPIO,
            .sda_gpio_num = ST7701_IO_3WIRE_SPI_LINE_CONFIG_SDA},
        .expect_clk_speed = ST7701_IO_3WIRE_SPI_EXPECT_CLK_SPEED,
        .spi_mode = ST7701_IO_3WIRE_SPI_SPI_MODE,
        .lcd_cmd_bytes = ST7701_IO_3WIRE_SPI_LCD_CMD_BYTES,
        .lcd_param_bytes = ST7701_IO_3WIRE_SPI_LCD_PARAM_BYTES,
        .flags = {.use_dc_bit = ST7701_IO_3WIRE_SPI_FLAGS_USE_DC_BIT, .dc_zero_on_data = ST7701_IO_3WIRE_SPI_FLAGS_DC_ZERO_ON_DATA, .lsb_first = ST7701_IO_3WIRE_SPI_FLAGS_LSB_FIRST, .cs_high_active = ST7701_IO_3WIRE_SPI_FLAGS_CS_HIGH_ACTIVE, .del_keep_cs_inactive = ST7701_IO_3WIRE_SPI_FLAGS_DEL_KEEP_CS_INACTIVE}};
    log_d("io_3wire_spi_config: line_config:{cs_io_type: %d, cs_gpio_num: %d, scl_io_type: %d, scl_gpio_num: %d, sda_io_type: %d, sda_gpio_num: %d}, expect_clk_speed: %d, spi_mode: %d, lcd_cmd_bytes: %d, lcd_param_bytes: %d, flags:{use_dc_bit: %d, dc_zero_on_data: %d, lsb_first: %d, cs_high_active: %d, del_keep_cs_inactive: %d}", io_3wire_spi_config.line_config.cs_io_type, io_3wire_spi_config.line_config.cs_gpio_num, io_3wire_spi_config.line_config.scl_io_type, io_3wire_spi_config.line_config.scl_gpio_num, io_3wire_spi_config.line_config.sda_io_type, io_3wire_spi_config.line_config.sda_gpio_num, io_3wire_spi_config.expect_clk_speed, io_3wire_spi_config.spi_mode, io_3wire_spi_config.lcd_cmd_bytes, io_3wire_spi_config.lcd_param_bytes, io_3wire_spi_config.flags.use_dc_bit, io_3wire_spi_config.flags.dc_zero_on_data, io_3wire_spi_config.flags.lsb_first, io_3wire_spi_config.flags.cs_high_active, io_3wire_spi_config.flags.del_keep_cs_inactive);
    esp_lcd_panel_io_handle_t io_handle;
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_3wire_spi(&io_3wire_spi_config, &io_handle));

    // Create direct_io panel handle
    const esp_lcd_rgb_panel_config_t rgb_panel_config = {
        .clk_src = ST7701_PANEL_CONFIG_CLK_SRC,
        .timings = {
            .pclk_hz = ST7701_PANEL_CONFIG_TIMINGS_PCLK_HZ,
            .h_res = ST7701_PANEL_CONFIG_TIMINGS_H_RES,
            .v_res = ST7701_PANEL_CONFIG_TIMINGS_V_RES,
            .hsync_pulse_width = ST7701_PANEL_CONFIG_TIMINGS_HSYNC_PULSE_WIDTH,
            .hsync_back_porch = ST7701_PANEL_CONFIG_TIMINGS_HSYNC_BACK_PORCH,
            .hsync_front_porch = ST7701_PANEL_CONFIG_TIMINGS_HSYNC_FRONT_PORCH,
            .vsync_pulse_width = ST7701_PANEL_CONFIG_TIMINGS_VSYNC_PULSE_WIDTH,
            .vsync_back_porch = ST7701_PANEL_CONFIG_TIMINGS_VSYNC_BACK_PORCH,
            .vsync_front_porch = ST7701_PANEL_CONFIG_TIMINGS_VSYNC_FRONT_PORCH,
            .flags = {
                .hsync_idle_low = ST7701_PANEL_CONFIG_TIMINGS_FLAGS_HSYNC_IDLE_LOW,
                .vsync_idle_low = ST7701_PANEL_CONFIG_TIMINGS_FLAGS_VSYNC_IDLE_LOW,
                .de_idle_high = ST7701_PANEL_CONFIG_TIMINGS_FLAGS_DE_IDLE_HIGH,
                .pclk_active_neg = ST7701_PANEL_CONFIG_TIMINGS_FLAGS_PCLK_ACTIVE_NEG,
                .pclk_idle_high = ST7701_PANEL_CONFIG_TIMINGS_FLAGS_PCLK_IDLE_HIGH}},
        .data_width = ST7701_PANEL_CONFIG_DATA_WIDTH,
        .bits_per_pixel = ST7701_PANEL_CONFIG_BITS_PER_PIXEL,
        .num_fbs = ST7701_PANEL_CONFIG_NUM_FBS,
        .bounce_buffer_size_px = ST7701_PANEL_CONFIG_BOUNCE_BUFFER_SIZE_PX,
        .dma_burst_size = ST7701_PANEL_CONFIG_DMA_BURST_SIZE,
        .hsync_gpio_num = ST7701_PANEL_CONFIG_HSYNC,
        .vsync_gpio_num = ST7701_PANEL_CONFIG_VSYNC,
        .de_gpio_num = ST7701_PANEL_CONFIG_DE,
        .pclk_gpio_num = ST7701_PANEL_CONFIG_PCLK,
        .data_gpio_nums = {ST7701_PANEL_CONFIG_DATA_R0, ST7701_PANEL_CONFIG_DATA_R1, ST7701_PANEL_CONFIG_DATA_R2, ST7701_PANEL_CONFIG_DATA_R3, ST7701_PANEL_CONFIG_DATA_R4, ST7701_PANEL_CONFIG_DATA_G0, ST7701_PANEL_CONFIG_DATA_G1, ST7701_PANEL_CONFIG_DATA_G2, ST7701_PANEL_CONFIG_DATA_G3, ST7701_PANEL_CONFIG_DATA_G4, ST7701_PANEL_CONFIG_DATA_G5, ST7701_PANEL_CONFIG_DATA_B0, ST7701_PANEL_CONFIG_DATA_B1, ST7701_PANEL_CONFIG_DATA_B2, ST7701_PANEL_CONFIG_DATA_B3, ST7701_PANEL_CONFIG_DATA_B4},
        .disp_gpio_num = ST7701_PANEL_CONFIG_DISP,
        .flags = {.disp_active_low = ST7701_PANEL_CONFIG_FLAGS_DISP_ACTIVE_LOW, .refresh_on_demand = ST7701_PANEL_CONFIG_FLAGS_REFRESH_ON_DEMAND, .fb_in_psram = ST7701_PANEL_CONFIG_FLAGS_FB_IN_PSRAM, .double_fb = ST7701_PANEL_CONFIG_FLAGS_DOUBLE_FB, .no_fb = ST7701_PANEL_CONFIG_FLAGS_NO_FB, .bb_invalidate_cache = ST7701_PANEL_CONFIG_FLAGS_BB_INVALIDATE_CACHE}};
    log_d("rgb_panel_config: clk_src: %d, timings:{pclk_hz: %d, h_res: %d, v_res: %d, hsync_pulse_width: %d, hsync_back_porch: %d, hsync_front_porch: %d, vsync_pulse_width: %d, vsync_back_porch: %d, vsync_front_porch: %d, flags:{hsync_idle_low: %d, vsync_idle_low: %d, de_idle_high: %d, pclk_active_neg: %d, pclk_idle_high: %d}}, data_width: %d, bits_per_pixel: %d, num_fbs: %d, bounce_buffer_size_px: %d, dma_burst_size: %d, hsync_gpio_num: %d, vsync_gpio_num: %d, de_gpio_num: %d, pclk_gpio_num: %d, data_gpio_nums:[%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d], disp_gpio_num: %d, flags:{disp_active_low: %d, refresh_on_demand: %d, fb_in_psram: %d, double_fb: %d, no_fb: %d, bb_invalidate_cache: %d}", rgb_panel_config.clk_src, rgb_panel_config.timings.pclk_hz, rgb_panel_config.timings.h_res, rgb_panel_config.timings.v_res, rgb_panel_config.timings.hsync_pulse_width, rgb_panel_config.timings.hsync_back_porch, rgb_panel_config.timings.hsync_front_porch, rgb_panel_config.timings.vsync_pulse_width, rgb_panel_config.timings.vsync_back_porch, rgb_panel_config.timings.vsync_front_porch, rgb_panel_config.timings.flags.hsync_idle_low, rgb_panel_config.timings.flags.vsync_idle_low, rgb_panel_config.timings.flags.de_idle_high, rgb_panel_config.timings.flags.pclk_active_neg, rgb_panel_config.timings.flags.pclk_idle_high, rgb_panel_config.data_width, rgb_panel_config.bits_per_pixel, rgb_panel_config.num_fbs, rgb_panel_config.bounce_buffer_size_px, rgb_panel_config.dma_burst_size, rgb_panel_config.hsync_gpio_num, rgb_panel_config.vsync_gpio_num, rgb_panel_config.de_gpio_num, rgb_panel_config.pclk_gpio_num, rgb_panel_config.data_gpio_nums[0], rgb_panel_config.data_gpio_nums[1], rgb_panel_config.data_gpio_nums[2], rgb_panel_config.data_gpio_nums[3], rgb_panel_config.data_gpio_nums[4], rgb_panel_config.data_gpio_nums[5], rgb_panel_config.data_gpio_nums[6], rgb_panel_config.data_gpio_nums[7], rgb_panel_config.data_gpio_nums[8], rgb_panel_config.data_gpio_nums[9], rgb_panel_config.data_gpio_nums[10], rgb_panel_config.data_gpio_nums[11], rgb_panel_config.data_gpio_nums[12], rgb_panel_config.data_gpio_nums[13], rgb_panel_config.data_gpio_nums[14], rgb_panel_config.data_gpio_nums[15], rgb_panel_config.disp_gpio_num, rgb_panel_config.flags.disp_active_low, rgb_panel_config.flags.refresh_on_demand, rgb_panel_config.flags.fb_in_psram, rgb_panel_config.flags.double_fb, rgb_panel_config.flags.no_fb, rgb_panel_config.flags.bb_invalidate_cache);
    log_d("refresh rate: %d Hz", (ST7701_PANEL_CONFIG_TIMINGS_PCLK_HZ * 16) / (ST7701_PANEL_CONFIG_TIMINGS_H_RES + ST7701_PANEL_CONFIG_TIMINGS_HSYNC_PULSE_WIDTH + ST7701_PANEL_CONFIG_TIMINGS_HSYNC_BACK_PORCH + ST7701_PANEL_CONFIG_TIMINGS_HSYNC_FRONT_PORCH) / (ST7701_PANEL_CONFIG_TIMINGS_V_RES + ST7701_PANEL_CONFIG_TIMINGS_VSYNC_PULSE_WIDTH + ST7701_PANEL_CONFIG_TIMINGS_VSYNC_BACK_PORCH + ST7701_PANEL_CONFIG_TIMINGS_VSYNC_FRONT_PORCH) / SOC_LCD_RGB_DATA_WIDTH);
    const esp_lcd_panel_dev_config_t panel_dev_config = {
        .reset_gpio_num = ST7701_DEV_CONFIG_RESET,
        .color_space = ST7701_DEV_CONFIG_COLOR_SPACE,
        .bits_per_pixel = ST7701_DEV_CONFIG_BITS_PER_PIXEL,
        .flags = {
            .reset_active_high = ST7701_DEV_CONFIG_FLAGS_RESET_ACTIVE_HIGH},
        .vendor_config = ST7701_DEV_CONFIG_VENDOR_CONFIG};
    log_d("panel_dev_config: reset_gpio_num: %d, color_space: %d, bits_per_pixel: %d, flags:{reset_active_high: %d}, vendor_config: %p", panel_dev_config.reset_gpio_num, panel_dev_config.color_space, panel_dev_config.bits_per_pixel, panel_dev_config.flags.reset_active_high, panel_dev_config.vendor_config);
    esp_lcd_panel_handle_t panel_handle;
    ESP_ERROR_CHECK(esp_lcd_new_panel_st7701(io_handle, &rgb_panel_config, &panel_dev_config, &panel_handle));

    // Initialize DMA for this panel (async_color_trans = true for RGB panels with SPI control)
    ESP_ERROR_CHECK(smartdisplay_dma_init_with_logging(panel_handle, "ST7701", true));
 
    lvgl_setup_panel(panel_handle);
    display->user_data = panel_handle;
    display->flush_cb = lv_flush_software;
    return display;
}

#endif