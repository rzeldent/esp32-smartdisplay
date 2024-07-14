#ifdef DISPLAY_ST7789_I80

#include <esp32_smartdisplay.h>
#include <esp_lcd_panel_io.h>
#include <esp_lcd_panel_vendor.h>
#include <esp_lcd_panel_ops.h>

bool st7789_color_trans_done(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_io_event_data_t *edata, void *user_ctx)
{
    lv_display_t *display = user_ctx;
    lv_display_flush_ready(display);
    return false;
}

void st7789_lv_flush(lv_display_t *drv, const lv_area_t *area, uint8_t *px_map)
{
    const esp_lcd_panel_handle_t panel_handle = drv->user_data;
    uint32_t pixels = lv_area_get_size(area);
    uint16_t *p = (uint16_t*)px_map;
    while (pixels--) {
        *p = (uint16_t)((*p >> 8) | (*p << 8));
        p++;
    }

    ESP_ERROR_CHECK(esp_lcd_panel_draw_bitmap(panel_handle, area->x1, area->y1, area->x2 + 1, area->y2 + 1, px_map));
};

lv_display_t *lvgl_lcd_init(uint32_t hor_res, uint32_t ver_res)
{
    lv_display_t *display = lv_display_create(hor_res, ver_res);
    log_v("display:0x%08x", display);

    // Hardware rotation is supported
    display->sw_rotate = 0;
    display->rotation = LV_DISPLAY_ROTATION_0;

    pinMode(ST7789_RD_GPIO, OUTPUT);
    digitalWrite(ST7789_RD_GPIO, HIGH);

    const esp_lcd_i80_bus_config_t i80_bus_config = {
        .clk_src = ST7789_I80_BUS_CONFIG_CLK_SRC,
        .dc_gpio_num = ST7789_I80_BUS_CONFIG_DC,
        .wr_gpio_num = ST7789_I80_BUS_CONFIG_WR,
        .data_gpio_nums = {
            ST7789_I80_BUS_CONFIG_DATA_GPIO_D8,
            ST7789_I80_BUS_CONFIG_DATA_GPIO_D9,
            ST7789_I80_BUS_CONFIG_DATA_GPIO_D10,
            ST7789_I80_BUS_CONFIG_DATA_GPIO_D11,
            ST7789_I80_BUS_CONFIG_DATA_GPIO_D12,
            ST7789_I80_BUS_CONFIG_DATA_GPIO_D13,
            ST7789_I80_BUS_CONFIG_DATA_GPIO_D14,
            ST7789_I80_BUS_CONFIG_DATA_GPIO_D15},
        .bus_width = ST7789_I80_BUS_CONFIG_BUS_WIDTH,
        // transfer 100 lines of pixels (assume pixel is RGB565) at most in one transaction
        .max_transfer_bytes = ST7789_I80_BUS_CONFIG_MAX_TRANSFER_BYTES,
        .psram_trans_align = ST7789_I80_BUS_CONFIG_PSRAM_TRANS_ALIGN,
        .sram_trans_align = ST7789_I80_BUS_CONFIG_SRAM_TRANS_ALIGN};
    log_d("i80_bus_config: clk_src:%d, dc_gpio_num:%d, wr_gpio_num:%d, data_gpio_nums:[%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d], bus_width:%d, max_transfer_bytes:%d, psram_trans_align:%d, sram_trans_align:%d", i80_bus_config.clk_src, i80_bus_config.dc_gpio_num, i80_bus_config.wr_gpio_num, i80_bus_config.data_gpio_nums[0], i80_bus_config.data_gpio_nums[1], i80_bus_config.data_gpio_nums[2], i80_bus_config.data_gpio_nums[3], i80_bus_config.data_gpio_nums[4], i80_bus_config.data_gpio_nums[5], i80_bus_config.data_gpio_nums[6], i80_bus_config.data_gpio_nums[7], i80_bus_config.data_gpio_nums[8], i80_bus_config.data_gpio_nums[9], i80_bus_config.data_gpio_nums[10], i80_bus_config.data_gpio_nums[11], i80_bus_config.data_gpio_nums[12], i80_bus_config.data_gpio_nums[13], i80_bus_config.data_gpio_nums[14], i80_bus_config.data_gpio_nums[15], i80_bus_config.data_gpio_nums[16], i80_bus_config.data_gpio_nums[17], i80_bus_config.data_gpio_nums[18], i80_bus_config.data_gpio_nums[19], i80_bus_config.data_gpio_nums[20], i80_bus_config.data_gpio_nums[21], i80_bus_config.data_gpio_nums[22], i80_bus_config.data_gpio_nums[23], i80_bus_config.bus_width, i80_bus_config.max_transfer_bytes, i80_bus_config.psram_trans_align, i80_bus_config.sram_trans_align);
    esp_lcd_i80_bus_handle_t i80_bus;
    ESP_ERROR_CHECK(esp_lcd_new_i80_bus(&i80_bus_config, &i80_bus));

    // Create direct_io panel handle
    esp_lcd_panel_io_i80_config_t io_i80_config = {
        .cs_gpio_num = ST7789_IO_I80_CONFIG_CS_GPIO_NUM,
        .pclk_hz = ST7789_IO_I80_CONFIG_PCLK_HZ,
        .on_color_trans_done = st7789_color_trans_done,
        .user_ctx = display,
        .trans_queue_depth = ST7789_IO_I80_CONFIG_TRANS_QUEUE_DEPTH,
        .lcd_cmd_bits = ST7789_IO_I80_CONFIG_LCD_CMD_BITS,
        .lcd_param_bits = ST7789_IO_I80_CONFIG_LCD_PARAM_BITS,
        .dc_levels = {
            .dc_idle_level = ST7789_IO_I80_CONFIG_DC_LEVELS_DC_IDLE_LEVEL,
            .dc_cmd_level = ST7789_IO_I80_CONFIG_DC_LEVELS_DC_CMD_LEVEL,
            .dc_dummy_level = ST7789_IO_I80_CONFIG_DC_LEVELS_DC_DUMMY_LEVEL,
            .dc_data_level = ST7789_IO_I80_CONFIG_DC_LEVELS_DC_DATA_LEVEL},
        .flags = {.cs_active_high = ST7789_IO_I80_CONFIG_FLAGS_CS_ACTIVE_HIGH, .reverse_color_bits = ST7789_IO_I80_CONFIG_FLAGS_REVERSE_COLOR_BITS, .swap_color_bytes = ST7789_IO_I80_CONFIG_FLAGS_SWAP_COLOR_BYTES, .pclk_active_neg = ST7789_IO_I80_CONFIG_FLAGS_PCLK_ACTIVE_NEG, .pclk_idle_low = ST7789_IO_I80_CONFIG_FLAGS_PCLK_IDLE_LOW}};
    log_d("io_i80_config: cs_gpio_num:%d, pclk_hz:%d, on_color_trans_done:0x%8x, user_ctx:0x%08x, trans_queue_depth:%d, lcd_cmd_bits:%d, lcd_param_bits:%d, dc_levels:{dc_idle_level:%d, dc_cmd_level:%d, dc_dummy_level:%d, dc_data_level:%d}, flags:{cs_active_high:%d, reverse_color_bits:%d, swap_color_bytes:%d, pclk_active_neg:%d, pclk_idle_low:%d}", io_i80_config.cs_gpio_num, io_i80_config.pclk_hz, io_i80_config.on_color_trans_done, io_i80_config.user_ctx, io_i80_config.trans_queue_depth, io_i80_config.lcd_cmd_bits, io_i80_config.lcd_param_bits, io_i80_config.dc_levels.dc_idle_level, io_i80_config.dc_levels.dc_cmd_level, io_i80_config.dc_levels.dc_dummy_level, io_i80_config.dc_levels.dc_data_level, io_i80_config.flags.cs_active_high, io_i80_config.flags.reverse_color_bits, io_i80_config.flags.swap_color_bytes, io_i80_config.flags.pclk_active_neg, io_i80_config.flags.pclk_idle_low);
    esp_lcd_panel_io_handle_t io_handle;
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_i80(i80_bus, &io_i80_config, &io_handle));

    // Create ST7789 panel handle
    const esp_lcd_panel_dev_config_t panel_dev_config = {
        .reset_gpio_num = ST7789_DEV_CONFIG_RESET_GPIO_NUM,
        .color_space = ST7789_DEV_CONFIG_COLOR_SPACE,
        .bits_per_pixel = ST7789_DEV_CONFIG_BITS_PER_PIXEL,
        .flags = {
            .reset_active_high = ST7789_DEV_CONFIG_FLAGS_RESET_ACTIVE_HIGH},
        .vendor_config = ST7789_DEV_CONFIG_VENDOR_CONFIG};
    log_d("panel_dev_config: reset_gpio_num:%d, color_space:%d, bits_per_pixel:%d, flags:{reset_active_high:%d}, vendor_config:0x%08x", panel_dev_config.reset_gpio_num, panel_dev_config.color_space, panel_dev_config.bits_per_pixel, panel_dev_config.flags.reset_active_high, panel_dev_config.vendor_config);
    esp_lcd_panel_handle_t panel_handle;
    ESP_ERROR_CHECK(esp_lcd_new_panel_st7789(io_handle, &panel_dev_config, &panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));
#ifdef DISPLAY_IPS
    // If LCD is IPS invert the colors
    ESP_ERROR_CHECK(esp_lcd_panel_invert_color(panel_handle, true));
#endif
#if defined(DISPLAY_GAP_X) || defined(DISPLAY_GAP_Y)
    ESP_ERROR_CHECK(esp_lcd_panel_set_gap(panel_handle, DISPLAY_GAP_X, DISPLAY_GAP_Y));
#endif
    display->user_data = panel_handle;
    display->flush_cb = st7789_lv_flush;

    return display;
}

#endif