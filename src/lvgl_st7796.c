#ifdef LCD_USES_ST7796

#include <esp32_smartdisplay.h>
#include <driver/spi_master.h>
#include <esp_lcd_panel_io.h>
#include <esp_lcd_panel_vendor.h>
#include <esp_lcd_panel_ops.h>

#include "esp_lcd_st7796.h"

static bool st7796_color_trans_done(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_io_event_data_t *edata, void *user_ctx)
{
    lv_disp_drv_t *disp_driver = user_ctx;
    lv_disp_flush_ready(disp_driver);
    return false;
}

static void st7796_lv_flush(lv_disp_drv_t *drv, const lv_area_t *area, lv_color16_t *color_map)
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
    // Hardware rotation is supported
    drv->sw_rotate = 0;
    drv->rotated = LV_DISP_ROT_NONE;

    // Create SPI bus
    const spi_bus_config_t spi_bus_config = {
        .mosi_io_num = ST7796_SPI_MOSI,
        .miso_io_num = ST7796_SPI_MISO,
        .sclk_io_num = ST7796_SPI_SCLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1};
    ESP_ERROR_CHECK_WITHOUT_ABORT(spi_bus_initialize(ST7796_SPI_HOST, &spi_bus_config, SPI_DMA_CH_AUTO));

    // Attach the LCD controller to the SPI bus
    const esp_lcd_panel_io_spi_config_t io_spi_config = {
        .cs_gpio_num = ST7796_CS,
        .dc_gpio_num = ST7796_DC,
        .spi_mode = SPI_MODE0,
        .pclk_hz = 24000000,
        .on_color_trans_done = st7796_color_trans_done,
        .user_ctx = drv,
        .trans_queue_depth = 10,
        .lcd_cmd_bits = 8,
        .lcd_param_bits = 8};
    esp_lcd_panel_io_handle_t io_handle;
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)ST7796_SPI_HOST, &io_spi_config, &io_handle));

    // Create st7796 panel handle
    const esp_lcd_panel_dev_config_t panel_dev_config = {
        .reset_gpio_num = GPIO_NUM_NC,
        .color_space = ESP_LCD_COLOR_SPACE_BGR,
        .bits_per_pixel = 16,
        .vendor_config = ST7796_VENDOR_CONFIG};
    esp_lcd_panel_handle_t panel_handle;
    ESP_ERROR_CHECK(esp_lcd_new_panel_st7796(io_handle, &panel_dev_config, &panel_handle));

    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));
    // Turn display on
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, true));

    drv->user_data = panel_handle;
    drv->flush_cb = st7796_lv_flush;
}

#endif