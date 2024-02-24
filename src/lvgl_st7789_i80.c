#ifdef LCD_ST7789_I80

#include <esp32_smartdisplay.h>
#include <esp_lcd_panel_io.h>
#include <esp_lcd_panel_vendor.h>
#include <esp_lcd_panel_ops.h>

static bool st7789_color_trans_done(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_io_event_data_t *edata, void *user_ctx)
{
    lv_disp_drv_t *disp_driver = user_ctx;
    lv_disp_flush_ready(disp_driver);
    return false;
}

static void st7789_lv_flush(lv_disp_drv_t *drv, const lv_area_t *area, lv_color16_t *color_map)
{
    esp_lcd_panel_handle_t panel_handle = drv->user_data;
#if LV_COLOR_16_SWAP != 1
#warning "LV_COLOR_16_SWAP should be 1 for max performance"
    ushort pixels = lv_area_get_size(area);
    lv_color16_t *p = color_map;
    while (pixels--)
        p++->full = (uint16_t)((p->full >> 8) | (p->full << 8));
#endif
    ESP_ERROR_CHECK(esp_lcd_panel_draw_bitmap(panel_handle, area->x1, area->y1, area->x2 + 1, area->y2 + 1, color_map));
};

void lvgl_lcd_init(lv_disp_drv_t *drv)
{
    log_d("lvgl_lcd_init");
    // Hardware rotation is NOT supported
    drv->sw_rotate = 1;
    drv->rotated = LV_DISP_ROT_NONE;

    pinMode(ST7789_RD_GPIO, OUTPUT);
    digitalWrite(ST7789_RD_GPIO, HIGH);

    const esp_lcd_i80_bus_config_t bus_config = {
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
            ST7789_I80_BUS_CONFIG_DATA_GPIO_D15
        },
        .bus_width = ST7789_I80_BUS_CONFIG_BUS_WIDTH,
        // transfer 100 lines of pixels (assume pixel is RGB565) at most in one transaction
        .max_transfer_bytes = ST7789_I80_BUS_CONFIG_MAX_TRANSFER_BYTES,
        .psram_trans_align = ST7789_I80_BUS_CONFIG_PSRAM_TRANS_ALIGN,
        .sram_trans_align = ST7789_I80_BUS_CONFIG_SRAM_TRANS_ALIGN};
    esp_lcd_i80_bus_handle_t i80_bus;
    ESP_ERROR_CHECK(esp_lcd_new_i80_bus(&bus_config, &i80_bus));

    // Create direct_io panel handle
    esp_lcd_panel_io_i80_config_t io_config = {
        .cs_gpio_num = ST7789_IO_I80_CONFIG_CS_GPIO_NUM,
        .pclk_hz = ST7789_IO_I80_CONFIG_PCLK_HZ,
        .on_color_trans_done = st7789_color_trans_done,
        .user_ctx = drv,
        .trans_queue_depth = ST7789_IO_I80_CONFIG_TRANS_QUEUE_DEPTH,
        .lcd_cmd_bits = ST7789_IO_I80_CONFIG_LCD_CMD_BITS,
        .lcd_param_bits = ST7789_IO_I80_CONFIG_LCD_PARAM_BITS,
        .dc_levels = {
            .dc_idle_level = ST7789_IO_I80_CONFIG_DC_LEVELS_DC_IDLE_LEVEL,
            .dc_cmd_level = ST7789_IO_I80_CONFIG_DC_LEVELS_DC_CMD_LEVEL,
            .dc_dummy_level = ST7789_IO_I80_CONFIG_DC_LEVELS_DC_DUMMY_LEVEL,
            .dc_data_level = ST7789_IO_I80_CONFIG_DC_LEVELS_DC_DATA_LEVEL
        },
        .flags = {
            .cs_active_high = ST7789_IO_I80_CONFIG_FLAGS_CS_ACTIVE_HIGH,
            .reverse_color_bits = ST7789_IO_I80_CONFIG_FLAGS_REVERSE_COLOR_BITS,
            .swap_color_bytes = ST7789_IO_I80_CONFIG_FLAGS_SWAP_COLOR_BYTES,
            .pclk_active_neg = ST7789_IO_I80_CONFIG_FLAGS_PCLK_ACTIVE_NEG,
            .pclk_idle_low = ST7789_IO_I80_CONFIG_FLAGS_PCLK_IDLE_LOW}};
    esp_lcd_panel_io_handle_t io_handle;
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_i80(i80_bus, &io_config, &io_handle));

    // Create ST7789 panel handle
    const esp_lcd_panel_dev_config_t panel_dev_config = {
        .reset_gpio_num = ST7789_DEV_CONFIG_RESET_GPIO_NUM,
        .color_space = ST7789_DEV_CONFIG_COLOR_SPACE,
        .bits_per_pixel = ST7789_DEV_CONFIG_BITS_PER_PIXEL,
        .flags = {
            .reset_active_high = ST7789_DEV_CONFIG_FLAGS_RESET_ACTIVE_HIGH},
        .vendor_config = ST7789_DEV_CONFIG_VENDOR_CONFIG};
    esp_lcd_panel_handle_t panel_handle;
    ESP_ERROR_CHECK(esp_lcd_new_panel_st7789(io_handle, &panel_dev_config, &panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));
#ifdef LCD_IPS
    // If LCD is IPS invert the colors
    ESP_ERROR_CHECK(esp_lcd_panel_invert_color(panel_handle, true));
#endif

    drv->user_data = panel_handle;
    drv->flush_cb = st7789_lv_flush;
}

#endif