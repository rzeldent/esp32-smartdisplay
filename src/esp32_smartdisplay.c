#include <esp32_smartdisplay.h>
#include <esp_arduino_version.h>
#include <esp_lcd_types.h>
#include <esp_lcd_touch.h>
#include <esp_lcd_panel_ops.h>

// Defines for adaptive brightness adjustment
#define BRIGHTNESS_UPDATE_INTERVAL 100
#define BRIGHTNESS_SMOOTHING_MEASUREMENTS 100
#define BRIGHTNESS_DARK_ZONE 250

// Defines for calibration
#define CALIBRATION_CROSS_LENGTH 10

// Functions to be defined in the lcd/touch driver
extern void lvgl_tft_init(lv_disp_drv_t *drv);
extern void lvgl_touch_init(lv_indev_drv_t *drv);

// Static variables
lv_disp_drv_t disp_drv;
lv_indev_drv_t indev_drv;
#ifdef BOARD_HAS_TOUCH
touch_calibration_data_t touch_calibration_data;
lv_point_t calibration_point;
void (*driver_touch_read_cb)(struct _lv_indev_drv_t *indev_drv, lv_indev_data_t *data);
#endif
lv_timer_t *update_brightness_timer;

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
  const esp_lcd_panel_handle_t panel_handle = disp_drv.user_data;
  const esp_lcd_touch_handle_t touch_handle = indev_drv.user_data;
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

// Calibration and touch point adjustment
#ifdef BOARD_HAS_TOUCH
// See: https://www.maximintegrated.com/en/design/technical-documents/app-notes/5/5296.html
void lvgl_touch_calibration_transform(lv_indev_drv_t *drv, lv_indev_data_t *data)
{
  // Call low level read from the driver
  driver_touch_read_cb(drv, data);
  // Check if transformation is required
  if (touch_calibration_data.valid && data->state == LV_INDEV_STATE_PRESSED)
  {
    lv_point_t pt = {
        .x = roundf(data->point.x * touch_calibration_data.alphaX + data->point.y * touch_calibration_data.betaX + touch_calibration_data.deltaX),
        .y = roundf(data->point.x * touch_calibration_data.alphaY + data->point.y * touch_calibration_data.betaY + touch_calibration_data.deltaY)};
    log_i("Calibrate point (%d, %d) => (%d, %d)", data->point.x, data->point.y, pt.x, pt.y);
    data->point = (lv_point_t){pt.x, pt.y};
  }
}

void smartdisplay_compute_touch_calibration(const lv_point_t screen[3], const lv_point_t touch[3])
{
  touch_calibration_data.valid = false;
  const float delta = ((touch[0].x - touch[2].x) * (touch[1].y - touch[2].y)) - ((touch[1].x - touch[2].x) * (touch[0].y - touch[2].y));
  touch_calibration_data = (touch_calibration_data_t){
      .valid = true,
      .alphaX = (((screen[0].x - screen[2].x) * (touch[1].y - touch[2].y)) - ((screen[1].x - screen[2].x) * (touch[0].y - touch[2].y))) / delta,
      .betaX = (((touch[0].x - touch[2].x) * (screen[1].x - screen[2].x)) - ((touch[1].x - touch[2].x) * (screen[0].x - screen[2].x))) / delta,
      .deltaX = ((screen[0].x * ((touch[1].x * touch[2].y) - (touch[2].x * touch[1].y))) - (screen[1].x * ((touch[0].x * touch[2].y) - (touch[2].x * touch[0].y))) + (screen[2].x * ((touch[0].x * touch[1].y) - (touch[1].x * touch[0].y)))) / delta,
      .alphaY = (((screen[0].y - screen[2].y) * (touch[1].y - touch[2].y)) - ((screen[1].y - screen[2].y) * (touch[0].y - touch[2].y))) / delta,
      .betaY = (((touch[0].x - touch[2].x) * (screen[1].y - screen[2].y)) - ((touch[1].x - touch[2].x) * (screen[0].y - screen[2].y))) / delta,
      .deltaY = ((screen[0].y * (touch[1].x * touch[2].y - touch[2].x * touch[1].y)) - (screen[1].y * (touch[0].x * touch[2].y - touch[2].x * touch[0].y)) + (screen[2].y * (touch[0].x * touch[1].y - touch[1].x * touch[0].y))) / delta,
  };

  log_i("Calibration (alphaX, betaX, deltaX, alphaY, betaY, deltaY) = (%f, %f, %f, %f, %f, %f)", touch_calibration_data.alphaX, touch_calibration_data.betaX, touch_calibration_data.deltaX, touch_calibration_data.alphaY, touch_calibration_data.betaY, touch_calibration_data.deltaY);
};

