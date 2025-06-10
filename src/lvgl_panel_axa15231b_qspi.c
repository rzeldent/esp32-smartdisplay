#ifdef DISPLAY_AXS15231B_QSPI

#include <esp32_smartdisplay.h>
#include <esp_panel_axs15231b.h>
#include <driver/spi_master.h>
#include <esp_lcd_panel_io.h>
#include <esp_lcd_panel_ops.h>

bool axs15231b_color_trans_done(esp_lcd_panel_io_handle_t panel_io_handle, esp_lcd_panel_io_event_data_t *panel_io_event_data, void *user_ctx)
{
    log_v("panel_io_handle:0x%08x, panel_io_event_data:%0x%08x, user_ctx:0x%08x", panel_io_handle, panel_io_event_data, user_ctx);

    lv_display_t *display = user_ctx;
    lv_display_flush_ready(display);
    return false;
}

void axs15231b_lv_flush(lv_display_t *display, const lv_area_t *area, uint8_t *px_map)
{
    // Hardware rotation is supported
    log_v("display:0x%08x, area:%0x%08x, color_map:0x%08x", display, area, px_map);

    esp_lcd_panel_handle_t panel_handle = display->user_data;
    uint32_t pixels = lv_area_get_size(area);
    uint16_t *p = (uint16_t *)px_map;
    while (pixels--)
    {
        *p = (uint16_t)((*p >> 8) | (*p << 8));
        p++;
    }

    ESP_ERROR_CHECK(esp_lcd_panel_draw_bitmap(panel_handle, area->x1, area->y1, area->x2 + 1, area->y2 + 1, px_map));
}

