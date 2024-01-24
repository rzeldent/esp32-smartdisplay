#ifdef BOARD_HAS_XPT2046

/*
 * SPDX-FileCopyrightText: 2022 atanisoft (github.com/atanisoft)
 *
 * SPDX-License-Identifier: MIT
 */

#include <driver/gpio.h>
#include <esp_check.h>
#include <esp_err.h>
#include <esp_lcd_panel_io.h>
#include <esp_rom_gpio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
// This must be included after FreeRTOS includes due to missing include
// for portMUX_TYPE
#include <esp_lcd_touch.h>
#include <memory.h>

#include "sdkconfig.h"

static const char *TAG = "xpt2046";

enum xpt2046_registers
{
                        // START  ADDR  SER/  INT   VREF    ADC
                        //              DFR   ENA   INT/EXT ENA
    Z_VALUE_1   = 0xB1, // 1      011   0     0     0       1
    Z_VALUE_2   = 0xC1, // 1      100   0     0     0       1
    Y_POSITION  = 0x91, // 1      001   0     0     0       1
    X_POSITION  = 0xD1, // 1      101   0     0     0       1
    BATTERY     = 0xA7  // 1      010   0     1     1       1
};

#if CONFIG_XPT2046_ENABLE_LOCKING
#define XPT2046_LOCK(lock) portENTER_CRITICAL(lock)
#define XPT2046_UNLOCK(lock) portEXIT_CRITICAL(lock)
#else
#define XPT2046_LOCK(lock)
#define XPT2046_UNLOCK(lock)
#endif

static const uint16_t XPT2046_ADC_LIMIT = 4096;
static esp_err_t xpt2046_read_data(esp_lcd_touch_handle_t tp);
static bool xpt2046_get_xy(esp_lcd_touch_handle_t tp,
                           uint16_t *x, uint16_t *y,
                           uint16_t *strength,
                           uint8_t *point_num,
                           uint8_t max_point_num);
static esp_err_t xpt2046_del(esp_lcd_touch_handle_t tp);

esp_err_t esp_lcd_touch_new_spi_xpt2046(const esp_lcd_panel_io_handle_t io,
                                        const esp_lcd_touch_config_t *config,
                                        esp_lcd_touch_handle_t *out_touch)
{
    esp_err_t ret = ESP_OK;
    esp_lcd_touch_handle_t handle = NULL;

    ESP_GOTO_ON_FALSE(io, ESP_ERR_INVALID_ARG, err, TAG,
                      "esp_lcd_panel_io_handle_t must not be NULL");
    ESP_GOTO_ON_FALSE(config, ESP_ERR_INVALID_ARG, err, TAG,
                      "esp_lcd_touch_config_t must not be NULL");

    handle = (esp_lcd_touch_handle_t)calloc(1, sizeof(esp_lcd_touch_t));
    ESP_GOTO_ON_FALSE(handle, ESP_ERR_NO_MEM, err, TAG,
                      "No memory available for XPT2046 state");
    handle->io = io;
    handle->read_data = xpt2046_read_data;
    handle->get_xy = xpt2046_get_xy;
    handle->del = xpt2046_del;
    handle->data.lock.owner = portMUX_FREE_VAL;
    memcpy(&handle->config, config, sizeof(esp_lcd_touch_config_t));

    // this is not yet supported by esp_lcd_touch.
    if (config->int_gpio_num != GPIO_NUM_NC)
    {
        ESP_GOTO_ON_FALSE(GPIO_IS_VALID_GPIO(config->int_gpio_num),
            ESP_ERR_INVALID_ARG, err, TAG, "Invalid GPIO Interrupt Pin");
        gpio_config_t cfg;
        memset(&cfg, 0, sizeof(gpio_config_t));
        esp_rom_gpio_pad_select_gpio(config->int_gpio_num);
        cfg.pin_bit_mask = BIT64(config->int_gpio_num);
        cfg.mode = GPIO_MODE_INPUT;

        // If the user has provided a callback routine for the interrupt enable
        // the interrupt mode on the negative edge.
        if (config->interrupt_callback)
        {
            cfg.intr_type = GPIO_INTR_NEGEDGE;
        }

        ESP_GOTO_ON_ERROR(gpio_config(&cfg), err, TAG,
                          "Configure GPIO for Interrupt failed");

        // Connect the user interrupt callback routine.
        if (config->interrupt_callback)
        {
            esp_lcd_touch_register_interrupt_callback(handle, config->interrupt_callback);
        }
    }

err:
    if (ret != ESP_OK)
    {
        if (handle)
        {
            xpt2046_del(handle);
            handle = NULL;
        }
    }

    *out_touch = handle;

    return ret;
}

static esp_err_t xpt2046_del(esp_lcd_touch_handle_t tp)
{
    if (tp != NULL)
    {
        if (tp->config.int_gpio_num != GPIO_NUM_NC)
        {
            gpio_reset_pin(tp->config.int_gpio_num);
        }
    }
    free(tp);

    return ESP_OK;
}

static inline esp_err_t xpt2046_read_register(esp_lcd_touch_handle_t tp, uint8_t reg, uint16_t *value)
{
    uint8_t buf[2] = {0, 0};
    ESP_RETURN_ON_ERROR(esp_lcd_panel_io_rx_param(tp->io, reg, buf, 2), TAG, "XPT2046 read error!");
    *value = ((buf[0] << 8) | (buf[1]));
    return ESP_OK;
}

