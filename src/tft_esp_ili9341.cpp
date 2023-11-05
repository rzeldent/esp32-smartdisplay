#include <esp32_smartdisplay.h>

#ifdef USES_ILI9341_DISABLE

#include <driver/spi_master.h>
#include <esp_lcd_panel_io.h>
#include <esp_lcd_panel_vendor.h>
#include <esp_lcd_panel_ops.h>

esp_lcd_panel_io_handle_t io_handle;
esp_lcd_panel_handle_t panel_handle;

constexpr spi_bus_config_t buscfg = {
    .mosi_io_num = ST7796_SPI_MOSI,
    .miso_io_num = ST7796_SPI_MISO,
    .sclk_io_num = ST7796_SPI_SCLK,
    .quadwp_io_num = -1, // Quad SPI LCD driver is not yet supported
    .quadhd_io_num = -1, // Quad SPI LCD driver is not yet supported
    .data4_io_num = -1,
    .data5_io_num = -1,
    .data6_io_num = -1,
    .data7_io_num = -1,
    .max_transfer_sz = TFT_WIDTH * 80 * sizeof(lv_color_t), // transfer 80 lines of pixels (assume pixel is RGB565) at most in one SPI transaction
    .flags = 0,
    .intr_flags = 0,
};

constexpr esp_lcd_panel_io_spi_config_t io_config = {
    .cs_gpio_num = ST7796_PIN_CS,
    .dc_gpio_num = ST7796_PIN_DC,
    .spi_mode = SPI_MODE0,
    .pclk_hz = ST7796_SPI_FREQ,
    .trans_queue_depth = 10,
    .on_color_trans_done = nullptr,
    .user_ctx = nullptr,
    .lcd_cmd_bits = 8,
    .lcd_param_bits = 8,
    .flags = {
        .dc_as_cmd_phase = 0,
        .dc_low_on_data = 0,
        .octal_mode = 0,
        .lsb_first = 0,
    }};

constexpr esp_lcd_panel_dev_config_t panel_config = {
    .reset_gpio_num = -1,
    .color_space = ESP_LCD_COLOR_SPACE_RGB,
    .bits_per_pixel = 16,
    .flags = {
        .reset_active_high = 0},
    .vendor_config = nullptr};

void lvgl_tft_init(lv_disp_drv_t *drv)
{
    // Create SPI bus
    ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO)); // Enable the DMA feature
    // Attach the LCD to the SPI bus
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)SPI2_HOST, &io_config, &io_handle));
    // st7789 panel handle
    ESP_ERROR_CHECK(esp_lcd_new_panel_ili9341(io_handle, &panel_config, &panel_handle));

    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));

    pinMode(ST7796_PIN_BL, OUTPUT);
    digitalWrite(ST7796_PIN_BL, HIGH);
}

void lvgl_tft_flush(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_map)
{
    ESP_ERROR_CHECK(esp_lcd_panel_draw_bitmap(panel_handle, area->x1, area->y1, area->x2 + 1, area->y2 + 1, color_map));
    lv_disp_flush_ready(drv);
}

void smartdisplay_tft_set_backlight(uint16_t duty)
{
    ledcWrite(PWM_CHANNEL_BL, duty);
}

void smartdisplay_tft_sleep()
{
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, true));
}

void smartdisplay_tft_wake()
{
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, false));
}

#endif