#include <esp32_smartdisplay.h>
#include <esp_arduino_version.h>
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
  switch (drv->rotated)
  {
  case LV_DISP_ROT_NONE:
#if defined(LCD_SWAP_XY) && defined(LCD_MIRROR_X) && defined(LCD_MIRROR_Y)
    ESP_ERROR_CHECK(esp_lcd_panel_swap_xy(panel_handle, LCD_SWAP_XY));
    ESP_ERROR_CHECK(esp_lcd_panel_mirror(panel_handle, LCD_MIRROR_X, LCD_MIRROR_Y));
#endif    
#if defined(LCD_GAP_X) || defined(LCD_GAP_Y)
    ESP_ERROR_CHECK(esp_lcd_panel_set_gap(panel_handle, LCD_GAP_X, LCD_GAP_Y));
#endif
#ifdef BOARD_HAS_TOUCH
    ESP_ERROR_CHECK(esp_lcd_touch_set_mirror_x(touch_handle, TOUCH_SWAP_X));
    ESP_ERROR_CHECK(esp_lcd_touch_set_mirror_y(touch_handle, TOUCH_SWAP_Y));
#endif
    break;
  case LV_DISP_ROT_90:
#if defined(LCD_SWAP_XY) && defined(LCD_MIRROR_X) && defined(LCD_MIRROR_Y)
    ESP_ERROR_CHECK(esp_lcd_panel_swap_xy(panel_handle, !LCD_SWAP_XY));
    ESP_ERROR_CHECK(esp_lcd_panel_mirror(panel_handle, !LCD_MIRROR_X, LCD_MIRROR_Y));
#endif    
#if defined(LCD_GAP_X) || defined(LCD_GAP_Y)
    ESP_ERROR_CHECK(esp_lcd_panel_set_gap(panel_handle, LCD_GAP_Y, LCD_GAP_X));
#endif
#ifdef BOARD_HAS_TOUCH
    ESP_ERROR_CHECK(esp_lcd_touch_set_mirror_x(touch_handle, !TOUCH_SWAP_X));
    ESP_ERROR_CHECK(esp_lcd_touch_set_mirror_y(touch_handle, !TOUCH_SWAP_Y));
#endif
    break;
  case LV_DISP_ROT_180:
#if defined(LCD_SWAP_XY) && defined(LCD_MIRROR_X) && defined(LCD_MIRROR_Y)
    ESP_ERROR_CHECK(esp_lcd_panel_swap_xy(panel_handle, LCD_SWAP_XY));
    ESP_ERROR_CHECK(esp_lcd_panel_mirror(panel_handle, !LCD_MIRROR_X, !LCD_MIRROR_Y));
#endif    
#if defined(LCD_GAP_X) || defined(LCD_GAP_Y)
    ESP_ERROR_CHECK(esp_lcd_panel_set_gap(panel_handle, LCD_GAP_X, LCD_GAP_Y));
#endif
#ifdef BOARD_HAS_TOUCH
    ESP_ERROR_CHECK(esp_lcd_touch_set_mirror_x(touch_handle, TOUCH_SWAP_X));
    ESP_ERROR_CHECK(esp_lcd_touch_set_mirror_y(touch_handle, TOUCH_SWAP_Y));
#endif
    break;
  case LV_DISP_ROT_270:
#if defined(LCD_SWAP_XY) && defined(LCD_MIRROR_X) && defined(LCD_MIRROR_Y)
    ESP_ERROR_CHECK(esp_lcd_panel_swap_xy(panel_handle, !LCD_SWAP_XY));
    ESP_ERROR_CHECK(esp_lcd_panel_mirror(panel_handle, LCD_MIRROR_X, !LCD_MIRROR_Y));
#endif    
#if defined(LCD_GAP_X) || defined(LCD_GAP_Y)
    ESP_ERROR_CHECK(esp_lcd_panel_set_gap(panel_handle, LCD_GAP_Y, LCD_GAP_X));
#endif
#ifdef BOARD_HAS_TOUCH
    ESP_ERROR_CHECK(esp_lcd_touch_set_mirror_x(touch_handle, !TOUCH_SWAP_X));
    ESP_ERROR_CHECK(esp_lcd_touch_set_mirror_y(touch_handle, !TOUCH_SWAP_Y));
#endif
    break;
  }
}

void smartdisplay_init()
{
#ifdef BOARD_HAS_RGB_LED
  // Setup RGB LED.  High is off
  pinMode(LED_R_GPIO, OUTPUT);
  digitalWrite(LED_R_GPIO, true);
  pinMode(LED_G_GPIO, OUTPUT);
  digitalWrite(LED_G_GPIO, true);
  pinMode(LED_B_GPIO, OUTPUT);
  digitalWrite(LED_B_GPIO, true);
#endif

#ifdef HAS_LIGHTSENSOR
  // CDS Light sensor
  analogSetAttenuation(ADC_0db); // 0dB(1.0x) 0~800mV
  pinMode(LIGHTSENSOR_IN, INPUT);
#endif

#ifdef BOARD_HAS_SPEAK_GPIO
  // Speaker
  // Note: tone function uses PWM channel 0
  pinMode(SPEAK_GPIO, OUTPUT);
#endif

#if LV_USE_LOG
  lv_log_register_print_cb(lvgl_log);
#endif

  lv_init();
  // Setup backlight
  pinMode(LCD_BCKL_GPIO, OUTPUT);
#if ESP_ARDUINO_VERSION_MAJOR >= 3
  ledcAttach(LCD_BCKL_GPIO, PWM_FREQ_BCKL, PWM_BITS_BCKL);
#else
  ledcSetup(PWM_CHANNEL_BCKL, PWM_FREQ_BCKL, PWM_BITS_BCKL);
  ledcAttachPin(LCD_BCKL_GPIO, PWM_CHANNEL_BCKL);
#endif
  digitalWrite(LCD_BCKL_GPIO, LOW);
  // Setup TFT display
  lv_disp_drv_init(&disp_drv);
  disp_drv.hor_res = LCD_WIDTH;
  disp_drv.ver_res = LCD_HEIGHT;
  // Create drawBuffer
  disp_drv.draw_buf = (lv_disp_draw_buf_t *)malloc(sizeof(lv_disp_draw_buf_t));
  uint drawBufferPixels = LCD_WIDTH * LVGL_PIXEL_BUFFER_LINES;
  void *drawBuffer = heap_caps_malloc(sizeof(lv_color16_t) * drawBufferPixels, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
  lv_disp_draw_buf_init(disp_drv.draw_buf, drawBuffer, NULL, drawBufferPixels);
  // Initialize specific driver
  lvgl_tft_init(&disp_drv);
  lv_disp_t *display = lv_disp_drv_register(&disp_drv);
  // Clear screen
  lv_obj_clean(lv_scr_act());
  // Turn backlight on (50%)
  smartdisplay_tft_set_backlight(0.5f);

// If there is a touch controller defined
#ifdef BOARD_HAS_TOUCH
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

void smartdisplay_tft_set_backlight(float duty)
{
#if ESP_ARDUINO_VERSION_MAJOR >= 3
  ledcWrite(LCD_BCKL_GPIO, dut * PWM_MAX_BCKL);
#else
  ledcWrite(PWM_CHANNEL_BCKL, duty * PWM_MAX_BCKL);
#endif
}