#include <esp32_smartdisplay.h>
#include <esp_arduino_version.h>
#include <esp_lcd_types.h>
#include <esp_lcd_touch.h>
#include <esp_lcd_panel_ops.h>

#define BRIGHTNESS_UPDATE_INTERVAL 100
#define BRIGHTNESS_SMOOTHING_MEASUREMENTS 100
#define BRIGHTNESS_DARK_ZONE 100

// Functions to be defined in the tft/touch driver
extern void lvgl_tft_init(lv_disp_drv_t *drv);
extern void lvgl_touch_init(lv_indev_drv_t *drv);

static lv_disp_drv_t disp_drv;
static lv_indev_drv_t indev_drv;

#ifdef BOARD_HAS_CDS
static lv_timer_t *update_brightness_timer;
#endif

touch_calibration_data_t smartdisplay_touch_calibration_data;
void (*touch_read_cb)(struct _lv_indev_drv_t *indev_drv, lv_indev_data_t *data);

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

// See: https://www.maximintegrated.com/en/design/technical-documents/app-notes/5/5296.html

void lvgl_touch_calibration_transform(lv_indev_drv_t *drv, lv_indev_data_t *data)
{
  // Call low level read from the driver
  touch_read_cb(drv, data);
  // Check if transformation is required
  if (smartdisplay_touch_calibration_data.valid && data->state == LV_INDEV_STATE_PRESSED)
  {
    lv_point_t pt = {
        .x = roundf(data->point.x * smartdisplay_touch_calibration_data.alphaX + data->point.y * smartdisplay_touch_calibration_data.betaX + smartdisplay_touch_calibration_data.deltaX),
        .y = roundf(data->point.x * smartdisplay_touch_calibration_data.alphaY + data->point.y * smartdisplay_touch_calibration_data.betaY + smartdisplay_touch_calibration_data.deltaY)};
    log_i("Calibrate point (%d, %d) => (%d, %d)", data->point.x, data->point.y, pt.x, pt.y);
    data->point.x = pt.x;
    data->point.y = pt.y;
  }
}

void smartdisplay_compute_touch_calibration(const lv_point_t screen[3], const lv_point_t touch[3])
{
  smartdisplay_touch_calibration_data.valid = false;
  const int32_t delta = ((touch[0].x - touch[2].x) * (touch[1].y - touch[2].y)) - ((touch[1].x - touch[2].x) * (touch[0].y - touch[2].y));
  smartdisplay_touch_calibration_data.alphaX = (float)(((screen[0].x - screen[2].x) * (touch[1].y - touch[2].y)) - ((screen[1].x - screen[2].x) * (touch[0].y - touch[2].y))) / delta;
  smartdisplay_touch_calibration_data.betaX = (float)(((touch[0].x - touch[2].x) * (screen[1].x - screen[2].x)) - ((touch[1].x - touch[2].x) * (screen[0].x - screen[2].x))) / delta;
  smartdisplay_touch_calibration_data.deltaX = (((uint64_t)screen[0].x * ((touch[1].x * touch[2].y) - (touch[2].x * touch[1].y))) - ((uint64_t)screen[1].x * ((touch[0].x * touch[2].y) - (touch[2].x * touch[0].y))) + ((uint64_t)screen[2].x * ((touch[0].x * touch[1].y) - (touch[1].x * touch[0].y)))) / delta;
  smartdisplay_touch_calibration_data.alphaY = (float)(((screen[0].y - screen[2].y) * (touch[1].y - touch[2].y)) - ((screen[1].y - screen[2].y) * (touch[0].y - touch[2].y))) / delta;
  smartdisplay_touch_calibration_data.betaY = (float)(((touch[0].x - touch[2].x) * (screen[1].y - screen[2].y)) - ((touch[1].x - touch[2].x) * (screen[0].y - screen[2].y))) / delta;
  smartdisplay_touch_calibration_data.deltaY = (((uint64_t)screen[0].y * (touch[1].x * touch[2].y - touch[2].x * touch[1].y)) - ((uint64_t)screen[1].y * (touch[0].x * touch[2].y - touch[2].x * touch[0].y)) + ((uint64_t)screen[2].y * (touch[0].x * touch[1].y - touch[1].x * touch[0].y))) / delta;

  log_i("Calibration (a,b,c,d,e,f) = (%f, %f, %f, %f, %f, %f)", smartdisplay_touch_calibration_data.alphaX, smartdisplay_touch_calibration_data.betaX, smartdisplay_touch_calibration_data.deltaX, smartdisplay_touch_calibration_data.alphaY, smartdisplay_touch_calibration_data.betaY, smartdisplay_touch_calibration_data.deltaY);
  smartdisplay_touch_calibration_data.valid = true;
};

#define CROSS_LENGHT 5
#define READ_CALIBRATE_POINTS 10

void (*original_read_cb)(struct _lv_indev_drv_t *indev_drv, lv_indev_data_t *data);
int calibrate_point_count;
lv_point_t calibrate_points[READ_CALIBRATE_POINTS];

void lvgl_touch_calibrate_cb(lv_indev_drv_t *drv, lv_indev_data_t *data)
{
  // Call low level read from the driver
  touch_read_cb(drv, data);
  if (data->state == LV_INDEV_STATE_PRESSED && calibrate_point_count < READ_CALIBRATE_POINTS)
  {
    log_i("Calibrate got point %d: (%d, %d)", calibrate_point_count, data->point.x, data->point.y);
    calibrate_points[calibrate_point_count].x = data->point.x;
    calibrate_points[calibrate_point_count].y = data->point.y;
    calibrate_point_count++;
  }
}

