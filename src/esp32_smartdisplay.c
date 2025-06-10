#include <esp32_smartdisplay.h>
#include <esp_lcd_panel_ops.h>

#ifdef BOARD_HAS_TOUCH
#include <esp_lcd_touch.h>
#endif

// Defines for adaptive brightness adjustment
#define BRIGHTNESS_SMOOTHING_MEASUREMENTS 100
#define BRIGHTNESS_DARK_ZONE 250

// Functions to be defined in the tft/touch driver
extern lv_display_t *lvgl_lcd_init();
extern lv_indev_t *lvgl_touch_init();

lv_display_t *display;

#ifdef BOARD_HAS_TOUCH
lv_indev_t *indev;
touch_calibration_data_t touch_calibration_data;
void (*driver_touch_read_cb)(lv_indev_t *indev, lv_indev_data_t *data);
#endif

void lvgl_display_resolution_changed_callback(lv_event_t *drv);

lv_timer_t *update_brightness_timer;

#ifdef LV_USE_LOG
void lvgl_log(lv_log_level_t level, const char *buf)
{
  switch (level)
  {
  case LV_LOG_LEVEL_TRACE:
    log_printf("%s", buf);
    break;
  case LV_LOG_LEVEL_INFO:
    log_i("%s", buf);
    break;
  case LV_LOG_LEVEL_WARN:
    log_w("%s", buf);
    break;
  case LV_LOG_LEVEL_ERROR:
    log_e("%s", buf);
    break;
  }
}
#endif

// Set backlight intensity
void smartdisplay_lcd_set_backlight(float duty)
{
  log_v("duty:%2f", duty);

  if (duty > 1.0f)
    duty = 1.0f;
  if (duty < 0.0f)
    duty = 0.0f;
#if ESP_ARDUINO_VERSION_MAJOR >= 3
  ledcWrite(DISPLAY_BCKL, duty * PWM_MAX_BCKL);
#else
  ledcWrite(PWM_CHANNEL_BCKL, duty * PWM_MAX_BCKL);
#endif
}

#ifdef BOARD_HAS_CDS
// Read CdS sensor and return a value for the screen brightness
float smartdisplay_lcd_adaptive_brightness_cds()
{
  static float avgCds;
  // Read CdS sensor
  uint16_t sensorValue = analogRead(CDS);
  // Approximation of moving average for the sensor
  avgCds -= avgCds / BRIGHTNESS_SMOOTHING_MEASUREMENTS;
  avgCds += sensorValue / BRIGHTNESS_SMOOTHING_MEASUREMENTS;
  // Section of interest is 0 (full light) until ~500 (darkish)
  int16_t lightValue = BRIGHTNESS_DARK_ZONE - avgCds;
  if (lightValue < 0)
    lightValue = 0;
  // Set fixed percentage and variable based on CdS sensor
  return 0.01 + (0.99 / BRIGHTNESS_DARK_ZONE) * lightValue;
}
#endif

void adaptive_brightness(lv_timer_t *timer)
{
  log_v("timer:0x%08x", timer);

  const smartdisplay_lcd_adaptive_brightness_cb_t callback = timer->user_data;
  smartdisplay_lcd_set_backlight(callback());
}

void smartdisplay_lcd_set_brightness_cb(smartdisplay_lcd_adaptive_brightness_cb_t cb, uint interval)
{
  log_v("adaptive_brightness_cb:0x%08x, interval:%u", cb, interval);

  // Delete current timer if any
  if (update_brightness_timer != NULL)
    lv_timer_del(update_brightness_timer);

  // Use callback for intensity or 50% default
  if (cb != NULL && interval > 0)
    update_brightness_timer = lv_timer_create(adaptive_brightness, interval, cb);
  else
    smartdisplay_lcd_set_backlight(0.5f);
}

#ifdef BOARD_HAS_RGB_LED
void smartdisplay_led_set_rgb(bool r, bool g, bool b)
{
  log_d("R:%d, G:%d, B:%d", r, g, b);

  digitalWrite(RGB_LED_R, !r);
  digitalWrite(RGB_LED_G, !g);
  digitalWrite(RGB_LED_B, !b);
}
#endif

