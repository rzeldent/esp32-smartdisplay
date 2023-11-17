#include <esp32_smartdisplay.h>

#ifdef USES_GT911

#include "driver/i2c.h"
#include "esp_lcd_touch.h"
#include "esp_lcd_touch_gt911.h"

void gt911_lvgl_touch_cb(lv_indev_drv_t *drv, lv_indev_data_t *data)
{
    const auto touch_handle = (esp_lcd_touch_handle_t)drv->user_data;
    uint16_t touchpad_x[1] = {0};
    uint16_t touchpad_y[1] = {0};
    uint8_t touchpad_cnt = 0;

    /* Read touch controller data */
    ESP_ERROR_CHECK(esp_lcd_touch_read_data(touch_handle));
    /* Get coordinates */
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
    // Create I2C bus
    ESP_ERROR_CHECK(i2c_param_config(GT911_I2C_HOST, &gt911_i2c_config));
    ESP_ERROR_CHECK(i2c_driver_install(GT911_I2C_HOST, gt911_i2c_config.mode, 0, 0, 0));

    // Create IO handle
    esp_lcd_panel_io_handle_t io_handle;
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_i2c((esp_lcd_i2c_bus_handle_t)GT911_I2C_HOST, &gt911_io_i2c_config, &io_handle));

    // Create touch configuration
    esp_lcd_touch_config_t touch_config = gt911_touch_config;
    touch_config.user_data = io_handle;

    esp_lcd_touch_handle_t touch_handle;
    ESP_ERROR_CHECK(esp_lcd_touch_new_i2c_gt911(io_handle, &touch_config, &touch_handle));
    drv->type = LV_INDEV_TYPE_POINTER;
    drv->user_data = touch_handle;
    drv->read_cb = gt911_lvgl_touch_cb;
}

#endif