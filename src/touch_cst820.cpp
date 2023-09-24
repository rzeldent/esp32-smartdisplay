#include <esp32_smartdisplay.h>

#ifdef CST820

#define CST820_I2C_SLAVE_ADDR 0x15

enum Gesture : uint8_t
{
  None = 0x00,
  SlideDown = 0x01,
  SlideUp = 0x02,
  SlideLeft = 0x03,
  SlideRight = 0x04,
  SingleTap = 0x05,
  DoubleTap = 0x0B,
  LongPress = 0x0C
};

#if !defined(TFT_ORIENTATION_PORTRAIT) && !defined(TFT_ORIENTATION_LANDSCAPE) && !defined(TFT_ORIENTATION_PORTRAIT_INV) && !defined(TFT_ORIENTATION_LANDSCAPE_INV)
#error Please define orientation: TFT_ORIENTATION_PORTRAIT, TFT_ORIENTATION_LANDSCAPE, TFT_ORIENTATION_PORTRAIT_INV or TFT_ORIENTATION_LANDSCAPE_INV
#endif

bool cst820_write_register(uint8_t reg, const uint8_t buf[], int len)
{
  i2c_cst820.beginTransmission(CST820_I2C_SLAVE_ADDR);
  if (!i2c_cst820.write(reg))
    return false;

  auto sent = i2c_cst820.write(buf, len);
  i2c_cst820.endTransmission();
  return sent == len;
}

bool cst820_read_register(uint8_t reg, uint8_t buf[], int len)
{
  i2c_cst820.beginTransmission(CST820_I2C_SLAVE_ADDR);
  if (!i2c_cst820.write(reg))
    return false;

  i2c_cst820.endTransmission(false);
  auto requested = i2c_cst820.requestFrom(CST820_I2C_SLAVE_ADDR, len);
  if (requested != len)
    return false;

  while (i2c_cst820.available() && len--)
    *buf++ = i2c_cst820.read();

  return len == 0;
}

bool cst820_read_touch(uint16_t *x, uint16_t *y, uint8_t *gesture)
{
  uint8_t fingerIndex;
  if (!cst820_read_register(0x02, &fingerIndex, 1) || !cst820_read_register(0x01, gesture, 1))
    return false;

  uint8_t data[4];
  if (!cst820_read_register(0x03, data, 4))
    return false;

#ifdef TFT_ORIENTATION_PORTRAIT
  *x = TFT_WIDTH - (((data[0] & 0x0f) << 8) | data[1]);
  *y = TFT_HEIGHT - (((data[2] & 0x0f) << 8) | data[3]);
#else
#ifdef TFT_ORIENTATION_LANDSCAPE
  *x = ((data[2] & 0x0f) << 8) | data[3];
  *y = TFT_WIDTH - (((data[0] & 0x0f) << 8) | data[1]);
#else
#ifdef TFT_ORIENTATION_PORTRAIT_INV
  *x = ((data[0] & 0x0f) << 8) | data[1];
  *y = ((data[2] & 0x0f) << 8) | data[3];
#else
#ifdef TFT_ORIENTATION_LANDSCAPE_INV
  *x = TFT_HEIGHT - (((data[2] & 0x0f) << 8) | data[3]);
  *y = ((data[0] & 0x0f) << 8) | data[1];
#else
#error TFT_ORIENTATION not defined!
#endif
#endif
#endif
#endif
  return (bool)fingerIndex;
}

void lvgl_touch_init()
{
  static const uint8_t power[] = { 0xFF };
  cst820_write_register(0xFE, power, sizeof(power)); // Disable automatic entry into low power mode
}

void lvgl_touch_read(lv_indev_drv_t *drv, lv_indev_data_t *data)
{
  static int16_t last_x = 0, last_y = 0;
  uint16_t x, y;
  uint8_t gesture;
  if (cst820_read_touch(&x, &y, &gesture) && gesture == Gesture::None)
  {
    log_d("Touch: (%d,%d)", x, y);
    data->state = LV_INDEV_STATE_PR;
    last_x = data->point.x = x;
    last_y = data->point.y = y;
  }
  else
  {
    data->point.x = last_x;
    data->point.y = last_y;
    data->state = LV_INDEV_STATE_REL;
  }
}

#endif