#include <esp32_smartdisplay.h>

// #include <esp_err.h>
// #include <esp_lcd_panel_ops.h>

// Functions to be defined in the tft driver
extern void lvgl_tft_init(lv_disp_drv_t *drv);
// Functions to be defined in the touch driver
extern void lvgl_touch_init(lv_indev_drv_t *drv);

#if LV_USE_LOG
void lvgl_log(const char *buf)
{
  log_printf("%s", buf);
}
#endif

void smartdisplay_init()
{
#ifdef HAS_RGB_LED
  // Setup RGB LED.  High is off
  pinMode(LED_PIN_R, OUTPUT);
  digitalWrite(LED_PIN_R, true);
  pinMode(LED_PIN_G, OUTPUT);
  digitalWrite(LED_PIN_G, true);
  pinMode(LED_PIN_B, OUTPUT);
  digitalWrite(LED_PIN_B, true);
#endif

#ifdef HAS_LIGHTSENSOR
  // CDS Light sensor
  analogSetAttenuation(ADC_0db); // 0dB(1.0x) 0~800mV
  pinMode(LIGHTSENSOR_IN, INPUT);
#endif

#ifdef HAS_SPEAKER
  // Speaker
  // Note: tone function uses PWM channel 0
  pinMode(SPEAKER_PIN, INPUT); // Set high impedance
#endif

#if LV_USE_LOG
  lv_log_register_print_cb(lvgl_log);
#endif

  lv_init();
  // Setup backlight
  pinMode(PIN_BCKL, OUTPUT);
  ledcSetup(PWM_CHANNEL_BCKL, PWM_FREQ_BCKL, PWM_BITS_BCKL);
  ledcAttachPin(PIN_BCKL, PWM_CHANNEL_BCKL);
  digitalWrite(PIN_BCKL, LOW);
  // Setup TFT display
  static lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);
  disp_drv.hor_res = TFT_WIDTH;
  disp_drv.ver_res = TFT_HEIGHT;
  // Create drawBuffer
  disp_drv.draw_buf = new lv_disp_draw_buf_t;
  const auto drawBufferPixels = TFT_WIDTH * LVGL_PIXEL_BUFFER_LINES;
  auto drawBuffer = heap_caps_malloc(sizeof(lv_color_t) * drawBufferPixels, MALLOC_CAP_DMA);
  lv_disp_draw_buf_init(disp_drv.draw_buf, drawBuffer, nullptr, drawBufferPixels);
  // Initialize specific driver
  lvgl_tft_init(&disp_drv);
  auto display = lv_disp_drv_register(&disp_drv);
  // Clear screen
  lv_obj_clean(lv_scr_act());
  // Turn backlight on
  ledcWrite(PWM_CHANNEL_BCKL, PWM_MAX_BCKL);

// If there is a touch controller defined
#if defined(USES_CST816) || defined(USES_XPT2046) || defined(USES_GT911)
  // Setup touch
  static lv_indev_drv_t indev_drv;
  lv_indev_drv_init(&indev_drv);
  indev_drv.disp = display;
  lvgl_touch_init(&indev_drv);
  lv_indev_drv_register(&indev_drv);
#endif
}

void smartdisplay_tft_set_backlight(uint16_t duty)
{
  ledcWrite(PWM_CHANNEL_BCKL, duty);
}