static esp_err_t xpt2046_read_data(esp_lcd_touch_handle_t tp)
{
    uint16_t z1 = 0, z2 = 0, z = 0;
    uint32_t x = 0, y = 0;
    uint8_t point_count = 0;

    ESP_RETURN_ON_ERROR(xpt2046_read_register(tp, Z_VALUE_1, &z1), TAG, "XPT2046 read error!");
    ESP_RETURN_ON_ERROR(xpt2046_read_register(tp, Z_VALUE_2, &z2), TAG, "XPT2046 read error!");

    // Convert the received values into a Z value.
    z = (z1 >> 3) + (XPT2046_ADC_LIMIT - (z2 >> 3));

    // If the Z (pressure) exceeds the threshold it is likely the user has
    // pressed the screen, read in and average the positions.
    if (z >= CONFIG_XPT2046_Z_THRESHOLD)
    {
        uint16_t discard_buf = 0;

        // read and discard a value as it is usually not reliable.
        ESP_RETURN_ON_ERROR(xpt2046_read_register(tp, X_POSITION, &discard_buf),
                            TAG, "XPT2046 read error!");

        for (uint8_t idx = 0; idx < CONFIG_ESP_LCD_TOUCH_MAX_POINTS; idx++)
        {
            uint16_t x_temp = 0;
            uint16_t y_temp = 0;
            // Read X position and convert returned data to 12bit value
            ESP_RETURN_ON_ERROR(xpt2046_read_register(tp, X_POSITION, &x_temp),
                                TAG, "XPT2046 read error!");
            // drop lowest three bits to convert to 12-bit position
            x_temp >>= 3;

#if CONFIG_XPT2046_CONVERT_ADC_TO_COORDS
            // Convert the raw ADC value into a screen coordinate and store it
            // for averaging.
            x += ((x_temp / (double)XPT2046_ADC_LIMIT) * tp->config.x_max);
#else
            // store the raw ADC values and let the user convert them to screen
            // coordinates.
            x += x_temp;
#endif // CONFIG_XPT2046_CONVERT_ADC_TO_COORDS

            // Read Y position and convert returned data to 12bit value
            ESP_RETURN_ON_ERROR(xpt2046_read_register(tp, Y_POSITION, &y_temp),
                                TAG, "XPT2046 read error!");
            // drop lowest three bits to convert to 12-bit position
            y_temp >>= 3;

#if CONFIG_XPT2046_CONVERT_ADC_TO_COORDS
            // Convert the raw ADC value into a screen coordinate and store it
            // for averaging.
            y += ((y_temp / (double)XPT2046_ADC_LIMIT) * tp->config.y_max);
#else
            // store the raw ADC values and let the user convert them to screen
            // coordinates.
            y += y_temp;
#endif // CONFIG_XPT2046_CONVERT_ADC_TO_COORDS
        }

        // Average the accumulated coordinate data points.
        x /= CONFIG_ESP_LCD_TOUCH_MAX_POINTS;
        y /= CONFIG_ESP_LCD_TOUCH_MAX_POINTS;
        point_count = 1;
    }

    XPT2046_LOCK(&tp->data.lock);
    tp->data.coords[0].x = x;
    tp->data.coords[0].y = y;
    tp->data.coords[0].strength = z;
    tp->data.points = point_count;
    XPT2046_UNLOCK(&tp->data.lock);

    return ESP_OK;
}

static bool xpt2046_get_xy(esp_lcd_touch_handle_t tp, uint16_t *x, uint16_t *y,
                           uint16_t *strength, uint8_t *point_num,
                           uint8_t max_point_num)
{
    XPT2046_LOCK(&tp->data.lock);

    // Determine how many touch points that are available.
    if (tp->data.points > max_point_num)
    {
        *point_num = max_point_num;
    }
    else
    {
        *point_num = tp->data.points;
    }

    for (size_t i = 0; i < *point_num; i++)
    {
        x[i] = tp->data.coords[i].x;
        y[i] = tp->data.coords[i].y;

        if (strength)
        {
            strength[i] = tp->data.coords[i].strength;
        }
    }

    // Invalidate stored touch data.
    tp->data.points = 0;

    XPT2046_UNLOCK(&tp->data.lock);

    if (*point_num)
    {
        ESP_LOGD(TAG, "Touch point: %dx%d", x[0], y[0]);
    }
    else
    {
        ESP_LOGV(TAG, "No touch points");
    }

    return (*point_num > 0);
}

esp_err_t esp_lcd_touch_xpt2046_read_battery_level(const esp_lcd_touch_handle_t handle, float *output)
{
    uint16_t level;
    ESP_RETURN_ON_ERROR(xpt2046_read_register(handle, BATTERY, &level), TAG, "XPT2046 read error!");
    
    // battery voltage is reported as 1/4 the actual voltage due to logic in
    // the chip.
    *output = level * 4.0;

    // adjust for internal vref of 2.5v
    *output *= 2.5f;

    // adjust for ADC bit count
    *output /= 4096.0f;

    return ESP_OK;
}

#endif