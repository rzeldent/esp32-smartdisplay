#include <esp32_smartdisplay.h>

#ifdef USES_XPT2046

#include <driver/spi_master.h>
#include "esp_lcd_touch.h"
#include "esp_lcd_touch_xpt2046.h"

static void xpt2046_lvgl_touch_cb(lv_indev_drv_t *drv, lv_indev_data_t *data)
{
    const auto touch_handle = (esp_lcd_touch_handle_t)drv->user_data;

    uint16_t touchpad_x[1] = {0};
    uint16_t touchpad_y[1] = {0};
    uint8_t touchpad_cnt = 0;

    ESP_ERROR_CHECK(esp_lcd_touch_read_data(touch_handle));
    auto touchpad_pressed = esp_lcd_touch_get_coordinates(touch_handle, touchpad_x, touchpad_y, nullptr, &touchpad_cnt, 1);
    if (touchpad_pressed && touchpad_cnt > 0)
    {
        data->point.x = touchpad_x[0];
        data->point.y = touchpad_y[0];
        data->state = LV_INDEV_STATE_PRESSED;
    }
    else
        data->state = LV_INDEV_STATE_RELEASED;
}

void lvgl_touch_init(lv_indev_drv_t *drv)
{
    // Create SPI bus only if not already initialized (S035R shares the SPI bus)
    ESP_ERROR_CHECK_WITHOUT_ABORT(spi_bus_initialize(XPT2046_SPI_HOST, &xpt2046_spi_bus_config, SPI_DMA_CH_AUTO));

    // Attach the touch controller to the SPI bus
    esp_lcd_panel_io_spi_config_t io_spi_config = xpt2046_io_spi_config;
    io_spi_config.user_ctx = drv;
    esp_lcd_panel_io_handle_t io_handle;
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)XPT2046_SPI_HOST, &io_spi_config, &io_handle));

    // Create touch configuration
    esp_lcd_touch_config_t touch_config = xpt2046_touch_config;
    touch_config.user_data = io_handle;
    esp_lcd_touch_handle_t touch_handle;
    ESP_ERROR_CHECK(esp_lcd_touch_new_spi_xpt2046(io_handle, &touch_config, &touch_handle));

    drv->type = LV_INDEV_TYPE_POINTER;
    drv->user_data = touch_handle;
    drv->read_cb = xpt2046_lvgl_touch_cb;
}

#endif