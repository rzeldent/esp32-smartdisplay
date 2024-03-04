#ifdef TOUCH_XPT2046_SPI

#include <esp_lcd_touch_xpt2046.h>
#include <string.h>
#include <esp_rom_gpio.h>
#include <esp32-hal-log.h>

// See datasheet XPT2046.pdf
const uint8_t XPT2046_START_Z1_CONVERSION = 0xB1;  // S=1, ADDR=011, MODE=0 (12bits), SER/DFR=0, PD1=0, PD2=1
const uint8_t XPT2046_START_Z2_CONVERSION = 0xC1;  // S=1, ADDR=100, MODE=0 (12bits), SER/DFR=0, PD1=0, PD2=1
const uint8_t XPT2046_START_Y_CONVERSION = 0x91;   // S=1, ADDR=001, MODE=0 (12bits), SER/DFR=0, PD1=0, PD2=1
const uint8_t XPT2046_START_X_CONVERSION = 0xD1;   // S=1, ADDR=101, MODE=0 (12bits), SER/DFR=0, PD1=0, PD2=1
const uint8_t XPT2046_START_BAT_CONVERSION = 0xA7; // S=1, ADDR=010, MODE=0 (12bits), SER/DFR=1, PD1=1, PD2=1
const uint8_t XPT2046_START_Z1_POWER_DOWN = 0xB0;  // S=1, ADDR=011, MODE=0 (12bits), SER/DFR=1, PD1=0, PD2=0
// 12 bits ADC limit
const uint16_t XPT2046_ADC_LIMIT = (1 << 12); // 4096

