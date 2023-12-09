#include <esp32_smartdisplay.h>

#ifdef USES_GC9A01

#include <driver/spi_master.h>
#include <esp_lcd_panel_io.h>
#include <esp_lcd_panel_vendor.h>
#include <esp_lcd_panel_ops.h>

#include "esp_lcd_gc9a01.h"

bool gc9a01_color_trans_done(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_io_event_data_t *edata, void *user_ctx)
{
    lv_disp_drv_t *disp_driver = user_ctx;
    lv_disp_flush_ready(disp_driver);
    return false;
}

void gc9a01_lv_flush(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_map)
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

void lvgl_tft_init(lv_disp_drv_t *drv)
{
    // Create SPI bus
    const spi_bus_config_t spi_bus_config = GC9A01_SPI_BUS_CONFIG;
    ESP_ERROR_CHECK_WITHOUT_ABORT(spi_bus_initialize(GC9A01_SPI_HOST, &spi_bus_config, SPI_DMA_CH_AUTO));

    // Attach the LCD controller to the SPI bus
    esp_lcd_panel_io_spi_config_t io_config = GC9A01_IO_SPI_CONFIG;
    io_config.user_ctx = drv;
    io_config.on_color_trans_done = gc9a01_color_trans_done;
    esp_lcd_panel_io_handle_t io_handle;
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)GC9A01_SPI_HOST, &io_config, &io_handle));

    // Create gc9a01 panel handle
    esp_lcd_panel_dev_config_t panel_dev_config = GC9A01_PANEL_DEV_CONFIG;
    esp_lcd_panel_handle_t panel_handle;
    ESP_ERROR_CHECK(esp_lcd_new_panel_gc9a01(io_handle, &panel_dev_config, &panel_handle));

    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));
    drv->user_data = panel_handle;
    drv->flush_cb = gc9a01_lv_flush;

    // Colors are inverted
    ESP_ERROR_CHECK(esp_lcd_panel_invert_color(panel_handle, true));

    // Turn display on
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, true));
}

#endif