#ifdef BOARD_HAS_TOUCH
// See: https://www.maximintegrated.com/en/design/technical-documents/app-notes/5/5296.html
void lvgl_touch_calibration_transform(lv_indev_t *indev, lv_indev_data_t *data)
{
  log_v("indev:0x%08x, data:0x%08x", indev, data);

  // Call low level read from the driver
  driver_touch_read_cb(indev, data);
  // Check if transformation is required
  if (touch_calibration_data.valid && data->state == LV_INDEV_STATE_PRESSED)
  {
    log_d("lvgl_touch_calibration_transform: transformation applied");
    lv_point_t pt = {
        .x = roundf(data->point.x * touch_calibration_data.alphaX + data->point.y * touch_calibration_data.betaX + touch_calibration_data.deltaX),
        .y = roundf(data->point.x * touch_calibration_data.alphaY + data->point.y * touch_calibration_data.betaY + touch_calibration_data.deltaY)};
    log_d("Calibrate point (%d, %d) => (%d, %d)", data->point.x, data->point.y, pt.x, pt.y);
    data->point = (lv_point_t){pt.x, pt.y};
  }
}

touch_calibration_data_t smartdisplay_compute_touch_calibration(const lv_point_t screen[3], const lv_point_t touch[3])
{
  log_v("screen:0x%08x, touch:0x%08x", screen, touch);
  const float delta = ((touch[0].x - touch[2].x) * (touch[1].y - touch[2].y)) - ((touch[1].x - touch[2].x) * (touch[0].y - touch[2].y));
  touch_calibration_data_t touch_calibration_data = {
      .valid = true,
      .alphaX = (((screen[0].x - screen[2].x) * (touch[1].y - touch[2].y)) - ((screen[1].x - screen[2].x) * (touch[0].y - touch[2].y))) / delta,
      .betaX = (((touch[0].x - touch[2].x) * (screen[1].x - screen[2].x)) - ((touch[1].x - touch[2].x) * (screen[0].x - screen[2].x))) / delta,
      .deltaX = ((screen[0].x * ((touch[1].x * touch[2].y) - (touch[2].x * touch[1].y))) - (screen[1].x * ((touch[0].x * touch[2].y) - (touch[2].x * touch[0].y))) + (screen[2].x * ((touch[0].x * touch[1].y) - (touch[1].x * touch[0].y)))) / delta,
      .alphaY = (((screen[0].y - screen[2].y) * (touch[1].y - touch[2].y)) - ((screen[1].y - screen[2].y) * (touch[0].y - touch[2].y))) / delta,
      .betaY = (((touch[0].x - touch[2].x) * (screen[1].y - screen[2].y)) - ((touch[1].x - touch[2].x) * (screen[0].y - screen[2].y))) / delta,
      .deltaY = ((screen[0].y * (touch[1].x * touch[2].y - touch[2].x * touch[1].y)) - (screen[1].y * (touch[0].x * touch[2].y - touch[2].x * touch[0].y)) + (screen[2].y * (touch[0].x * touch[1].y - touch[1].x * touch[0].y))) / delta,
  };

  log_d("alphaX: %f, betaX: %f, deltaX: %f, alphaY: %f, betaY: %f, deltaY: %f", touch_calibration_data.alphaX, touch_calibration_data.betaX, touch_calibration_data.deltaX, touch_calibration_data.alphaY, touch_calibration_data.betaY, touch_calibration_data.deltaY);
  return touch_calibration_data;
};
#endif

