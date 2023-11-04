#include <esp32_smartdisplay.h>

// Functions to be defined in the tft driver
extern void lvgl_tft_init();
extern void lvgl_tft_flush(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_map);

// Functions to be defined in the touch driver
extern void lvgl_touch_init();
extern void lvgl_touch_read(lv_indev_drv_t *drv, lv_indev_data_t *data);

#if !defined(ESP32_SMARTDISPLAY_BOARD_VALID)
#error Please define the board: ESP32_2432S024N, ESP32_2432S024R, ESP32_2432S024C, ESP32_2432S028R, ESP32_3248S035R, ESP32_3248S035C, ESP32_8048S043N, ESP32_8048S043R, ESP32_8048S050N, ESP32_8048S050C, ESP32_8048S070N or ESP32_8048S070C
#endif

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
  // Use channel 0=R, 1=G, 2=B, 5kHz,  8 bit resolution
  pinMode(LED_PIN_R, OUTPUT);
  digitalWrite(LED_PIN_R, true);
  ledcSetup(LED_PWM_CHANNEL_R, LED_PWM_FREQ, LED_PWM_BITS);
  ledcAttachPin(LED_PIN_R, LED_PWM_CHANNEL_R);
  pinMode(LED_PIN_G, OUTPUT);
  digitalWrite(LED_PIN_G, true);
  ledcSetup(LED_PWM_CHANNEL_G, LED_PWM_FREQ, LED_PWM_BITS);
  ledcAttachPin(LED_PIN_G, LED_PWM_CHANNEL_G);
  pinMode(LED_PIN_B, OUTPUT);
  digitalWrite(LED_PIN_B, true);
  ledcSetup(LED_PWM_CHANNEL_B, LED_PWM_FREQ, LED_PWM_BITS);
  ledcAttachPin(LED_PIN_B, LED_PWM_CHANNEL_B);
#endif
#ifdef HAS_LIGHTSENSOR
  // Setup CDS Light sensor
  analogSetAttenuation(ADC_0db); // 0dB(1.0x) 0~800mV
  pinMode(LIGHTSENSOR_IN, INPUT);
#endif
#ifdef HAS_SPEAKER
  // Audio
  pinMode(SPEAKER_PIN, INPUT); // Set high impedance
#endif

#if LV_USE_LOG
  lv_log_register_print_cb(lvgl_log);
#endif
  lv_init();

// Setup interfaces
#if  defined(HAS_ILI9431) || defined(HAS_ST7796)
  SPI.begin(TFT_SPI_SCLK, TFT_SPI_MISO, TFT_SPI_MOSI);
#endif

// XPT2046 sometimes shares the TFT
#ifdef HAS_XPT2046
  SPI.begin(TOUCH_SPI_SCLK, TOUCH_SPI_MISO, TOUCH_SPI_MOSI);
#endif

#if defined(HAS_CST820) || defined(HAS_GT911)
  Wire1.begin(TOUCH_IIC_SDA, TOUCH_IIC_SCL);
#endif

  // Setup TFT display
  lvgl_tft_init();
  static lv_disp_draw_buf_t draw_buf;
  static lv_color_t buf[DRAW_BUFFER_SIZE];
  lv_disp_draw_buf_init(&draw_buf, buf, NULL, DRAW_BUFFER_SIZE);

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

  // Clear screen
  lv_obj_clean(lv_scr_act());

// If there is a touch controller defined
#if defined(HAS_CST820) || defined(HAS_XPT2046) || defined(HAS_GT911)
  // Setup touch
  lvgl_touch_init();
  static lv_indev_drv_t indev_drv;
  lv_indev_drv_init(&indev_drv);
  indev_drv.type = LV_INDEV_TYPE_POINTER;
  indev_drv.read_cb = lvgl_touch_read;
  lv_indev_drv_register(&indev_drv);
#endif
}

#ifdef HAS_RGB_LED
void smartdisplay_set_led_color(lv_color32_t rgb)
{
  ledcWrite(LED_PWM_CHANNEL_R, LED_PWM_MAX - rgb.ch.red);
  ledcWrite(LED_PWM_CHANNEL_G, LED_PWM_MAX - rgb.ch.green);
  ledcWrite(LED_PWM_CHANNEL_B, LED_PWM_MAX - rgb.ch.blue);
}
#endif

#ifdef HAS_LIGHTSENSOR
int smartdisplay_get_light_intensity()
{
  return analogRead(LIGHTSENSOR_IN);
}
#endif

#ifdef SPEAKER_PIN
void smartdisplay_beep(unsigned int frequency, unsigned long duration)
{
  // Uses PWM Channel 0
  tone(SPEAKER_PIN, frequency, duration);
}
#endif
