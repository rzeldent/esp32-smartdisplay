#ifdef TOUCH_USES_XPT2046

#include <esp32_smartdisplay.h>
#include <driver/spi_master.h>
#include "esp_lcd_touch.h"
#include "esp_lcd_touch_xpt2046.h"

static void xpt2046_lvgl_touch_cb(lv_indev_drv_t *drv, lv_indev_data_t *data)
{
    esp_lcd_touch_handle_t touch_handle = drv->user_data;

    uint16_t touchpad_x[1] = {0};
    uint16_t touchpad_y[1] = {0};
    uint8_t touchpad_cnt = 0;

    // Read touch controller data
    ESP_ERROR_CHECK(esp_lcd_touch_read_data(touch_handle));
    // Get coordinates
    bool pressed = esp_lcd_touch_get_coordinates(touch_handle, touchpad_x, touchpad_y, NULL, &touchpad_cnt, 1);
    if (pressed && touchpad_cnt > 0)
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
    const spi_bus_config_t spi_bus_config = {.mosi_io_num = XPT2046_SPI_MOSI, .miso_io_num = XPT2046_SPI_MISO, .sclk_io_num = XPT2046_SPI_SCLK, .quadwp_io_num = -1, .quadhd_io_num = -1};
    ESP_ERROR_CHECK_WITHOUT_ABORT(spi_bus_initialize(XPT2046_SPI_HOST, &spi_bus_config, SPI_DMA_CH_AUTO));

    // Attach the touch controller to the SPI bus
    esp_lcd_panel_io_spi_config_t io_spi_config = {.cs_gpio_num = XPT2046_CS, .dc_gpio_num = -1, .spi_mode = SPI_MODE0, .pclk_hz = 2000000, .trans_queue_depth = 3, .lcd_cmd_bits = 8, .lcd_param_bits = 8};
    io_spi_config.user_ctx = drv;
    esp_lcd_panel_io_handle_t io_handle;
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)XPT2046_SPI_HOST, &io_spi_config, &io_handle));

    // Create touch configuration
    esp_lcd_touch_config_t touch_config = {.x_max = LCD_WIDTH, .y_max = LCD_HEIGHT, .rst_gpio_num = GPIO_NUM_NC, .int_gpio_num = XPT2046_INT};
    touch_config.user_data = io_handle;
    esp_lcd_touch_handle_t touch_handle;
    ESP_ERROR_CHECK(esp_lcd_touch_new_spi_xpt2046(io_handle, &touch_config, &touch_handle));

    drv->type = LV_INDEV_TYPE_POINTER;
    drv->user_data = touch_handle;
    drv->read_cb = xpt2046_lvgl_touch_cb;
}

#endif