lv_display_t *lvgl_lcd_init()
{
    lv_display_t *display = lv_display_create(DISPLAY_WIDTH, DISPLAY_HEIGHT);
    log_v("display:0x%08x", display);
    //  Create drawBuffer
    uint32_t drawBufferSize = sizeof(lv_color_t) * LVGL_BUFFER_PIXELS;
    void *drawBuffer = heap_caps_malloc(drawBufferSize, LVGL_BUFFER_MALLOC_FLAGS);
    lv_display_set_buffers(display, drawBuffer, NULL, drawBufferSize, LV_DISPLAY_RENDER_MODE_PARTIAL);

    // Create QSPI bus
    const spi_bus_config_t spi_bus_config = {
        .sclk_io_num = AXS15231B_SPI_BUS_PCLK,
        .data0_io_num = AXS15231B_SPI_BUS_DATA0,
        .data1_io_num = AXS15231B_SPI_BUS_DATA1,
        .data2_io_num = AXS15231B_SPI_BUS_DATA2,
        .data3_io_num = AXS15231B_SPI_BUS_DATA3,
        .max_transfer_sz = AXS15231B_SPI_BUS_MAX_TRANSFER_SZ,
        .flags = AXS15231B_SPI_BUS_FLAGS,
        .intr_flags = AXS15231B_SPI_BUS_INTR_FLAGS};
    log_d("spi_bus_config: sclk_io_num:%d, data0_io_num:%d, data1_io_num:%d, data2_io_num:%d, data3_io_num:%d, max_transfer_sz:%d, flags:0x%08x, intr_flags:0x%04x", spi_bus_config.sclk_io_num, spi_bus_config.data0_io_num, spi_bus_config.data1_io_num, spi_bus_config.data2_io_num, spi_bus_config.data3_io_num, spi_bus_config.max_transfer_sz, spi_bus_config.flags, spi_bus_config.intr_flags);
    ESP_ERROR_CHECK_WITHOUT_ABORT(spi_bus_initialize(AXS15231B_SPI_HOST, &spi_bus_config, AXS15231B_SPI_DMA_CHANNEL));

    // Attach the LCD controller to the SPI bus
    const esp_lcd_panel_io_spi_config_t io_spi_config = {
        .cs_gpio_num = AXS15231B_SPI_CONFIG_CS,
        .dc_gpio_num = AXS15231B_SPI_CONFIG_DC,
        .spi_mode = SPI_MODE0, // AXS15231B_SPI_CONFIG_SPI_MODE - QSPI not yet supported
        .pclk_hz = AXS15231B_SPI_CONFIG_PCLK_HZ,
        .trans_queue_depth = AXS15231B_SPI_CONFIG_TRANS_QUEUE_DEPTH,
        .user_ctx = display,
        .on_color_trans_done = axs15231b_color_trans_done,
        .lcd_cmd_bits = 8, // AXS15231B_SPI_CONFIG_LCD_CMD_BITS - QSPI not yet supported
        .lcd_param_bits = AXS15231B_SPI_CONFIG_LCD_PARAM_BITS,
        .flags = {
            .dc_as_cmd_phase = AXS15231B_SPI_CONFIG_FLAGS_DC_AS_CMD_PHASE,
            .dc_low_on_data = AXS15231B_SPI_CONFIG_FLAGS_DC_LOW_ON_DATA,
            .octal_mode = AXS15231B_SPI_CONFIG_FLAGS_OCTAL_MODE,
            .lsb_first = AXS15231B_SPI_CONFIG_FLAGS_LSB_FIRST}};
    log_d("io_spi_config: cs_gpio_num:%d, dc_gpio_num:%d, spi_mode:%d, pclk_hz:%d, trans_queue_depth:%d, user_ctx:0x%08x, on_color_trans_done:0x%08x, lcd_cmd_bits:%d, lcd_param_bits:%d, flags:{dc_as_cmd_phase:%d, dc_low_on_data:%d, octal_mode:%d, lsb_first:%d}", io_spi_config.cs_gpio_num, io_spi_config.dc_gpio_num, io_spi_config.spi_mode, io_spi_config.pclk_hz, io_spi_config.trans_queue_depth, io_spi_config.user_ctx, io_spi_config.on_color_trans_done, io_spi_config.lcd_cmd_bits, io_spi_config.lcd_param_bits, io_spi_config.flags.dc_as_cmd_phase, io_spi_config.flags.dc_low_on_data, io_spi_config.flags.octal_mode, io_spi_config.flags.lsb_first);

    // Attach the LCD controller to the QSPI bus
    // const esp_lcd_panel_io_spi_config_t io_spi_config = {
    //     .cs_gpio_num = AXS15231B_SPI_CONFIG_CS,
    //     .dc_gpio_num = AXS15231B_SPI_CONFIG_DC,
    //     .spi_mode = AXS15231B_SPI_CONFIG_SPI_MODE,
    //     .pclk_hz = AXS15231B_SPI_CONFIG_PCLK_HZ,
    //     .trans_queue_depth = AXS15231B_SPI_CONFIG_TRANS_QUEUE_DEPTH,
    //     .user_ctx = display,
    //     .on_color_trans_done = axs15231b_color_trans_done,
    //     .lcd_cmd_bits = AXS15231B_SPI_CONFIG_LCD_CMD_BITS,
    //     .lcd_param_bits = AXS15231B_SPI_CONFIG_LCD_PARAM_BITS,
    //     .flags = {
    //         .dc_as_cmd_phase = AXS15231B_SPI_CONFIG_FLAGS_DC_AS_CMD_PHASE,
    //         .dc_low_on_data = AXS15231B_SPI_CONFIG_FLAGS_DC_LOW_ON_DATA,
    //         .octal_mode = AXS15231B_SPI_CONFIG_FLAGS_OCTAL_MODE,
    //         .quad_mode = AXS15231B_SPI_CONFIG_FLAGS_QUAD_MODE,
    //         .sio_mode = AXS15231B_SPI_CONFIG_FLAGS_SIO_MODE,
    //         .lsb_first = AXS15231B_SPI_CONFIG_FLAGS_LSB_FIRST}};
    // //log_d("io_spi_config: cs_gpio_num:%d, dc_gpio_num:%d, spi_mode:%d, pclk_hz:%d, trans_queue_depth:%d, user_ctx:0x%08x, on_color_trans_done:0x%08x, lcd_cmd_bits:%d, lcd_param_bits:%d, flags:{dc_as_cmd_phase:%d, dc_low_on_data:%d, octal_mode:%d, quad_mode:%d, sio_mode:%d, lsb_first:%d}", io_spi_config.cs_gpio_num, io_spi_config.dc_gpio_num, io_spi_config.spi_mode, io_spi_config.pclk_hz, io_spi_config.trans_queue_depth, io_spi_config.user_ctx, io_spi_config.on_color_trans_done, io_spi_config.lcd_cmd_bits, io_spi_config.lcd_param_bits, io_spi_config.flags.dc_as_cmd_phase, io_spi_config.flags.dc_low_on_data, io_spi_config.flags.octal_mode, io_spi_config.quad_mode, io_spi_config.sio_mode, io_spi_config.flags.lsb_first);
    
    esp_lcd_panel_io_handle_t io_handle;
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)AXS15231B_SPI_HOST, &io_spi_config, &io_handle));

    // Create axs15231b panel handle
    const esp_lcd_panel_dev_config_t panel_dev_config = {
        .reset_gpio_num = AXS15231B_DEV_CONFIG_RESET,
        .color_space = AXS15231B_DEV_CONFIG_COLOR_SPACE,
        .bits_per_pixel = AXS15231B_DEV_CONFIG_BITS_PER_PIXEL,
        .flags = {
            .reset_active_high = AXS15231B_DEV_CONFIG_FLAGS_RESET_ACTIVE_HIGH},
        .vendor_config = AXS15231B_DEV_CONFIG_VENDOR_CONFIG};
    log_d("panel_dev_config: reset_gpio_num:%d, color_space:%d, bits_per_pixel:%d, flags:{reset_active_high:%d}, vendor_config: 0x%08x", panel_dev_config.reset_gpio_num, panel_dev_config.color_space, panel_dev_config.bits_per_pixel, panel_dev_config.flags.reset_active_high, panel_dev_config.vendor_config);
    esp_lcd_panel_handle_t panel_handle;
    ESP_ERROR_CHECK(esp_lcd_new_panel_axs15231b(io_handle, &panel_dev_config, &panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));
#ifdef DISPLAY_IPS
    // If LCD is IPS invert the colors
    ESP_ERROR_CHECK(esp_lcd_panel_invert_color(panel_handle, true));
#endif
#if (DISPLAY_SWAP_XY)
    ESP_ERROR_CHECK(esp_lcd_panel_swap_xy(panel_handle, DISPLAY_SWAP_XY));
#endif
#if (DISPLAY_MIRROR_X || DISPLAY_MIRROR_Y)
    ESP_ERROR_CHECK(esp_lcd_panel_mirror(panel_handle, DISPLAY_MIRROR_X, DISPLAY_MIRROR_Y));
#endif
#if (DISPLAY_GAP_X || DISPLAY_GAP_Y)
    ESP_ERROR_CHECK(esp_lcd_panel_set_gap(panel_handle, DISPLAY_GAP_X, DISPLAY_GAP_Y));
#endif
    // Turn display on
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, true));

    display->user_data = panel_handle;
    display->flush_cb = axs15231b_lv_flush;

    return display;
}

#endif