void smartdisplay_calibrate()
{
  log_i("Starting calibration");
  // Save touch read callback
  original_read_cb = indev_drv.read_cb;
  indev_drv.read_cb = lvgl_touch_calibrate_cb;

  // Create screen points
  const lv_point_t screen_pt[3] = {
      {.x = LCD_WIDTH * .1, .y = LCD_HEIGHT * .1},
      {.x = LCD_WIDTH * .5, .y = LCD_HEIGHT * .5},
      {.x = LCD_WIDTH * .9, .y = LCD_HEIGHT * .9}};

  // Results for touch
  lv_point_t touch_pt[3];

  lv_obj_t *screen = lv_scr_act();
  // No nothing
  lv_obj_remove_style_all(screen);

  // Create style
  lv_style_t style_line;
  lv_style_init(&style_line);
  lv_style_set_line_width(&style_line, 3);
  lv_style_set_line_color(&style_line, lv_color_black());

  lv_obj_t *cross;

  lv_point_t *pt = (lv_point_t *)&screen_pt;
  for (int p = 0; p < 3; p++, pt++)
  {
    // Clear the screen
    lv_obj_clean(screen);
    delay(1000);
    log_i("Calibrate screen point %d: (%d, %d)", p, pt->x, pt->y);

    // Cross
    lv_point_t line_points[] = {
        {pt->x - CROSS_LENGHT, pt->y},
        {pt->x + CROSS_LENGHT, pt->y},
        {pt->x, pt->y},
        {pt->x, pt->y - CROSS_LENGHT},
        {pt->x, pt->y + CROSS_LENGHT}};

    cross = lv_line_create(screen);
    lv_line_set_points(cross, line_points, sizeof(line_points) / sizeof(lv_point_t));
    lv_obj_add_style(cross, &style_line, 0);
    lv_obj_align(cross, LV_ALIGN_TOP_LEFT, 0, 0);

    calibrate_point_count = 0;
    while (calibrate_point_count < READ_CALIBRATE_POINTS)
      lv_timer_handler();

    lv_obj_del(cross);
    lv_timer_handler();

    // Calculate the average
    uint total_x = 0;
    uint total_y = 0;
    for (int i = 0; i < READ_CALIBRATE_POINTS; ++i)
    {
      total_x += calibrate_points[i].x;
      total_y += calibrate_points[i].y;
    }

    touch_pt[p].x = total_x / READ_CALIBRATE_POINTS;
    touch_pt[p].y = total_y / READ_CALIBRATE_POINTS;

    log_i("result %d: Screen (%d,%d), Touch (%d, %d)", p, pt->x, pt->y, touch_pt[p].x, touch_pt[p].y);
  }

  smartdisplay_compute_touch_calibration(screen_pt, touch_pt);

  // Restore original touch read callback
  indev_drv.read_cb = original_read_cb;
}

void update_brightness(lv_timer_t *timer)
{
  static float avgCds;
  // Read CdS sensor
  uint16_t sensorValue = analogRead(CDS_GPIO);
  // Approximation of moving average for the sensor
  avgCds -= avgCds / BRIGHTNESS_SMOOTHING_MEASUREMENTS;
  avgCds += sensorValue / BRIGHTNESS_SMOOTHING_MEASUREMENTS;
  // Section of interest is 0 = full light until ~500 darkish
  int16_t lightValue = BRIGHTNESS_DARK_ZONE - avgCds;
  if (lightValue < 0)
    lightValue = 0;
  // Set fixed percentage and variable based on CdS sensor
  uint32_t pwmDuty = 0.01 * PWM_MAX_BCKL + (0.99 * PWM_MAX_BCKL / BRIGHTNESS_DARK_ZONE) * lightValue;
// Set backlight
#if ESP_ARDUINO_VERSION_MAJOR >= 3
  ledcWrite(LCD_BCKL_GPIO, pwmDuty);
#else
  ledcWrite(PWM_CHANNEL_BCKL, pwmDuty);
#endif
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
  // Turn backlight on (50%)
  smartdisplay_tft_set_backlight(0.5f);

// If there is a touch controller defined
#ifdef BOARD_HAS_TOUCH
  // Setup touch
  lv_indev_drv_init(&indev_drv);
  indev_drv.disp = display;
  lvgl_touch_init(&indev_drv);
  // Chain calibration handler
  touch_read_cb = indev_drv.read_cb;
  indev_drv.read_cb = lvgl_touch_calibration_transform;
  lv_indev_drv_register(&indev_drv);
#endif
  // Register callback for changes to the driver parameters
  disp_drv.drv_update_cb = lvgl_update_callback;
  // Call the callback to set the rotation
  lvgl_update_callback(&disp_drv);

#ifdef BOARD_HAS_CDS
  update_brightness_timer = lv_timer_create(update_brightness, BRIGHTNESS_UPDATE_INTERVAL, NULL);
#endif

  smartdisplay_calibrate();
}

#ifdef BOARD_HAS_CDS
void smartdisplay_set_auto_brightness(bool enable)
{
  if (enable)
    lv_timer_resume(update_brightness_timer);
  else
    lv_timer_pause(update_brightness_timer);
}
#endif

void smartdisplay_tft_set_backlight(float duty)
{
  log_i("Set backlight PWM ratio to: %.2f%%", duty * 100);
#if ESP_ARDUINO_VERSION_MAJOR >= 3
  ledcWrite(LCD_BCKL_GPIO, dut * PWM_MAX_BCKL);
#else
  ledcWrite(PWM_CHANNEL_BCKL, duty * PWM_MAX_BCKL);
#endif
}