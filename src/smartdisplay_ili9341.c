#include <esp32_smartdisplay.h>

#ifdef USES_ILI9341

#include <driver/spi_master.h>
#include <esp_lcd_panel_io.h>
#include <esp_lcd_panel_vendor.h>
#include <esp_lcd_panel_ops.h>

#include "esp_lcd_ili9341.h"

static bool ili9341_color_trans_done(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_io_event_data_t *edata, void *user_ctx)
{
    lv_disp_drv_t *disp_driver = user_ctx;
    lv_disp_flush_ready(disp_driver);
    return false;
}

static void ili9341_lv_flush(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_map)
{
    esp_lcd_panel_handle_t panel_handle = drv->user_data;
    ESP_ERROR_CHECK(esp_lcd_panel_draw_bitmap(panel_handle, area->x1, area->y1, area->x2 + 1, area->y2 + 1, color_map));
};

void lvgl_tft_init(lv_disp_drv_t *drv)
{
    // Create SPI bus
    const spi_bus_config_t spi_bus_config = ILI9341_SPI_BUS_CONFIG;
    ESP_ERROR_CHECK_WITHOUT_ABORT(spi_bus_initialize(ILI9341_SPI_HOST, &spi_bus_config, SPI_DMA_CH_AUTO));

    // Attach the LCD controller to the SPI bus
    esp_lcd_panel_io_spi_config_t io_config = ILI9341_IO_SPI_CONFIG;
    io_config.on_color_trans_done = ili9341_color_trans_done;
    io_config.user_ctx = drv;
    esp_lcd_panel_io_handle_t io_handle;
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)ILI9341_SPI_HOST, &io_config, &io_handle));

    // Create ili9341 panel handle
    const esp_lcd_panel_dev_config_t panel_dev_config = ILI9341_PANEL_DEV_CONFIG;
    esp_lcd_panel_handle_t panel_handle;
    ESP_ERROR_CHECK(esp_lcd_new_panel_ili9341(io_handle, &panel_dev_config, &panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));

    drv->user_data = panel_handle;
    drv->flush_cb = ili9341_lv_flush;

    // Turn display on
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, true));
}

#endif