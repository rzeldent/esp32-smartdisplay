#include <Arduino.h>
#include <lvgl_drv.h>

// Functions to be defined in the tft driver
extern void lvgl_tft_init();
extern void lvgl_tft_flush(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_map);

// Functions to be defined in the touch driver
extern void lvgl_touch_init();
extern void lvgl_touch_read(lv_indev_drv_t *drv, lv_indev_data_t *data);

// Bus
#if defined(LVGL_TFT_ST7796) || defined(LVGL_TFT_IL9341) || defined(LVGL_TOUCH_XPT2046)
SPIClass lvgl_bus_spi;
#endif

#if defined(LVGL_TOUCH_GT911)
TwoWire lvgl_bus_i2c = TwoWire(1); // Bus number 1
#endif

#if LV_USE_LOG
void lvgl_log(const char *buf)
{
    log_printf("%s", buf);
}
#endif

void lvgl_init()
{
#if LV_USE_LOG
    lv_log_register_print_cb(lvgl_log);
#endif
    lv_init();

    // Initialize buses
    // SPI
#if defined(LVGL_TFT_ST7796) || defined(LVGL_TFT_IL9341) || defined(LVGL_TOUCH_XPT2046)
    lvgl_bus_spi.begin(LVGL_SPI_SCLK, LVGL_SPI_MISO, LVGL_SPI_MOSI);
#endif
// I2C
#if defined(LVGL_TOUCH_GT911)
    lvgl_bus_i2c.begin(TOUCH_IIC_SDA, TOUCH_IIC_SCL);
#endif

#if defined(LVGL_TFT_ST7796) || defined(LVGL_TFT_IL9341)
    lvgl_tft_init();

    static lv_disp_draw_buf_t draw_buf;
    static lv_color_t buf[TFT_WIDTH * 10];
    lv_disp_draw_buf_init(&draw_buf, buf, NULL, TFT_WIDTH * 10);

    // Setup TFT display
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
#if defined(TFT_ORIENTATION_PORTRAIT) || defined(TFT_ORIENTATION_PORTRAIT_INV)
    disp_drv.hor_res = TFT_WIDTH;
    disp_drv.ver_res = TFT_HEIGHT;
#else
#if defined(TFT_ORIENTATION_LANDSCAPE) || defined(TFT_ORIENTATION_LANDSCAPE_INV)
    disp_drv.hor_res = TFT_HEIGHT;
    disp_drv.ver_res = TFT_WIDTH;
#else
#error TFT_ORIENTATION not defined!
#endif
#endif
    disp_drv.flush_cb = lvgl_tft_flush;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register(&disp_drv);
#endif

#if defined(LVGL_TOUCH_GT911) || defined(LVGL_TOUCH_XPT2046)
    lvgl_touch_init();

    // Setup touch
    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = lvgl_touch_read;
    lv_indev_drv_register(&indev_drv);
#endif
}