void smartdisplay_init()
{
  log_d("smartdisplay_init");
#ifdef BOARD_HAS_RGB_LED
  // Setup RGB LED.  High is off
  pinMode(RGB_LED_R, OUTPUT);
  pinMode(RGB_LED_G, OUTPUT);
  pinMode(RGB_LED_B, OUTPUT);
  smartdisplay_led_set_rgb(false, false, false);
#endif

#ifdef BOARD_HAS_CDS
  // CDS Light sensor
  pinMode(CDS, INPUT);
  analogSetAttenuation(ADC_0db); // 0dB(1.0x) 0~800mV
#endif

#ifdef BOARD_HAS_SPEAK
  // Speaker
  // Note: tone function uses PWM channel 0
  pinMode(SPEAK, OUTPUT);
#endif

#if LV_USE_LOG
  lv_log_register_print_cb(lvgl_log);
#endif

  lv_init();
  // Setup backlight
  pinMode(DISPLAY_BCKL, OUTPUT);
  digitalWrite(DISPLAY_BCKL, LOW);
#if ESP_ARDUINO_VERSION_MAJOR >= 3
  ledcAttach(DISPLAY_BCKL, PWM_FREQ_BCKL, PWM_BITS_BCKL);
#else
  ledcSetup(PWM_CHANNEL_BCKL, PWM_FREQ_BCKL, PWM_BITS_BCKL);
  ledcAttachPin(DISPLAY_BCKL, PWM_CHANNEL_BCKL);
#endif
  // Setup TFT display
  display = lvgl_lcd_init();

#ifndef DISPLAY_SOFTWARE_ROTATION
  // Register callback for hardware rotation
  lv_display_add_event_cb(display, lvgl_display_resolution_changed_callback, LV_EVENT_RESOLUTION_CHANGED, NULL);
#endif

  //  Clear screen
  lv_obj_clean(lv_scr_act());
  // Turn backlight on (50%)
  smartdisplay_lcd_set_backlight(0.5f);

// If there is a touch controller defined
#ifdef BOARD_HAS_TOUCH
  // Setup touch
  indev = lvgl_touch_init();
  indev->disp = display;
  // Intercept callback
  driver_touch_read_cb = indev->read_cb;
  indev->read_cb = lvgl_touch_calibration_transform;
  lv_indev_enable(indev, true);
#endif
}

#ifndef DISPLAY_SOFTWARE_ROTATION
// Called when driver resolution is updated (including rotation)
// Top of the display is top left when connector is at the bottom
// The rotation values are relative to how you would rotate the physical display in the clockwise direction.
// So, LV_DISPLAY_ROTATION_90 means you rotate the hardware 90 degrees clockwise, and the display rotates 90 degrees counterclockwise to compensate.
void lvgl_display_resolution_changed_callback(lv_event_t *event)
{
  const esp_lcd_panel_handle_t panel_handle = display->user_data;
  switch (display->rotation)
  {
  case LV_DISPLAY_ROTATION_0:
    ESP_ERROR_CHECK(esp_lcd_panel_swap_xy(panel_handle, DISPLAY_SWAP_XY));
    ESP_ERROR_CHECK(esp_lcd_panel_mirror(panel_handle, DISPLAY_MIRROR_X, DISPLAY_MIRROR_Y));
    break;
  case LV_DISPLAY_ROTATION_90:
    ESP_ERROR_CHECK(esp_lcd_panel_swap_xy(panel_handle, !DISPLAY_SWAP_XY));
    ESP_ERROR_CHECK(esp_lcd_panel_mirror(panel_handle, DISPLAY_MIRROR_X, !DISPLAY_MIRROR_Y));
    break;
  case LV_DISPLAY_ROTATION_180:
    ESP_ERROR_CHECK(esp_lcd_panel_swap_xy(panel_handle, DISPLAY_SWAP_XY));
    ESP_ERROR_CHECK(esp_lcd_panel_mirror(panel_handle, !DISPLAY_MIRROR_X, !DISPLAY_MIRROR_Y));
    break;
  case LV_DISPLAY_ROTATION_270:
    ESP_ERROR_CHECK(esp_lcd_panel_swap_xy(panel_handle, !DISPLAY_SWAP_XY));
    ESP_ERROR_CHECK(esp_lcd_panel_mirror(panel_handle, !DISPLAY_MIRROR_X, DISPLAY_MIRROR_Y));
    break;
  }
}

#endif