void btn_click_action_calibration(lv_event_t *event)
{
  if (event)
  {
    // Get the hardware coordinates from the driver here.
    lv_indev_t *indev = lv_indev_get_act();
    lv_indev_get_point(indev, &calibration_point);
    log_i("Calibration touch at %d, %d\n", calibration_point.x, calibration_point.y);
  }
}

void smartdisplay_touch_calibrate()
{
  log_i("Starting calibration");

  // Disable calibration adjustment
  touch_calibration_data.valid = false;

  // Create screen points
  const lv_point_t screen_pt[] = {
      {.x = CALIBRATION_CROSS_LENGTH, .y = CALIBRATION_CROSS_LENGTH},               // X=~0, Y=~0
      {.x = LCD_WIDTH - CALIBRATION_CROSS_LENGTH, .y = LCD_HEIGHT / 2},             // X=~Max, Y=~Max/2
      {.x = CALIBRATION_CROSS_LENGTH, .y = LCD_HEIGHT - CALIBRATION_CROSS_LENGTH}}; // X=~0, Y=~Max

  // Results for touch points
  lv_point_t touch_pt[sizeof(screen_pt) / sizeof(lv_point_t)];

  // Save the old screen
  lv_obj_t *oldscreen = lv_scr_act();

  // Create a calibration screen, full size!
  lv_obj_t *screen_calibration = lv_obj_create(NULL);
  lv_obj_clear_flag(screen_calibration, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_style_text_align(screen_calibration, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_size(screen_calibration, LV_HOR_RES, LV_VER_RES);
  // Make the screen one big button
  lv_obj_t *calibration_button_area = lv_btn_create(screen_calibration);
  // Use the complete screen
  lv_obj_set_size(calibration_button_area, LV_HOR_RES, LV_VER_RES);
  // Set some text
  lv_obj_t *calibration_scrren_text = lv_label_create(screen_calibration);
  lv_obj_set_size(calibration_scrren_text, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
  lv_obj_set_align(calibration_scrren_text, LV_ALIGN_CENTER);
  lv_label_set_text(calibration_scrren_text, "CALIBRATION\n\nUse the stylus to click on the crosses\nthat appear as precise as possible.");

  // lv_obj_remove_style(calibration_button_area, NULL, LV_PART_MAIN | LV_STATE_DEFAULT);

  // lv_obj_set_style_opa(big_btn, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT); // Opacity zero
  lv_obj_add_event_cb(calibration_button_area, btn_click_action_calibration, LV_EVENT_CLICKED, NULL);
  lv_obj_set_layout(calibration_button_area, 0); // Disable layout of children. The first registered layout starts at 1

  lv_scr_load(screen_calibration);

  lv_point_t *pt = (lv_point_t *)&screen_pt;
  for (int p = 0; p < sizeof(screen_pt) / sizeof(lv_point_t); p++, pt++)
  {
    // Clear the screen
    lv_obj_clean(calibration_button_area);
    delay(1000);
    log_i("Calibrate screen point %d: (%d, %d)", p, pt->x, pt->y);

    // Cross
    lv_point_t line_points[] = {
        {pt->x - CALIBRATION_CROSS_LENGTH, pt->y},
        {pt->x + CALIBRATION_CROSS_LENGTH, pt->y},
        {pt->x, pt->y},
        {pt->x, pt->y - CALIBRATION_CROSS_LENGTH},
        {pt->x, pt->y + CALIBRATION_CROSS_LENGTH}};

    // Create style
    lv_style_t style_line;
    lv_style_init(&style_line);
    lv_style_set_line_width(&style_line, 1);
    lv_style_set_line_color(&style_line, lv_color_black());

    lv_obj_t *cross = lv_line_create(calibration_button_area);
    lv_line_set_points(cross, line_points, sizeof(line_points) / sizeof(lv_point_t));
    lv_obj_add_style(cross, &style_line, 0);
    lv_obj_align(cross, LV_ALIGN_TOP_LEFT, 0, 0);

    calibration_point = (lv_point_t){-1, -1};
    while (calibration_point.y == -1)
      lv_timer_handler();

    touch_pt[p] = calibration_point;
    lv_obj_del(cross);
    lv_timer_handler();

    log_i("Result %d: Screen (%d,%d), Touch (%d, %d)", p, pt->x, pt->y, touch_pt[p].x, touch_pt[p].y);
  }

  // Calculate the transform parameters
  smartdisplay_compute_touch_calibration(screen_pt, touch_pt);
}
#endif

// Backlight
void smartdisplay_lcd_set_backlight(float duty)
{
  if (duty > 1.0)
    duty = 1.0f;
  if (duty < 0.0)
    duty = 0.0f;
#if ESP_ARDUINO_VERSION_MAJOR >= 3
  ledcWrite(LCD_BCKL_GPIO, duty * PWM_MAX_BCKL);
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
  uint16_t sensorValue = analogRead(CDS_GPIO);
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
  const smartdisplay_lcd_adaptive_brightness_cb_t callback = timer->user_data;
  smartdisplay_lcd_set_backlight(callback());
}

void smartdisplay_lcd_set_brightness_cb(smartdisplay_lcd_adaptive_brightness_cb_t cb, uint interval)
{
  // Delete current timer if any
  if (update_brightness_timer)
    lv_timer_del(update_brightness_timer);

  // Use callback for intensity or 50% default
  if (cb && interval > 0)
    update_brightness_timer = lv_timer_create(adaptive_brightness, interval, cb);
  else
    smartdisplay_lcd_set_backlight(0.5f);
}

#ifdef BOARD_HAS_RGB_LED
void smartdisplay_led_set_rgb(bool r, bool g, bool b)
{
  digitalWrite(LED_R_GPIO, !r);
  digitalWrite(LED_G_GPIO, !g);
  digitalWrite(LED_B_GPIO, !b);
}
#endif

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

#ifdef BOARD_HAS_CDS
  // CDS Light sensor. Is shorted to ground over a voltage divider (1M Ohm) connected between 3.3V and ground.
  // So, see GT36516 specs: Dark = 0.3M, Light 5k. Theory: voltage between 1.28V (dark) and 16mV (light)
  // But threshold is ~0.15mv so light
  pinMode(CDS_GPIO, INPUT);
  // 0dB(1.0x) 0~800mV, 1V input => ADC reading of 1088
  analogSetAttenuation(ADC_0db);
#endif

#ifdef BOARD_HAS_SPEAK
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

#ifdef BOARD_HAS_CDS
  // Enable auto brightness based on CdS
  smartdisplay_lcd_set_brightness_cb(smartdisplay_lcd_adaptive_brightness_cds, BRIGHTNESS_UPDATE_INTERVAL);
#else
  // Turn backlight on (50%)
  smartdisplay_lcd_set_backlight(0.5f);
#endif

// If there is a touch controller defined
#ifdef BOARD_HAS_TOUCH
  // Setup touch
  lv_indev_drv_init(&indev_drv);
  indev_drv.disp = display;
  lvgl_touch_init(&indev_drv);
  // Chain calibration handler for calibration
  driver_touch_read_cb = indev_drv.read_cb;
  indev_drv.read_cb = lvgl_touch_calibration_transform;
  lv_indev_t *input_device = lv_indev_drv_register(&indev_drv);
#endif
  // Register callback for changes to the driver parameters
  disp_drv.drv_update_cb = lvgl_update_callback;
  // Call the callback to set the rotation
  lvgl_update_callback(&disp_drv);
}
