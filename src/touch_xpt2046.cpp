#include <esp32_smartdisplay.h>

#ifdef XPT2046

#define CMD_START_Z1_CONVERSION 0xB1
#define CMD_START_Z2_CONVERSION 0xC1
#define CMD_START_X_CONVERSION 0xD1
#define CMD_START_Y_CONVERSION 0x91
#define Z_THRESHOLD 600

bool xpt2046_read_xy(int16_t *x, int16_t *y) {
  spi_xpt2046.beginTransaction(SPISettings(XPT2046_SPI_FREQ, MSBFIRST, SPI_MODE0));
  digitalWrite(XPT2046_PIN_CS, LOW);
  spi_xpt2046.transfer16(CMD_START_Z1_CONVERSION);
  auto z1 = spi_xpt2046.transfer16(CMD_START_Z2_CONVERSION) >> 3;
  auto z2 = spi_xpt2046.transfer16(CMD_START_X_CONVERSION) >> 3;
  auto raw_x = spi_xpt2046.transfer16(CMD_START_Y_CONVERSION) >> 3;  // Normalize to 12 bits
  auto raw_y = spi_xpt2046.transfer16(0) >> 3;                       // Normalize to 12 bits
  digitalWrite(XPT2046_PIN_CS, HIGH);
  spi_xpt2046.endTransaction();
  int16_t z = z1 + 4095 - z2;
  if (z < Z_THRESHOLD)
    return false;

#if 0  // For calibration
    static auto min_x = INT_MAX, max_x = -INT_MAX, min_y = INT_MAX, max_y = -INT_MAX;
    if (raw_x < min_x)
        min_x = raw_x;
    if (raw_x > max_x)
        max_x = raw_x;
    if (raw_y < min_y)
        min_y = raw_y;
    if (raw_y > max_y)
        max_y = raw_y;
    log_i("min_x=%d, max_x=%d, min_y=%d, max_y=%d", min_x, max_x, min_y, max_y);
#endif

#ifdef TFT_ORIENTATION_PORTRAIT
  *x = ((raw_x - XPT2046_MIN_X) * TFT_WIDTH) / (XPT2046_MAX_X - XPT2046_MIN_X);
  *y = TFT_HEIGHT - ((raw_y - XPT2046_MIN_Y) * TFT_HEIGHT) / (XPT2046_MAX_Y - XPT2046_MIN_Y);
#else
#ifdef TFT_ORIENTATION_LANDSCAPE
  *x = ((raw_y - XPT2046_MIN_Y) * TFT_HEIGHT) / (XPT2046_MAX_Y - XPT2046_MIN_Y);
  *y = ((raw_x - XPT2046_MIN_X) * TFT_WIDTH) / (XPT2046_MAX_X - XPT2046_MIN_X);
#else
#ifdef TFT_ORIENTATION_PORTRAIT_INV
  *x = TFT_WIDTH - ((raw_x - XPT2046_MIN_X) * TFT_WIDTH) / (XPT2046_MAX_X - XPT2046_MIN_X);
  *y = ((raw_y - XPT2046_MIN_Y) * TFT_HEIGHT) / (XPT2046_MAX_Y - XPT2046_MIN_Y);
#else
#ifdef TFT_ORIENTATION_LANDSCAPE_INV
  *x = TFT_HEIGHT - ((raw_y - XPT2046_MIN_Y) * TFT_HEIGHT) / (XPT2046_MAX_Y - XPT2046_MIN_Y);
  *y = TFT_WIDTH - ((raw_x - XPT2046_MIN_X) * TFT_WIDTH) / (XPT2046_MAX_X - XPT2046_MIN_X);
#else
#error TFT_ORIENTATION not defined!
#endif
#endif
#endif
#endif
  // log_i("x=%d,y=%d,raw_z=%d", *x, *y, z);
  return true;
}

void lvgl_touch_init() {
  pinMode(XPT2046_PIN_CS, OUTPUT);
  digitalWrite(XPT2046_PIN_CS, true);
}

void lvgl_touch_read(lv_indev_drv_t *drv, lv_indev_data_t *data) {
  static int16_t last_x = 0, last_y = 0;
  // log_d("Touch: (%d,%d)", last_x, last_y);
  data->state = xpt2046_read_xy(&last_x, &last_y) ? LV_INDEV_STATE_PR : LV_INDEV_STATE_RELEASED;
  data->point.x = last_x;
  data->point.y = last_y;
  // log_d("Touch: (%d,%d)", last_x, last_y);
}

#endif