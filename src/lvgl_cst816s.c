#ifdef TOUCH_USES_CST816S

#include <esp32_smartdisplay.h>
#include "driver/i2c.h"
#include "esp_lcd_touch.h"
#include "esp_lcd_touch_cst816s.h"

static void cst816s_lvgl_touch_cb(lv_indev_drv_t *drv, lv_indev_data_t *data)
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
    // Create I2C bus
    const i2c_config_t i2c_config = {.mode = I2C_MODE_MASTER, .sda_io_num = CST816S_I2C_SDA, .scl_io_num = CST816S_I2C_SCL, .sda_pullup_en = GPIO_PULLUP_ENABLE, .scl_pullup_en = GPIO_PULLUP_ENABLE, .master = {.clk_speed = 400000}};
    ESP_ERROR_CHECK(i2c_param_config(CST816S_I2C_HOST, &i2c_config));
    ESP_ERROR_CHECK(i2c_driver_install(CST816S_I2C_HOST, i2c_config.mode, 0, 0, 0));

    // Create IO handle
    esp_lcd_panel_io_i2c_config_t io_i2c_config = {.dev_addr = ESP_LCD_TOUCH_IO_I2C_CST816S_ADDRESS, .control_phase_bytes = 1, .lcd_cmd_bits = 8, .flags = {.disable_control_phase = 1}};
    io_i2c_config.user_ctx = drv;
    esp_lcd_panel_io_handle_t io_handle;
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_i2c((esp_lcd_i2c_bus_handle_t)CST816S_I2C_HOST, &io_i2c_config, &io_handle));

    // Create touch configuration
    esp_lcd_touch_config_t touch_config = {.x_max = LCD_WIDTH, .y_max = LCD_HEIGHT, .rst_gpio_num = CST816S_RST, .int_gpio_num = CST816S_INT};
    touch_config.user_data = io_handle;
    esp_lcd_touch_handle_t touch_handle;
    ESP_ERROR_CHECK(esp_lcd_touch_new_i2c_cst816s(io_handle, &touch_config, &touch_handle));

    drv->type = LV_INDEV_TYPE_POINTER;
    drv->user_data = touch_handle;
    drv->read_cb = cst816s_lvgl_touch_cb;
}

#endif