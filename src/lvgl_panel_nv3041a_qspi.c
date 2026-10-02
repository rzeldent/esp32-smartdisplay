#ifdef DISPLAY_NV3041A_QSPI

#include <esp32_smartdisplay.h>
#include <esp_panel_nv3041a.h>
#include <driver/spi_master.h>
#include <esp_lcd_panel_ops.h>
#include <lvgl_panel_common.h>

lv_display_t *lvgl_lcd_init()
{
    lv_display_t *display = lvgl_create_display();
    log_v("display: %p", display);

    // Initialize QSPI bus with 4 data lines
    const spi_bus_config_t spi_bus_config = {
        .sclk_io_num = NV3041A_SPI_BUS_PCLK,
        .data0_io_num = NV3041A_SPI_BUS_DATA0,
        .data1_io_num = NV3041A_SPI_BUS_DATA1,
        .data2_io_num = NV3041A_SPI_BUS_DATA2,
        .data3_io_num = NV3041A_SPI_BUS_DATA3,
        .max_transfer_sz = NV3041A_SPI_BUS_MAX_TRANSFER_SZ,
        .flags = NV3041A_SPI_BUS_FLAGS,
        .intr_flags = NV3041A_SPI_BUS_INTR_FLAGS};
    log_d("spi_bus_config: sclk_io_num:%d, data0_io_num:%d, data1_io_num:%d, data2_io_num:%d, data3_io_num:%d, max_transfer_sz:%d, flags:0x%08x, intr_flags:0x%04x",
          spi_bus_config.sclk_io_num, spi_bus_config.data0_io_num, spi_bus_config.data1_io_num,
          spi_bus_config.data2_io_num, spi_bus_config.data3_io_num, spi_bus_config.max_transfer_sz,
          spi_bus_config.flags, spi_bus_config.intr_flags);
    ESP_ERROR_CHECK_WITHOUT_ABORT(spi_bus_initialize(NV3041A_SPI_HOST, &spi_bus_config, SPI_DMA_CH_AUTO));

    // Add SPI device directly — required for SPI_TRANS_MODE_QIO (QSPI pixel data).
    // esp_lcd_panel_io_spi is NOT used because it does not support quad_mode in ESP-IDF v4.4.
    // NV3041A QSPI protocol: cmd=8-bit opcode (0x02/0x32), addr=24-bit (register<<8).
    // spics_io_num=-1: CS is controlled manually via GPIO in esp_panel_nv3041a.c.
    const spi_device_interface_config_t spi_dev_cfg = {
        .command_bits = NV3041A_SPI_CONFIG_LCD_CMD_BITS,
        .address_bits = 24,
        .dummy_bits = 0,
        .mode = NV3041A_SPI_CONFIG_SPI_MODE,
        .clock_speed_hz = NV3041A_SPI_CONFIG_PCLK_HZ,
        .spics_io_num = -1,
        .queue_size = 1,
        .flags = SPI_DEVICE_HALFDUPLEX,
    };
    log_d("spi_dev_cfg: command_bits:%d, address_bits:24, mode:%d, clock_speed_hz:%d, spics_io_num:%d, queue_size:%d",
          spi_dev_cfg.command_bits, spi_dev_cfg.mode, spi_dev_cfg.clock_speed_hz,
          spi_dev_cfg.spics_io_num, spi_dev_cfg.queue_size);
    spi_device_handle_t spi_dev;
    ESP_ERROR_CHECK(spi_bus_add_device(NV3041A_SPI_HOST, &spi_dev_cfg, &spi_dev));

    // Create NV3041A panel using direct SPI handle
    const esp_lcd_panel_dev_config_t panel_dev_config = {
        .reset_gpio_num = NV3041A_DEV_CONFIG_RESET,
        .color_space = NV3041A_DEV_CONFIG_COLOR_SPACE,
        .bits_per_pixel = NV3041A_DEV_CONFIG_BITS_PER_PIXEL,
        .flags = {
            .reset_active_high = NV3041A_DEV_CONFIG_FLAGS_RESET_ACTIVE_HIGH},
        .vendor_config = NV3041A_DEV_CONFIG_VENDOR_CONFIG};
    log_d("panel_dev_config: reset_gpio_num:%d, color_space:%d, bits_per_pixel:%d, flags:{reset_active_high:%d}, vendor_config: 0x%08x",
          panel_dev_config.reset_gpio_num, panel_dev_config.color_space, panel_dev_config.bits_per_pixel,
          panel_dev_config.flags.reset_active_high, panel_dev_config.vendor_config);
    esp_lcd_panel_handle_t panel_handle;
    ESP_ERROR_CHECK(esp_lcd_new_panel_nv3041a(spi_dev, NV3041A_SPI_CONFIG_CS, &panel_dev_config, &panel_handle));

    lvgl_setup_panel(panel_handle);
    display->user_data = panel_handle;
    display->flush_cb = lv_flush_hardware;
    return display;
}

#endif
