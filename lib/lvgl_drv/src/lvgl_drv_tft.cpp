#include <Arduino.h>
#include <lvgl_drv_tft.h>

// Functions need to be defined in driver
static void lvgl_tft_init();
static void lvgl_tft_flush(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_map);

#if LV_USE_LOG
void lvgl_log(const char *buf)
{
    log_printf("%s", buf);
}
#endif

void lvgl_init()
{
    lvgl_tft_init();
#if LV_USE_LOG
    lv_log_register_print_cb(lvgl_log);
#endif

    lv_init();

    static lv_disp_draw_buf_t draw_buf;
    static lv_color_t buf[TFT_WIDTH * 10];
    lv_disp_draw_buf_init(&draw_buf, buf, NULL, TFT_WIDTH * 10);

    // Initialize the display
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
}