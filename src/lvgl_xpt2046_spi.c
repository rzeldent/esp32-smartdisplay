#ifdef TOUCH_XPT2046_SPI

#include <esp32_smartdisplay.h>
#include <driver/spi_master.h>
#include "esp_lcd_touch.h"
#include "esp_lcd_touch_xpt2046.h"

static void xpt2046_lvgl_touch_cb(lv_indev_drv_t *drv, lv_indev_data_t *data)
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
    // Create SPI bus only if not already initialized (S035R shares the SPI bus)
    const spi_bus_config_t spi_bus_config = {
        .mosi_io_num = XPT2046_SPI_BUS_MOSI_IO_NUM,
        .miso_io_num = XPT2046_SPI_BUS_MISO_IO_NUM,
        .sclk_io_num = XPT2046_SPI_BUS_SCLK_IO_NUM,
        .quadwp_io_num = XPT2046_SPI_BUS_QUADWP_IO_NUM,
        .quadhd_io_num = XPT2046_SPI_BUS_QUADHD_IO_NUM};
    ESP_ERROR_CHECK_WITHOUT_ABORT(spi_bus_initialize(XPT2046_SPI_HOST, &spi_bus_config, XPT2046_SPI_DMA_CHANNEL));

    // Attach the touch controller to the SPI bus
    const esp_lcd_panel_io_spi_config_t io_spi_config = {
        .cs_gpio_num = XPT2046_SPI_CONFIG_CS_GPIO_NUM,
        .dc_gpio_num = XPT2046_SPI_CONFIG_DC_GPIO_NUM,
        .spi_mode = XPT2046_SPI_CONFIG_SPI_MODE,
        .pclk_hz = XPT2046_SPI_CONFIG_PCLK_HZ,
        .user_ctx = drv,
        .trans_queue_depth = XPT2046_SPI_CONFIG_TRANS_QUEUE_DEPTH,
        .lcd_cmd_bits = XPT2046_SPI_CONFIG_LCD_CMD_BITS,
        .lcd_param_bits = XPT2046_SPI_CONFIG_LCD_PARAM_BITS,
        .flags = {
            .dc_as_cmd_phase = XPT2046_SPI_CONFIG_FLAGS_DC_AS_CMD_PHASE,
            .dc_low_on_data = XPT2046_SPI_CONFIG_FLAGS_DC_LOW_ON_DATA,
            .octal_mode = XPT2046_SPI_CONFIG_FLAGS_OCTAL_MODE,
            .lsb_first = XPT2046_SPI_CONFIG_FLAGS_LSB_FIRST}};
    esp_lcd_panel_io_handle_t io_handle;
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)XPT2046_SPI_HOST, &io_spi_config, &io_handle));

    // Create touch configuration
    const esp_lcd_touch_config_t touch_config = {
        .x_max = XPT2046_TOUCH_CONFIG_X_MAX,
        .y_max = XPT2046_TOUCH_CONFIG_Y_MAX,
        .rst_gpio_num = XPT2046_TOUCH_CONFIG_RST_GPIO_NUM,
        .int_gpio_num = XPT2046_TOUCH_CONFIG_INT_GPIO_NUM,
        .levels = {
            .reset = XPT2046_TOUCH_CONFIG_LEVELS_RESET,
            .interrupt = XPT2046_TOUCH_CONFIG_LEVELS_INTERRUPT},
        // Unfortunately not supported
        //.flags = {.swap_xy = XPT2046_TOUCH_CONFIG_FLAGS_SWAP_XY, .mirror_x = XPT2046_TOUCH_CONFIG_FLAGS_MIRROR_X, .mirror_y = XPT2046_TOUCH_CONFIG_FLAGS_MIRROR_Y},
        .user_data = io_handle};
    esp_lcd_touch_handle_t touch_handle;
    ESP_ERROR_CHECK(esp_lcd_touch_new_spi_xpt2046(io_handle, &touch_config, &touch_handle));

    drv->type = LV_INDEV_TYPE_POINTER;
    drv->user_data = touch_handle;
    drv->read_cb = xpt2046_lvgl_touch_cb;
}

#endif