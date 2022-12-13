#ifdef LVGL_TOUCH_GT911

#include <Arduino.h>
#include <Wire.h>

#include <lvgl_drv.h>
#include <lvgl_drv_touch_gt911.h>

TwoWire gt911_i2c = TwoWire(1); // Bus number 1
GTConfig gt911_config;

bool lvgl_touch_gt911_write_register(uint16_t reg, const uint8_t *buf, int len)
{
    gt911_i2c.beginTransmission(GT911_I2C_SLAVE_ADDR);
    if (!gt911_i2c.write(reg >> 8) || !gt911_i2c.write(reg & 0xFF))
        return false;

    auto sent = gt911_i2c.write(buf, len);
    gt911_i2c.endTransmission();
    return sent == len;
}

bool lvgl_touch_gt911_read_register(uint16_t reg, uint8_t *buf, int len)
{
    gt911_i2c.beginTransmission(GT911_I2C_SLAVE_ADDR);
    if (!gt911_i2c.write(reg >> 8) || !gt911_i2c.write(reg & 0xFF))
        return false;

    gt911_i2c.endTransmission(false);
    auto requested = gt911_i2c.requestFrom(GT911_I2C_SLAVE_ADDR, len);
    if (requested != len)
        return false;

    while (gt911_i2c.available() && len--)
        *buf++ = gt911_i2c.read();

    return len == 0;
}

bool lvgl_touch_gt911_read_info(const GTInfo *info)
{
    uint8_t *info_ptr = (uint8_t *)info;
    if (!lvgl_touch_gt911_read_register(GT911_REG_DATA, info_ptr, sizeof(GTInfo)))
    {
        log_e("Unable to read the GTInfo");
        return false;
    }

    return true;
}

bool lvgl_touch_gt911_read_config(GTConfig *config)
{
    uint8_t *config_ptr = (uint8_t *)config;
    if (!lvgl_touch_gt911_read_register(GT911_REG_CFG, config_ptr, sizeof(GTConfig)))
    {
        log_e("Unable to read GTConfig");
        return false;
    }

    uint8_t checksum;
    if (!lvgl_touch_gt911_read_register(GT911_REG_CHECKSUM, &checksum, sizeof(checksum)))
    {
        log_e("Unable to read the GTConfig checksum");
        return false;
    }

    // Validate the checksum
    for (auto i = 0; i < sizeof(GTConfig); ++i)
        checksum += *config_ptr++;

    return !checksum;
}

bool lvgl_touch_gt911_write_config(const GTConfig *config)
{
    uint8_t *config_ptr = (uint8_t *)config;
    uint8_t checksum = 0;
    for (auto i = 0; i < sizeof(GTConfig); ++i)
        checksum += *config_ptr++;

    // Two's complement (is minus)
    checksum = ~checksum + 1;
    if (!lvgl_touch_gt911_write_register(GT911_REG_CFG, config_ptr, sizeof(GTConfig)))
    {
        log_e("Unable to write the GTConfig");
        return false;
    }

    uint8_t checksum_buffer[2] = {checksum, 1};
    if (!lvgl_touch_gt911_write_register(GT911_REG_CHECKSUM, (uint8_t *)&checksum_buffer, sizeof(checksum_buffer)))
    {
        log_e("Unable to write the GTConfig checksum");
        return false;
    }

    return true;
}

int8_t lvgl_touch_gt911_num_points_available()
{
    uint8_t coord_addr;
    if (!lvgl_touch_gt911_read_register(GT911_REG_COORD_ADDR, &coord_addr, sizeof(coord_addr)))
    {
        log_e("Unable to read COORD_ADDR register");
        return 0;
    }

    if ((coord_addr & 0x80) && ((coord_addr & 0x0F) < GT911_MAX_CONTACTS))
    {
        static const uint8_t zero = 0;
        if (!lvgl_touch_gt911_write_register(GT911_REG_COORD_ADDR, &zero, sizeof(zero)))
        {
            log_e("Unable to reset COORD_ADDR register");
            return 0;
        }

        return coord_addr & 0x0F;
    }

    return 0;
}

bool lvgl_touch_gt911_read_touches(GTPoint *points, uint8_t numPoints = GT911_MAX_CONTACTS)
{
    if (!lvgl_touch_gt911_read_register(GT911_REG_COORD_ADDR + 1, (uint8_t *)points, sizeof(GTPoint) * numPoints))
    {
        log_e("Unable to read GTPoints");
        return false;
    }

#ifdef TFT_ORIENTATION_PORTRAIT
    for (uint8_t i = 0; i < numPoints; ++i)
    {
        points[i].x = TFT_WIDTH - points[i].x;
        points[i].y = TFT_HEIGHT - points[i].y;
    }
#else
#ifdef TFT_ORIENTATION_LANDSCAPE
    uint16_t swap;
    for (uint8_t i = 0; i < numPoints; ++i)
    {
        swap = points[i].x;
        points[i].x = points[i].y;
        points[i].y = TFT_WIDTH - swap;
    }
#else
#ifdef TFT_ORIENTATION_PORTRAIT_INV
    for (uint8_t i = 0; i < numPoints; ++i)
    {
        points[i].x = points[i].x;
        points[i].y = points[i].y;
    }
#else
#ifdef TFT_ORIENTATION_LANDSCAPE_INV
    uint16_t swap;
    for (uint8_t i = 0; i < numPoints; ++i)
    {
        swap = points[i].x;
        points[i].x = TFT_HEIGHT - points[i].y;
        points[i].y = swap;
    }
#else
#error TFT_ORIENTATION not defined!
#endif
#endif
#endif
#endif

    return true;
}

void lvgl_touch_init()
{
    gt911_i2c.begin(TOUCH_IIC_SDA, TOUCH_IIC_SCL);

    uint8_t productId[4];
    if (!lvgl_touch_gt911_read_register(GT911_PRODUCT_ID1, productId, sizeof(productId)))
    {
        log_e("No GT911 touch device found");
        return;
    }

    log_i("DeviceId: %c%c%c%c", productId[0], productId[1], productId[2], productId[3]);
}

void lvgl_touch_read(lv_indev_drv_t *drv, lv_indev_data_t *data)
{
    static int16_t last_x = 0;
    static int16_t last_y = 0;
    // Ignore multi-touch
    auto points_available = lvgl_touch_gt911_num_points_available();
    if (points_available == 1)
    {
        log_d("Touches present: %d", points_available);
        GTPoint point;
        if (lvgl_touch_gt911_read_touches(&point, 1))
        {
            log_d("Touch: (%d,%d)", point.x, point.y);
            data->state = LV_INDEV_STATE_PR;
            last_x = data->point.x = point.x;
            last_y = data->point.y = point.y;
        }
    }
    else
    {
        data->point.x = last_x;
        data->point.y = last_y;
        data->state = LV_INDEV_STATE_REL;
    }
}

#endif