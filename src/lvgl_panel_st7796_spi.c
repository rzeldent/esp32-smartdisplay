#ifdef DISPLAY_ST7796_SPI

#include <esp32_smartdisplay.h>
#include <esp_panel_st7796.h>
#include <driver/spi_master.h>
#include <esp_lcd_panel_io.h>
#include <esp_lcd_panel_vendor.h>
#include <esp_lcd_panel_ops.h>
#include <esp32_smartdisplay_dma_helpers.h>

bool st7796_color_trans_done(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_io_event_data_t *edata, void *user_ctx)
{
    // Note: When using DMA, lv_display_flush_ready() is called by DMA callbacks
    // This callback is only used for direct transfers (non-DMA fallback)
    // We return false to indicate we're not handling the flush completion here
    return false;
}

void st7796_lv_flush(lv_display_t *display, const lv_area_t *area, uint8_t *px_map)
{
    // Hardware rotation is supported - use optimized helper function
    esp_lcd_panel_handle_t panel_handle = display->user_data;
    smartdisplay_dma_flush_with_byteswap(display, area, px_map, panel_handle, "ST7796 SPI");
}

lv_display_t *lvgl_lcd_init(uint32_t hor_res, uint32_t ver_res)
{
    lv_display_t *display = lv_display_create(DISPLAY_WIDTH, DISPLAY_HEIGHT);
    log_v("display:0x%08x", display);
    //  Create drawBuffer
    uint32_t drawBufferSize = sizeof(lv_color_t) * LVGL_BUFFER_PIXELS;
    void *drawBuffer = heap_caps_malloc(drawBufferSize, LVGL_BUFFER_MALLOC_FLAGS);
    lv_display_set_buffers(display, drawBuffer, NULL, drawBufferSize, LV_DISPLAY_RENDER_MODE_PARTIAL);

    // Create SPI bus
    const spi_bus_config_t spi_bus_config = {
        .mosi_io_num = ST7796_SPI_BUS_MOSI,
        .miso_io_num = ST7796_SPI_BUS_MISO,
        .sclk_io_num = ST7796_SPI_BUS_SCLK,
        .quadwp_io_num = ST7796_SPI_BUS_QUADWP,
        .quadhd_io_num = ST7796_SPI_BUS_QUADHD,
        .max_transfer_sz = ST7796_SPI_BUS_MAX_TRANSFER_SZ,
        .flags = ST7796_SPI_BUS_FLAGS,
        .intr_flags = ST7796_SPI_BUS_INTR_FLAGS};
    log_d("spi_bus_config: mosi_io_num:%d, miso_io_num:%d, sclk_io_num:%d, quadwp_io_num:%d, quadhd_io_num:%d, max_transfer_sz:%d, flags:0x%08x, intr_flags:0x%04x", spi_bus_config.mosi_io_num, spi_bus_config.miso_io_num, spi_bus_config.sclk_io_num, spi_bus_config.quadwp_io_num, spi_bus_config.quadhd_io_num, spi_bus_config.max_transfer_sz, spi_bus_config.flags, spi_bus_config.intr_flags);
    ESP_ERROR_CHECK_WITHOUT_ABORT(spi_bus_initialize(ST7796_SPI_HOST, &spi_bus_config, ST7796_SPI_DMA_CHANNEL));

    // Attach the LCD controller to the SPI bus
    const esp_lcd_panel_io_spi_config_t io_spi_config = {
        .cs_gpio_num = ST7796_SPI_CONFIG_CS,
        .dc_gpio_num = ST7796_SPI_CONFIG_DC,
        .spi_mode = ST7796_SPI_CONFIG_SPI_MODE,
        .pclk_hz = ST7796_SPI_CONFIG_PCLK_HZ,
        .on_color_trans_done = st7796_color_trans_done,
        .user_ctx = display,
        .trans_queue_depth = ST7796_SPI_CONFIG_TRANS_QUEUE_DEPTH,
        .lcd_cmd_bits = ST7796_SPI_CONFIG_LCD_CMD_BITS,
        .lcd_param_bits = ST7796_SPI_CONFIG_LCD_PARAM_BITS,
        .flags = {
            .dc_low_on_data = ST7796_SPI_CONFIG_FLAGS_DC_LOW_ON_DATA,
            .octal_mode = ST7796_SPI_CONFIG_FLAGS_OCTAL_MODE,
            .lsb_first = ST7796_SPI_CONFIG_FLAGS_LSB_FIRST}};
    log_d("io_spi_config: cs_gpio_num:%d, dc_gpio_num:%d, spi_mode:%d, pclk_hz:%d, trans_queue_depth:%d, user_ctx:0x%08x, on_color_trans_done:0x%08x, lcd_cmd_bits:%d, lcd_param_bits:%d, flags:{dc_low_on_data:%d, octal_mode:%d, lsb_first:%d}", io_spi_config.cs_gpio_num, io_spi_config.dc_gpio_num, io_spi_config.spi_mode, io_spi_config.pclk_hz, io_spi_config.trans_queue_depth, io_spi_config.user_ctx, io_spi_config.on_color_trans_done, io_spi_config.lcd_cmd_bits, io_spi_config.lcd_param_bits, io_spi_config.flags.dc_low_on_data, io_spi_config.flags.octal_mode, io_spi_config.flags.lsb_first);
    esp_lcd_panel_io_handle_t io_handle;
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)ST7796_SPI_HOST, &io_spi_config, &io_handle));

    // Create st7796 panel handle
    const esp_lcd_panel_dev_config_t panel_dev_config = {
        .reset_gpio_num = ST7796_DEV_CONFIG_RESET,
        .color_space = ST7796_DEV_CONFIG_COLOR_SPACE,
        .bits_per_pixel = ST7796_DEV_CONFIG_BITS_PER_PIXEL,
        .flags = {
            .reset_active_high = ST7796_DEV_CONFIG_FLAGS_RESET_ACTIVE_HIGH},
        .vendor_config = ST7796_DEV_CONFIG_VENDOR_CONFIG};
    log_d("panel_dev_config: reset_gpio_num:%d, color_space:%d, bits_per_pixel:%d, flags:{reset_active_high:%d}, vendor_config:0x%08x", panel_dev_config.reset_gpio_num, panel_dev_config.color_space, panel_dev_config.bits_per_pixel, panel_dev_config.flags.reset_active_high, panel_dev_config.vendor_config);
    esp_lcd_panel_handle_t panel_handle;
    ESP_ERROR_CHECK(esp_lcd_new_panel_st7796(io_handle, &panel_dev_config, &panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));
    
    // Initialize DMA for optimized transfers
    smartdisplay_dma_init_with_logging(panel_handle, "ST7796 SPI");
    
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
    display->flush_cb = st7796_lv_flush;

    return display;
}

#endif