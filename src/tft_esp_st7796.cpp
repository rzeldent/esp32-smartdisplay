#include <esp32_smartdisplay.h>

#ifdef USES_ST7796_X

#include <driver/spi_master.h>
#include <esp_lcd_panel_io.h>
#include <esp_lcd_panel_vendor.h>
#include <esp_lcd_panel_ops.h>

#define LCD_SPI_HOST SPI2_HOST

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

esp_lcd_panel_io_spi_config_t io_config = {
    .cs_gpio_num = ST7796_PIN_CS,
    .dc_gpio_num = ST7796_PIN_DC,
    .spi_mode = SPI_MODE0,
    .pclk_hz = ST7796_SPI_FREQ,
    .trans_queue_depth = 10,
    .on_color_trans_done = [](esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_io_event_data_t *edata, void *user_ctx)
    {
        auto disp_driver = (lv_disp_drv_t *)user_ctx;
        lv_disp_flush_ready(disp_driver);
        return false;
    },
    .user_ctx = nullptr,
    .lcd_cmd_bits = 8,
    .lcd_param_bits = 8,
    .flags = {
        .dc_as_cmd_phase = 0,
        .dc_low_on_data = 0,
        .octal_mode = 0,
        .lsb_first = 0,
    }};


  //ESP_ERROR_CHECK(esp_lcd_panel_io_tx_param(io_handle, 0x01, nullptr, 0));             // Software Reset
    // static const uint8_t cscon1[] = {0xC3};                                              // Enable extension command 2 part I
    // ESP_ERROR_CHECK(esp_lcd_panel_io_tx_param(io_handle, 0xF0, cscon1, sizeof(cscon1))); // Command Set Control
    // static const uint8_t cscon2[] = {0x96};                                              // Enable extension command 2 part II
    // ESP_ERROR_CHECK(esp_lcd_panel_io_tx_param(io_handle, 0xF0, cscon2, sizeof(cscon2))); // Command Set Control
    //static const uint8_t madctl[] = {0x40};
    //ESP_ERROR_CHECK(esp_lcd_panel_io_tx_param(io_handle, 0x36, madctl, sizeof(madctl)));              // Memory Data Access Control

    // static const uint8_t colmod[] = {0x55};                                              // 16 bits R5G6B5 (COLMOD_RGB_16BIT | COLMOD_CTRL_16BIT)
    // ESP_ERROR_CHECK(esp_lcd_panel_io_tx_param(io_handle, 0x3A, colmod, sizeof(colmod)));              // Interface Pixel Format
    // static const uint8_t pgc[] = {0xF0, 0x09, 0x0B, 0x06, 0x04, 0x15, 0x2F, 0x54, 0x42, 0x3C, 0x17, 0x14, 0x18, 0x1B};
    // ESP_ERROR_CHECK(esp_lcd_panel_io_tx_param(io_handle, 0xE0, pgc, sizeof(pgc))); // Positive Gamma Control
    // static const uint8_t ngc[] = {0xE0, 0x09, 0x0B, 0x06, 0x04, 0x03, 0x2B, 0x43, 0x42, 0x3B, 0x16, 0x14, 0x17, 0x1B};
    // ESP_ERROR_CHECK(esp_lcd_panel_io_tx_param(io_handle, 0xE1, pgc, sizeof(pgc))); // Negative Gamma Control


constexpr esp_lcd_panel_dev_config_t panel_config = {
    .reset_gpio_num = -1,
    .color_space = ESP_LCD_COLOR_SPACE_RGB,
    .bits_per_pixel = 16,
    .flags = {
        .reset_active_high = 0},
    .vendor_config = nullptr};

void st7796_drv_update(lv_disp_drv_t *drv)
{
    auto panel_handle = (esp_lcd_panel_handle_t)drv->user_data;
    switch (drv->rotated)
    {
    case LV_DISP_ROT_NONE:
        // Rotate LCD display
        esp_lcd_panel_swap_xy(panel_handle, false);
        esp_lcd_panel_mirror(panel_handle, true, false);
#if CONFIG_EXAMPLE_LCD_TOUCH_ENABLED
        // Rotate LCD touch
        esp_lcd_touch_set_mirror_y(tp, false);
        esp_lcd_touch_set_mirror_x(tp, false);
#endif
        break;
    case LV_DISP_ROT_90:
        // Rotate LCD display
        esp_lcd_panel_swap_xy(panel_handle, true);
        esp_lcd_panel_mirror(panel_handle, true, true);
#if CONFIG_EXAMPLE_LCD_TOUCH_ENABLED
        // Rotate LCD touch
        esp_lcd_touch_set_mirror_y(tp, false);
        esp_lcd_touch_set_mirror_x(tp, false);
#endif
        break;
    case LV_DISP_ROT_180:
        // Rotate LCD display
        esp_lcd_panel_swap_xy(panel_handle, false);
        esp_lcd_panel_mirror(panel_handle, false, true);
#if CONFIG_EXAMPLE_LCD_TOUCH_ENABLED
        // Rotate LCD touch
        esp_lcd_touch_set_mirror_y(tp, false);
        esp_lcd_touch_set_mirror_x(tp, false);
#endif
        break;
    case LV_DISP_ROT_270:
        // Rotate LCD display
        esp_lcd_panel_swap_xy(panel_handle, true);
        esp_lcd_panel_mirror(panel_handle, false, false);
#if CONFIG_EXAMPLE_LCD_TOUCH_ENABLED
        // Rotate LCD touch
        esp_lcd_touch_set_mirror_y(tp, false);
        esp_lcd_touch_set_mirror_x(tp, false);
#endif
        break;
    }
}

void lvgl_tft_init(lv_disp_drv_t *drv)
{
    // pinMode(ST7796_PIN_DC, OUTPUT); // Data or Command
    // pinMode(ST7796_PIN_CS, OUTPUT); // Chip Select
    // Initialize SPI bus
    ESP_ERROR_CHECK(spi_bus_initialize(LCD_SPI_HOST, &buscfg, SPI_DMA_CH_AUTO)); // Enable the DMA feature

    io_config.user_ctx = drv;
    // io_config.on_color_trans_done = notify_lvgl_flush_ready;
    //  Attach the LCD to the SPI bus
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)LCD_SPI_HOST, &io_config, &io_handle));
    // st7789 panel handle
    ESP_ERROR_CHECK(esp_lcd_new_panel_st7789(io_handle, &panel_config, &panel_handle));
    // Flip display
    ESP_ERROR_CHECK(esp_lcd_panel_mirror(panel_handle, true, false));
    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));

    drv->user_data = panel_handle;
    drv->drv_update_cb = st7796_drv_update;

    pinMode(ST7796_PIN_BL, OUTPUT);
    //    ledcSetup(PWM_CHANNEL_BL, PWM_FREQ_BL, PWM_BITS_BL);
    //  ledcAttachPin(ST7796_PIN_BL, PWM_CHANNEL_BL);
    digitalWrite(ST7796_PIN_BL, HIGH);
}

void lvgl_tft_flush(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_map)
{
    Serial.write("flush\n");
    ESP_ERROR_CHECK(esp_lcd_panel_draw_bitmap(panel_handle, area->x1, area->y1, area->x2 + 1, area->y2 + 1, color_map));
    // lv_disp_flush_ready(drv);
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