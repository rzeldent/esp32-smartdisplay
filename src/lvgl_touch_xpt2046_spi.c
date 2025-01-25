#ifdef TOUCH_XPT2046_SPI

#include <esp32_smartdisplay.h>
#include <esp_touch_xpt2046.h>
#include <driver/spi_master.h>
#include <driver/spi_common_internal.h>

void xpt2046_lvgl_touch_cb(lv_indev_t *indev, lv_indev_data_t *data)
{
    esp_lcd_touch_handle_t touch_handle = indev->user_data;

    uint16_t x[1];
    uint16_t y[1];
    uint8_t touch_cnt = 0;

    // Read touch controller data
    ESP_ERROR_CHECK(esp_lcd_touch_read_data(touch_handle));
    // Get coordinates
    bool pressed = esp_lcd_touch_get_coordinates(touch_handle, x, y, NULL, &touch_cnt, 1);
    if (pressed && touch_cnt > 0)
    {
        data->point.x = x[0];
        data->point.y = y[0];
        data->state = LV_INDEV_STATE_PRESSED;
        log_v("Pressed at: (%d,%d)", data->point.x, data->point.y);
    }
    else
        data->state = LV_INDEV_STATE_RELEASED;
}

lv_indev_t *lvgl_touch_init()
{
    lv_indev_t *indev = lv_indev_create();
    log_v("indev:0x%08x", indev);

    // Create SPI bus only if not already initialized (S035R shares the SPI bus)
    if(spi_bus_get_attr(XPT2046_SPI_HOST) == NULL) {
        const spi_bus_config_t spi_bus_config = {
            .mosi_io_num = XPT2046_SPI_BUS_MOSI_IO_NUM,
            .miso_io_num = XPT2046_SPI_BUS_MISO_IO_NUM,
            .sclk_io_num = XPT2046_SPI_BUS_SCLK_IO_NUM,
            .quadwp_io_num = XPT2046_SPI_BUS_QUADWP_IO_NUM,
            .quadhd_io_num = XPT2046_SPI_BUS_QUADHD_IO_NUM};
        log_d("spi_bus_config: mosi_io_num:%d, miso_io_num:%d, sclk_io_num:%d, quadwp_io_num:%d, quadhd_io_num:%d, max_transfer_sz:%d, flags:0x%08x, intr_flags:0x%04x", spi_bus_config.mosi_io_num, spi_bus_config.miso_io_num, spi_bus_config.sclk_io_num, spi_bus_config.quadwp_io_num, spi_bus_config.quadhd_io_num, spi_bus_config.max_transfer_sz, spi_bus_config.flags, spi_bus_config.intr_flags);
        ESP_ERROR_CHECK_WITHOUT_ABORT(spi_bus_initialize(XPT2046_SPI_HOST, &spi_bus_config, XPT2046_SPI_DMA_CHANNEL));
    }

    // Attach the touch controller to the SPI bus
    const esp_lcd_panel_io_spi_config_t io_spi_config = {
        .cs_gpio_num = XPT2046_SPI_CONFIG_CS_GPIO_NUM,
        .dc_gpio_num = XPT2046_SPI_CONFIG_DC_GPIO_NUM,
        .spi_mode = XPT2046_SPI_CONFIG_SPI_MODE,
        .pclk_hz = XPT2046_SPI_CONFIG_PCLK_HZ,
        .user_ctx = indev,
        .trans_queue_depth = XPT2046_SPI_CONFIG_TRANS_QUEUE_DEPTH,
        .lcd_cmd_bits = XPT2046_SPI_CONFIG_LCD_CMD_BITS,
        .lcd_param_bits = XPT2046_SPI_CONFIG_LCD_PARAM_BITS,
        .flags = {
            .dc_as_cmd_phase = XPT2046_SPI_CONFIG_FLAGS_DC_AS_CMD_PHASE,
            .dc_low_on_data = XPT2046_SPI_CONFIG_FLAGS_DC_LOW_ON_DATA,
            .octal_mode = XPT2046_SPI_CONFIG_FLAGS_OCTAL_MODE,
            .lsb_first = XPT2046_SPI_CONFIG_FLAGS_LSB_FIRST}};
    log_d("io_spi_config: cs_gpio_num:%d, dc_gpio_num:%d, spi_mode:%d, pclk_hz:%d, trans_queue_depth:%d, user_ctx:0x%08x, on_color_trans_done:0x%08x, lcd_cmd_bits:%d, lcd_param_bits:%d, flags:{dc_as_cmd_phase:%d, dc_low_on_data:%d, octal_mode:%d, lsb_first:%d}", io_spi_config.cs_gpio_num, io_spi_config.dc_gpio_num, io_spi_config.spi_mode, io_spi_config.pclk_hz, io_spi_config.trans_queue_depth, io_spi_config.user_ctx, io_spi_config.on_color_trans_done, io_spi_config.lcd_cmd_bits, io_spi_config.lcd_param_bits, io_spi_config.flags.dc_as_cmd_phase, io_spi_config.flags.dc_low_on_data, io_spi_config.flags.octal_mode, io_spi_config.flags.lsb_first);
    esp_lcd_panel_io_handle_t io_handle;
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)XPT2046_SPI_HOST, &io_spi_config, &io_handle));

    // Create touch configuration
    const esp_lcd_touch_config_t touch_config = {
        .x_max = XPT2046_TOUCH_CONFIG_X_MAX,
        .y_max = XPT2046_TOUCH_CONFIG_Y_MAX,
        .rst_gpio_num = XPT2046_TOUCH_CONFIG_RST_GPIO_NUM,
        .int_gpio_num = XPT2046_TOUCH_CONFIG_INT_GPIO_NUM,
        .levels = {
            .reset = XPT2046_TOUCH_CONFIG_LEVELS_RESET,
            .interrupt = XPT2046_TOUCH_CONFIG_LEVELS_INTERRUPT},
        .flags = {.swap_xy = TOUCH_SWAP_XY, .mirror_x = TOUCH_MIRROR_X, .mirror_y = TOUCH_MIRROR_Y},
        .user_data = io_handle};
    log_d("touch_config: x_max:%d, y_max:%d, rst_gpio_num:%d, int_gpio_num:%d, levels:{reset:%d, interrupt:%d}, flags:{swap_xy:%d, mirror_x:%d, mirror_y:%d}, user_data:0x%08x", touch_config.x_max, touch_config.y_max, touch_config.rst_gpio_num, touch_config.int_gpio_num, touch_config.levels.reset, touch_config.levels.interrupt, touch_config.flags.swap_xy, touch_config.flags.mirror_x, touch_config.flags.mirror_y, touch_config.user_data);
    esp_lcd_touch_handle_t touch_handle;
    ESP_ERROR_CHECK(esp_lcd_touch_new_spi_xpt2046(io_handle, &touch_config, &touch_handle));

    indev->type = LV_INDEV_TYPE_POINTER;
    indev->user_data = touch_handle;
    indev->read_cb = xpt2046_lvgl_touch_cb;

    return indev;
}

#endif