#ifdef __cplusplus
extern "C"
{
#endif

    esp_err_t xpt2046_read_register(esp_lcd_touch_handle_t th, uint8_t reg, uint16_t *value)
    {
        uint8_t buf[2];
        esp_err_t res = esp_lcd_panel_io_rx_param(th->io, reg, buf, sizeof(buf));
        if (res != ESP_OK)
            return res;

        *value = (buf[0] << 8) + buf[1];

        return ESP_OK;
    }

    esp_err_t xpt2046_enter_sleep(esp_lcd_touch_handle_t th)
    {
        log_v("xpt2046_enter_sleep. th:0x%08x", th);

        esp_err_t res;
        uint16_t discard;
        if ((res = xpt2046_read_register(th, XPT2046_START_Z1_POWER_DOWN, &discard)) != ESP_OK)
        {
            log_w("Could not read XPT2046_START_Z1_POWER_DOWN");
            return res;
        }

        return ESP_OK;
    }

    esp_err_t xpt2046_exit_sleep(esp_lcd_touch_handle_t th)
    {
        log_v("xpt2046_exit_sleep. th:0x%08x", th);

        esp_err_t res;
        uint16_t discard;
        if ((res = xpt2046_read_register(th, XPT2046_START_Z1_CONVERSION, &discard)) != ESP_OK)
        {
            log_w("Could not read XPT2046_START_Z1_CONVERSION");
            return res;
        }

        return ESP_OK;
    }

    esp_err_t xpt2046_read_data(esp_lcd_touch_handle_t th)
    {
        log_v("xpt2046_read_data. th:0x%08x", th);

        esp_err_t res;
        uint32_t x = 0, y = 0;
        uint8_t points = 0;

        uint16_t z1, z2;
        if (((res = xpt2046_read_register(th, XPT2046_START_Z1_CONVERSION, &z1)) != ESP_OK) ||
            ((res = xpt2046_read_register(th, XPT2046_START_Z2_CONVERSION, &z2)) != ESP_OK))
        {
            log_w("Could not XPT2046_START_Z1_CONVERSION or XPT2046_START_Z2_CONVERSION");
            return res;
        }

        // Convert to 12 bits Z value.
        uint16_t z = (z1 >> 3) + (XPT2046_ADC_LIMIT - (z2 >> 3));
        // If the Z exceeds the Z threshold the user has pressed the screen
        if (z >= XPT2046_Z_THRESHOLD)
        {
            uint16_t x_temp, y_temp;
            // Discard first value as it is usually not reliable.
            if ((res = xpt2046_read_register(th, XPT2046_START_X_CONVERSION, &x_temp)) != ESP_OK)
            {
                log_w("Could not read XPT2046_START_X_CONVERSION");
                return res;
            }

            // CONFIG_ESP_LCD_TOUCH_MAX_POINTS is to average the points read and gives a better precision
            for (uint8_t idx = 0; idx < CONFIG_ESP_LCD_TOUCH_MAX_POINTS; idx++)
            {
                // Read X and Y positions
                if (((res = xpt2046_read_register(th, XPT2046_START_X_CONVERSION, &x_temp)) != ESP_OK) ||
                    ((res = xpt2046_read_register(th, XPT2046_START_Y_CONVERSION, &y_temp)) != ESP_OK))
                {
                    log_w("Could not read XPT2046_START_X_CONVERSION or XPT2046_START_Y_CONVERSION");
                    return res;
                }

                // Add to accumulated raw ADC values
                x += x_temp;
                y += y_temp;
            }

            // Convert X and Y to 12 bits by dropping upper 3 bits and average the accumulated coordinate data points.
            x = (double)(x >> 3) / XPT2046_ADC_LIMIT / CONFIG_ESP_LCD_TOUCH_MAX_POINTS * th->config.x_max;
            y = (double)(y >> 3) / XPT2046_ADC_LIMIT / CONFIG_ESP_LCD_TOUCH_MAX_POINTS * th->config.y_max;
            points = 1;
        }

        portENTER_CRITICAL(&th->data.lock);
        th->data.coords[0].x = x;
        th->data.coords[0].y = y;
        th->data.coords[0].strength = z;
        th->data.points = points;
        portEXIT_CRITICAL(&th->data.lock);

        return ESP_OK;
    }

    bool xpt2046_get_xy(esp_lcd_touch_handle_t th, uint16_t *x, uint16_t *y, uint16_t *strength, uint8_t *point_num, uint8_t max_point_num)
    {
        log_v("xpt2046_get_xy. th:0x%08x, x:0x%08x, y:0x%08x, strength:0x%08x, point_num:0x%08x, max_point_num:%d", th, x, y, strength, point_num, max_point_num);

        portENTER_CRITICAL(&th->data.lock);
        *point_num = th->data.points > max_point_num ? max_point_num : th->data.points;
        for (uint8_t i = 0; i < *point_num; i++)
        {
            if (th->config.flags.swap_xy)
            {
                x[i] = th->config.flags.mirror_y ? th->config.y_max - th->data.coords[i].y : th->data.coords[i].y;
                y[i] = th->config.flags.mirror_x ? th->config.x_max - th->data.coords[i].x : th->data.coords[i].x;
            }
            else
            {
                x[i] = th->config.flags.mirror_x ? th->config.x_max - th->data.coords[i].x : th->data.coords[i].x;
                y[i] = th->config.flags.mirror_y ? th->config.y_max - th->data.coords[i].y : th->data.coords[i].y;
            }

            if (strength != NULL)
                strength[i] = th->data.coords[i].strength;
        }

        th->data.points = 0;
        portEXIT_CRITICAL(&th->data.lock);

        return *point_num > 0;
    }

    esp_err_t xpt2046_del(esp_lcd_touch_handle_t th)
    {
        log_v("xpt2046_del. th:0x%08x", th);

        portENTER_CRITICAL(&th->data.lock);
        // Remove interrupts and reset INT
        if (th->config.int_gpio_num != GPIO_NUM_NC)
        {
            if (th->config.interrupt_callback != NULL)
                gpio_isr_handler_remove(th->config.int_gpio_num);

            gpio_reset_pin(th->config.int_gpio_num);
        }

        free(th);

        return ESP_OK;
    }

    esp_err_t esp_lcd_touch_new_spi_xpt2046(const esp_lcd_panel_io_handle_t io, const esp_lcd_touch_config_t *config, esp_lcd_touch_handle_t *handle)
    {
        log_v("esp_lcd_touch_new_spi_xpt2046. io:0x%08x, config:0x%08x, handle:0x%08x", io, config, handle);

        assert(io != NULL);
        assert(config != NULL);
        assert(handle != NULL);

        if (config->int_gpio_num != GPIO_NUM_NC && !GPIO_IS_VALID_GPIO(config->int_gpio_num))
        {
            log_e("Invalid GPIO INT pin: %d", config->int_gpio_num);
            return ESP_ERR_INVALID_ARG;
        }

        esp_err_t res;
        const esp_lcd_touch_handle_t th = calloc(1, sizeof(esp_lcd_touch_t));
        if (th == NULL)
        {
            log_e("No memory available for esp_lcd_touch_t");
            return ESP_ERR_NO_MEM;
        }

        th->io = io;
        th->enter_sleep = xpt2046_enter_sleep;
        th->exit_sleep = xpt2046_exit_sleep;
        th->read_data = xpt2046_read_data;
        th->get_xy = xpt2046_get_xy;
        th->del = xpt2046_del;
        th->data.lock.owner = portMUX_FREE_VAL;
        memcpy(&th->config, config, sizeof(esp_lcd_touch_config_t));

        if (config->int_gpio_num != GPIO_NUM_NC)
        {
            esp_rom_gpio_pad_select_gpio(config->int_gpio_num);
            const gpio_config_t cfg = {
                .pin_bit_mask = BIT64(config->int_gpio_num),
                .mode = GPIO_MODE_INPUT,
                // If the user has provided a callback routine for the interrupt enable the interrupt mode on the negative edge.
                .intr_type = config->interrupt_callback ? GPIO_INTR_NEGEDGE : GPIO_INTR_DISABLE};
            if ((res = gpio_config(&cfg)) != ESP_OK)
            {
                free(th);
                log_e("Configuring GPIO for INT failed");
                return res;
            }

            if (config->interrupt_callback != NULL)
            {
                if ((res = esp_lcd_touch_register_interrupt_callback(th, config->interrupt_callback)) != ESP_OK)
                {
                    gpio_reset_pin(th->config.int_gpio_num);
                    free(th);
                    log_e("Registering INT callback failed");
                    return res;
                }
            }
        }

        if (config->rst_gpio_num != GPIO_NUM_NC)
            log_w("RST pin defined but is not available on the XPT2046");

        *handle = th;

        return ESP_OK;
    }

    esp_err_t esp_lcd_touch_xpt2046_read_battery_level(const esp_lcd_touch_handle_t th, float *outhut)
    {
        log_v("esp_lcd_touch_xpt2046_read_battery_level. th:0x%08x, outhut:0x%08x", th, outhut);

        assert(th != NULL);
        assert(outhut != NULL);

        esp_err_t res;
        uint16_t level;
        // Read Y position and convert returned data to 12bit value
        if ((res = xpt2046_read_register(th, XPT2046_START_BAT_CONVERSION, &level)) != ESP_OK)
        {
            log_w("Could not read battery level");
            return res;
        }

        // battery voltage is reported as 1/4 the actual voltage due to logic in the chip, then
        // adjust for internal vref of 2.5v and finally
        // adjust for ADC bit count
        *outhut = level * 4 * 2.5f / XPT2046_ADC_LIMIT;

        return ESP_OK;
    }

#ifdef __cplusplus
}
#endif

#endif // TOUCH_XPT2046_SPI