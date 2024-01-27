#ifdef TOUCH_GT911_I2C

#include <esp32_smartdisplay.h>
#include "driver/i2c.h"
#include "esp_lcd_touch.h"
#include "esp_lcd_touch_gt911.h"

static void gt911_lvgl_touch_cb(lv_indev_drv_t *drv, lv_indev_data_t *data)
{
    esp_lcd_touch_handle_t touch_handle = drv->user_data;

    uint16_t touch_x[1] = {0};
    uint16_t touch_y[1] = {0};
    uint16_t touch_strength[1] = {0};
    uint8_t touch_cnt = 0;

    // Read touch controller data
    ESP_ERROR_CHECK(esp_lcd_touch_read_data(touch_handle));
    // Get coordinates
    bool pressed = esp_lcd_touch_get_coordinates(touch_handle, touch_x, touch_y, touch_strength, &touch_cnt, 1);
    if (pressed && touch_cnt > 0)
    {
        data->point.x = touch_x[0];
        data->point.y = touch_y[0];
        data->state = LV_INDEV_STATE_PRESSED;
        log_d("Pressed at: (%d,%d)", data->point.x, data->point.y);
    }
    else
        data->state = LV_INDEV_STATE_RELEASED;
}

void lvgl_touch_init(lv_indev_drv_t *drv)
{
    log_d("lvgl_touch_init");
    // Create I2C bus
    const i2c_config_t i2c_config = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = GT911_I2C_CONFIG_SDA_IO_NUM,
        .scl_io_num = GT911_I2C_CONFIG_SCL_IO_NUM,
        .sda_pullup_en = GT911_I2C_CONFIG_SDA_PULLUP_EN,
        .scl_pullup_en = GT911_I2C_CONFIG_SCL_PULLUP_EN,
        .master = {
            .clk_speed = GT911_I2C_CONFIG_MASTER_CLK_SPEED},
        .clk_flags = GT911_I2C_CONFIG_CLK_FLAGS};
    ESP_ERROR_CHECK(i2c_param_config(GT911_I2C_HOST, &i2c_config));
    log_d("i2c_param_config. host: %d", GT911_I2C_HOST);
    ESP_ERROR_CHECK(i2c_driver_install(GT911_I2C_HOST, i2c_config.mode, 0, 0, 0));
    log_d("i2c_driver_install host: %d", GT911_I2C_HOST);

    // Create IO handle
    const esp_lcd_panel_io_i2c_config_t io_i2c_config = {
        .dev_addr = GT911_IO_I2C_CONFIG_DEV_ADDR,
        .control_phase_bytes = GT911_IO_I2C_CONFIG_CONTROL_PHASE_BYTES,
        .user_ctx = drv,
        .dc_bit_offset = GT911_IO_I2C_CONFIG_DC_BIT_OFFSET,
        .lcd_cmd_bits = GT911_IO_I2C_CONFIG_LCD_CMD_BITS,
        .lcd_param_bits = GT911_IO_I2C_CONFIG_LCD_PARAM_BITS,
        .flags = {
            .dc_low_on_data = GT911_IO_I2C_CONFIG_FLAGS_DC_LOW_ON_DATA,
            .disable_control_phase = GT911_IO_I2C_CONFIG_FLAGS_DISABLE_CONTROL_PHASE}};

    esp_lcd_panel_io_handle_t io_handle;
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_i2c((esp_lcd_i2c_bus_handle_t)GT911_I2C_HOST, &io_i2c_config, &io_handle));
    log_d("esp_lcd_new_panel_io_i2c. host: %d", GT911_I2C_HOST);

    // Create touch configuration
    const esp_lcd_touch_config_t touch_config = {
        .x_max = GT911_TOUCH_CONFIG_X_MAX,
        .y_max = GT911_TOUCH_CONFIG_Y_MAX,
        .rst_gpio_num = GT911_TOUCH_CONFIG_RST_GPIO_NUM,
        .int_gpio_num = GT911_TOUCH_CONFIG_INT_GPIO_NUM,
        .levels = {
            .reset = GT911_TOUCH_CONFIG_LEVELS_RESET,
            .interrupt = GT911_TOUCH_CONFIG_LEVELS_INTERRUPT},
        // Unfortunately not supported
        //.flags = {.swap_xy = GT911_TOUCH_CONFIG_FLAGS_SWAP_XY, .mirror_x = GT911_TOUCH_CONFIG_FLAGS_MIRROR_X, .mirror_y = GT911_TOUCH_CONFIG_FLAGS_MIRROR_Y},
        .user_data = io_handle};

    esp_lcd_touch_handle_t touch_handle;
    ESP_ERROR_CHECK(esp_lcd_touch_new_i2c_gt911(io_handle, &touch_config, &touch_handle));
    log_d("esp_lcd_touch_new_i2c_gt911. host: %d", GT911_I2C_HOST);

    drv->type = LV_INDEV_TYPE_POINTER;
    drv->user_data = touch_handle;
    drv->read_cb = gt911_lvgl_touch_cb;
}

#endif