#ifdef TOUCH_CST816S_I2C

#include "esp_touch_cst816s.h"
#include <esp32_smartdisplay.h>
#include "driver/i2c.h"

void cst816s_lvgl_touch_cb(lv_indev_t *indev, lv_indev_data_t *data)
{
    esp_lcd_touch_handle_t touch_handle = indev->user_data;

    uint16_t x[1];
    uint16_t y[1];
    uint8_t touch_cnt = 0;

    // Read touch controller data
    ESP_ERROR_CHECK(esp_lcd_touch_read_data(touch_handle));
    // Get coordinates
    bool pressed = esp_lcd_touch_get_coordinates(touch_handle, x, y, NULL, &touch_cnt, 1);
    if (pressed && touch_cnt > 0)
    {
        data->point.x = x[0];
        data->point.y = y[0];
        data->state = LV_INDEV_STATE_PRESSED;
        log_v("Pressed at: (%d,%d)", data->point.x, data->point.y);
    }
    else
        data->state = LV_INDEV_STATE_RELEASED;
}

lv_indev_t *lvgl_touch_init()
{
    lv_indev_t *indev = lv_indev_create();
    log_v("indev:0x%08x", indev);

    // Create I2C bus
    const i2c_config_t i2c_config = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = CST816S_I2C_CONFIG_SDA_IO_NUM,
        .scl_io_num = CST816S_I2C_CONFIG_SCL_IO_NUM,
        .sda_pullup_en = CST816S_I2C_CONFIG_SDA_PULLUP_EN,
        .scl_pullup_en = CST816S_I2C_CONFIG_SCL_PULLUP_EN,
        .master = {
            .clk_speed = CST816S_I2C_CONFIG_MASTER_CLK_SPEED},
        .clk_flags = CST816S_I2C_CONFIG_CLK_FLAGS};
    log_d("i2c_config: mode:%d, sda_io_num:%d, scl_io_num:%d, sda_pullup_en:%d, scl_pullup_en:%d, master:{clk_speed:%d}, clk_flags:%d", i2c_config.mode, i2c_config.sda_io_num, i2c_config.scl_io_num, i2c_config.sda_pullup_en, i2c_config.scl_pullup_en, i2c_config.master.clk_speed, i2c_config.clk_flags);
    ESP_ERROR_CHECK(i2c_param_config(CST816S_I2C_HOST, &i2c_config));
    ESP_ERROR_CHECK(i2c_driver_install(CST816S_I2C_HOST, i2c_config.mode, 0, 0, 0));

    // Create IO handle
    const esp_lcd_panel_io_i2c_config_t io_i2c_config = {
        .dev_addr = CST816S_IO_I2C_CONFIG_DEV_ADDRESS,
        .control_phase_bytes = CST816S_IO_I2C_CONFIG_CONTROL_PHASE_BYTES,
        .user_ctx = indev,
        .dc_bit_offset = CST816S_IO_I2C_CONFIG_DC_BIT_OFFSET,
        .lcd_cmd_bits = CST816S_IO_I2C_CONFIG_LCD_CMD_BITS,
        .lcd_param_bits = CST816S_IO_I2C_CONFIG_LCD_PARAM_BITS,
        .flags = {
            .dc_low_on_data = CST816S_IO_I2C_CONFIG_FLAGS_DC_LOW_ON_DATA,
            .disable_control_phase = CST816S_IO_I2C_CONFIG_FLAGS_DISABLE_CONTROL_PHASE}};
    log_d("io_i2c_config: dev_addr:0x%02x, control_phase_bytes:%d, user_ctx:0x%08x, dc_bit_offset:%d, lcd_cmd_bits:%d, lcd_param_bits:%d, flags:{.dc_low_on_data:%d, disable_control_phase:%d}", io_i2c_config.dev_addr, io_i2c_config.control_phase_bytes, io_i2c_config.user_ctx, io_i2c_config.dc_bit_offset, io_i2c_config.lcd_cmd_bits, io_i2c_config.lcd_param_bits, io_i2c_config.flags.dc_low_on_data, io_i2c_config.flags.disable_control_phase);
    esp_lcd_panel_io_handle_t io_handle;
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_i2c((esp_lcd_i2c_bus_handle_t)CST816S_I2C_HOST, &io_i2c_config, &io_handle));

    // Create touch configuration
    const esp_lcd_touch_config_t touch_config = {
        .x_max = CST816S_TOUCH_CONFIG_X_MAX,
        .y_max = CST816S_TOUCH_CONFIG_Y_MAX,
        .rst_gpio_num = CST816S_TOUCH_CONFIG_RST_GPIO_NUM,
        .int_gpio_num = CST816S_TOUCH_CONFIG_INT_GPIO_NUM,
        .levels = {
            .reset = CST816S_TOUCH_CONFIG_LEVELS_RESET,
            .interrupt = CST816S_TOUCH_CONFIG_LEVELS_INTERRUPT},
        .flags = {.swap_xy = TOUCH_SWAP_XY, .mirror_x = TOUCH_MIRROR_X, .mirror_y = TOUCH_MIRROR_Y},
        .user_data = io_handle};
    log_d("touch_config: x_max:%d, y_max:%d, rst_gpio_num:%d, int_gpio_num:%d, levels:{reset:%d, interrupt:%d}, flags:{swap_xy:%d, mirror_x:%d, mirror_y:%d}, user_data:0x%08x", touch_config.x_max, touch_config.y_max, touch_config.rst_gpio_num, touch_config.int_gpio_num, touch_config.levels.reset, touch_config.levels.interrupt, touch_config.flags.swap_xy, touch_config.flags.mirror_x, touch_config.flags.mirror_y, touch_config.user_data);
    esp_lcd_touch_handle_t touch_handle;
    ESP_ERROR_CHECK(esp_lcd_touch_new_i2c_cst816s(io_handle, &touch_config, &touch_handle));

    indev->type = LV_INDEV_TYPE_POINTER;
    indev->user_data = touch_handle;
    indev->read_cb = cst816s_lvgl_touch_cb;

    return indev;
}

#endif