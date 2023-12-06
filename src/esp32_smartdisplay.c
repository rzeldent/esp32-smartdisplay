#include <esp32_smartdisplay.h>

#include <esp_lcd_types.h>
#include <esp_lcd_touch.h>
#include <esp_lcd_panel_ops.h>

// Functions to be defined in the tft/touch driver
extern void lvgl_tft_init(lv_disp_drv_t *drv);
extern void lvgl_touch_init(lv_indev_drv_t *drv);

static lv_disp_drv_t disp_drv;
static lv_indev_drv_t indev_drv;

#if LV_USE_LOG
void lvgl_log(const char *buf)
{
  log_printf("%s", buf);
}
#endif

// Called when driver parameters are updated (rotation)
// Top of the display is top left when connector is at the bottom
static void lvgl_update_callback(lv_disp_drv_t *drv)
{
  esp_lcd_panel_handle_t panel_handle = disp_drv.user_data;
  esp_lcd_touch_handle_t touch_handle = indev_drv.user_data;
#ifdef PANEL_SWAP_XY
  switch (drv->rotated)
  {
  case LV_DISP_ROT_NONE:
    ESP_ERROR_CHECK(esp_lcd_panel_swap_xy(panel_handle, PANEL_SWAP_XY));
    ESP_ERROR_CHECK(esp_lcd_panel_mirror(panel_handle, PANEL_MIRROR_X, PANEL_MIRROR_Y));
#if defined(PANEL_GAP_X) || defined(PANEL_GAP_Y)
    ESP_ERROR_CHECK(esp_lcd_panel_set_gap(panel_handle, PANEL_GAP_X, PANEL_GAP_Y));
#endif
#ifdef USES_TOUCH
    ESP_ERROR_CHECK(esp_lcd_touch_set_mirror_x(touch_handle, TOUCH_ROT_SWAP_X));
    ESP_ERROR_CHECK(esp_lcd_touch_set_mirror_y(touch_handle, TOUCH_ROT_SWAP_Y));
#endif
    break;
  case LV_DISP_ROT_90:
    ESP_ERROR_CHECK(esp_lcd_panel_swap_xy(panel_handle, !PANEL_SWAP_XY));
    ESP_ERROR_CHECK(esp_lcd_panel_mirror(panel_handle, !PANEL_MIRROR_X, PANEL_MIRROR_Y));
#if defined(PANEL_GAP_X) || defined(PANEL_GAP_Y)
    ESP_ERROR_CHECK(esp_lcd_panel_set_gap(panel_handle, PANEL_GAP_Y, PANEL_GAP_X));
#endif
#ifdef USES_TOUCH
    ESP_ERROR_CHECK(esp_lcd_touch_set_mirror_x(touch_handle, !TOUCH_ROT_SWAP_X));
    ESP_ERROR_CHECK(esp_lcd_touch_set_mirror_y(touch_handle, !TOUCH_ROT_SWAP_Y));
#endif
    break;
  case LV_DISP_ROT_180:
    ESP_ERROR_CHECK(esp_lcd_panel_swap_xy(panel_handle, PANEL_SWAP_XY));
    ESP_ERROR_CHECK(esp_lcd_panel_mirror(panel_handle, !PANEL_MIRROR_X, !PANEL_MIRROR_Y));
#if defined(PANEL_GAP_X) || defined(PANEL_GAP_Y)
    ESP_ERROR_CHECK(esp_lcd_panel_set_gap(panel_handle, PANEL_GAP_X, PANEL_GAP_Y));
#endif
#ifdef USES_TOUCH
    ESP_ERROR_CHECK(esp_lcd_touch_set_mirror_x(touch_handle, TOUCH_ROT_SWAP_X));
    ESP_ERROR_CHECK(esp_lcd_touch_set_mirror_y(touch_handle, TOUCH_ROT_SWAP_Y));
#endif
    break;
  case LV_DISP_ROT_270:
    ESP_ERROR_CHECK(esp_lcd_panel_swap_xy(panel_handle, !PANEL_SWAP_XY));
    ESP_ERROR_CHECK(esp_lcd_panel_mirror(panel_handle, PANEL_MIRROR_X, !PANEL_MIRROR_Y));
#if defined(PANEL_GAP_X) || defined(PANEL_GAP_Y)
    ESP_ERROR_CHECK(esp_lcd_panel_set_gap(panel_handle, PANEL_GAP_Y, PANEL_GAP_X));
#endif
#ifdef USES_TOUCH
    ESP_ERROR_CHECK(esp_lcd_touch_set_mirror_x(touch_handle, !TOUCH_ROT_SWAP_X));
    ESP_ERROR_CHECK(esp_lcd_touch_set_mirror_y(touch_handle, !TOUCH_ROT_SWAP_Y));
#endif
    break;
  }
#endif
}

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
  lv_disp_drv_init(&disp_drv);
  disp_drv.hor_res = TFT_WIDTH;
  disp_drv.ver_res = TFT_HEIGHT;
  // Create drawBuffer
  disp_drv.draw_buf = (lv_disp_draw_buf_t *)malloc(sizeof(lv_disp_draw_buf_t));
  uint drawBufferPixels = TFT_WIDTH * LVGL_PIXEL_BUFFER_LINES;
  void *drawBuffer = heap_caps_malloc(sizeof(lv_color_t) * drawBufferPixels, MALLOC_CAP_DMA);
  lv_disp_draw_buf_init(disp_drv.draw_buf, drawBuffer, NULL, drawBufferPixels);
  // Initialize specific driver
  lvgl_tft_init(&disp_drv);
  lv_disp_t *display = lv_disp_drv_register(&disp_drv);
  // Clear screen
  lv_obj_clean(lv_scr_act());
  // Turn backlight on
  ledcWrite(PWM_CHANNEL_BCKL, PWM_MAX_BCKL);

// If there is a touch controller defined
#ifdef USES_TOUCH
  // Setup touch
  lv_indev_drv_init(&indev_drv);
  indev_drv.disp = display;
  lvgl_touch_init(&indev_drv);
  lv_indev_drv_register(&indev_drv);
#endif

  // Register callback for changes to the driver parameters
  disp_drv.drv_update_cb = lvgl_update_callback;
  // Call the callback to set the rotation
  lvgl_update_callback(&disp_drv);
}

void smartdisplay_tft_set_backlight(uint16_t duty)
{
  ledcWrite(PWM_CHANNEL_BCKL, duty);
}