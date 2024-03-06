#ifdef TOUCH_CST816S_I2C

#include <esp_lcd_touch_cst816s.h>
#include <string.h>
#include <esp_rom_gpio.h>
#include <esp32-hal-log.h>

// Registers
const uint8_t CST816S_GESTURE_REG = 0x01;
const uint8_t CST816S_FINGERNUM_REG = 0x02;
const uint8_t CST816S_XPOSH_REG = 0x03;
const uint8_t CST816S_XPOSL_REG = 0x04;
const uint8_t CST816S_YPOSH_REG = 0x05;
const uint8_t CST816S_YPOSL_REG = 0x06;

const uint8_t CST816S_BC0H_REG = 0xB0;
const uint8_t CST816S_BC0L_REG = 0xB1;
const uint8_t CST816S_BC1H_REG = 0xB2;
const uint8_t CST816S_BC1L_REG = 0xB3;

const uint8_t CST816S_SLEEP_REG = 0xA5;
const uint8_t CST816S_CHIPID_REG = 0xA7;
const uint8_t CST816S_PROJID_REG = 0xA8;
const uint8_t CST816S_FWVERSION_REG = 0xA9;

const uint8_t CST816S_MOTIONMASK_REG = 0xEC;
const uint8_t CST816S_IRQPULSEWIDTH_REG = 0xED;
const uint8_t CST816S_NORSCANPER_REG = 0xEE;
const uint8_t CST816S_MOTIONSIANGLE_REG = 0xEF;
const uint8_t CST816S_LPSCANRAW1H_REG = 0xF0;
const uint8_t CST816S_LPSCANRAW1L_REG = 0xF1;
const uint8_t CST816S_LPSCANRAW2H_REG = 0xF2;
const uint8_t CST816S_LPSCANRAW2L_REG = 0xF3;
const uint8_t CST816S_LPAUTOWAKEUPTIME_REG = 0xF4;
const uint8_t CST816S_LPSCANTH_REG = 0xF5;
const uint8_t CST816S_LPSCANWIN_REG = 0xF6;
const uint8_t CST816S_LPSCANFREQ_REG = 0xF7;
const uint8_t CST816S_LPSCANIDAC_REG = 0xF8;
const uint8_t CST816S_AUTOSLEEPTIME_REG = 0xF9;
const uint8_t CST816S_IRQCTL_REG = 0xFA;
const uint8_t CST816S_AUTORESET_REG = 0xFB;
const uint8_t CST816S_LONGPRESSTIME_REG = 0xFC;
const uint8_t CST816S_IOCTL_REG = 0xFD;
const uint8_t CST816S_AUTOSLEEP_REG = 0xFE;

// Touch events
const uint8_t CST816S_TOUCH_EVENT_NONE = 0x0;
const uint8_t CST816S_TOUCH_EVENT_SLIDE_DOWN = 0x1;
const uint8_t CST816S_TOUCH_EVENT_SLIDE_UP = 0x2;
const uint8_t CST816S_TOUCH_EVENT_SLIDE_LEFT = 0x3;
const uint8_t CST816S_TOUCH_EVENT_SLIDE_RIGHT = 0x4;
const uint8_t CST816S_TOUCH_EVENT_CLICK = 0x5;
const uint8_t CST816S_TOUCH_EVENT_DOUBLE_CLICK = 0xB;
const uint8_t CST816S_TOUCH_EVENT_PRESS = 0xC;

typedef struct __attribute__((packed))
{
    uint8_t id;        // 0xA7
    uint8_t projectId; // 0xA8
    uint8_t fwVersion; // 0xA9
} cst816s_info;

typedef struct __attribute__((packed))
{
    uint16_t x;
    uint16_t y;
} cst816s_point;

typedef struct __attribute__((packed))
{
    uint8_t event;
    uint8_t fingerNum;
    cst816s_point point; // POSH (4 bits) + POSL (8 bits)
} cst816s_touch_event;

#ifdef __cplusplus
extern "C"
{
#endif

    esp_err_t cst816s_reset(esp_lcd_touch_handle_t th)
    {
        log_v("th:0x%08x", th);
        if (th == NULL)
            return ESP_ERR_INVALID_ARG;

        esp_err_t res;
        // Set RST active
        if ((res = gpio_set_level(th->config.rst_gpio_num, th->config.levels.reset)) != ESP_OK)
        {
            log_e("Setting RST failed");
            return res;
        }

        // Wait at least 100us
        vTaskDelay(pdMS_TO_TICKS(1));

        // Set RST high
        if ((res = gpio_set_level(th->config.rst_gpio_num, !th->config.levels.reset)) != ESP_OK)
        {
            log_e("Resetting RST failed");
            return res;
        }

        // Wait at least 50ms
        vTaskDelay(pdMS_TO_TICKS(50));

        return ESP_OK;
    }

    esp_err_t cst816s_read_info(esp_lcd_touch_handle_t th)
    {
        log_v("th:0x%08x", th);
        if (th == NULL)
            return ESP_ERR_INVALID_ARG;

        esp_err_t res;
        cst816s_info info;
        if ((res = esp_lcd_panel_io_rx_param(th->io, CST816S_CHIPID_REG, &info, sizeof(info))) != ESP_OK)
        {
            log_e("Unable to read CST816S info");
            return res;
        }

        log_d("CST816S Id: 0x%02X", info.id);
        log_d("CST816S Project id: %d", info.projectId);
        log_d("CST816S Firmware version: %d", info.fwVersion);

        return ESP_OK;
    }

    esp_err_t cst816s_enter_sleep(esp_lcd_touch_handle_t th)
    {
        log_v("th:0x%08x", th);
        if (th == NULL)
            return ESP_ERR_INVALID_ARG;

        esp_err_t res;
        const uint8_t data[] = {0x03}; // Sleep
        if ((res = esp_lcd_panel_io_tx_param(th->io, CST816S_SLEEP_REG, data, sizeof(data))) != ESP_OK)
            log_e("Unable to write GT911_CONTROL_REG");

        return res;
    }

    esp_err_t cst816s_read_data(esp_lcd_touch_handle_t th)
    {
        log_v("th:0x%08x", th);
        if (th == NULL)
            return ESP_ERR_INVALID_ARG;

        esp_err_t res;
        cst816s_touch_event buffer;

        // Read only the XY register
        if ((res = esp_lcd_panel_io_rx_param(th->io, CST816S_GESTURE_REG, &buffer, sizeof(buffer))) != ESP_OK)
        {
            log_e("Unable to read CST816S point");
            return res;
        }

        portENTER_CRITICAL(&th->data.lock);
        if ((th->data.points = buffer.fingerNum) > 0)
        {
            th->data.coords[0].x = buffer.point.x;
            th->data.coords[0].y = buffer.point.y;
            th->data.coords[0].strength = 0;
        }

        portEXIT_CRITICAL(&th->data.lock);

        return ESP_OK;
    }

    bool cst816s_get_xy(esp_lcd_touch_handle_t th, uint16_t *x, uint16_t *y, uint16_t *strength, uint8_t *point_num, uint8_t max_point_num)
    {
        log_v("th:0x%08x, x:0x%08x, y:0x%08x, strength:0x%08x, point_num:0x%08x, max_point_num:%d", th, x, y, strength, point_num, max_point_num);
        if (th == NULL || x == NULL || y == NULL || point_num == NULL)
            return ESP_ERR_INVALID_ARG;

        portENTER_CRITICAL(&th->data.lock);
        *point_num = th->data.points > max_point_num ? max_point_num : th->data.points;
        for (uint8_t i = 0; i < *point_num; i++)
        {
            x[i] = th->data.coords[i].y;
            y[i] = th->data.coords[i].x;
            if (strength != NULL)
                strength[i] = th->data.coords[i].strength;
        }

        th->data.points = 0;
        portEXIT_CRITICAL(&th->data.lock);

        return *point_num > 0;
    }

    esp_err_t cst816s_del(esp_lcd_touch_handle_t th)
    {
        log_v("th:0x%08x", th);
        if (th == NULL)
            return ESP_ERR_INVALID_ARG;

        portENTER_CRITICAL(&th->data.lock);

        // Remove interrupts and reset INT
        if (th->config.int_gpio_num != GPIO_NUM_NC)
        {
            if (th->config.interrupt_callback)
                gpio_isr_handler_remove(th->config.int_gpio_num);

            gpio_reset_pin(th->config.int_gpio_num);
        }

        // Reset RST
        if (th->config.rst_gpio_num != GPIO_NUM_NC)
            gpio_reset_pin(th->config.rst_gpio_num);

        free(th);

        return ESP_OK;
    }

    esp_err_t esp_lcd_touch_new_i2c_cst816s(const esp_lcd_panel_io_handle_t io, const esp_lcd_touch_config_t *config, esp_lcd_touch_handle_t *handle)
    {
        log_v("io:0x%08x, config:0x%08x, handle:0x%08x", io, config, handle);
        if (io == NULL || config == NULL || handle == NULL)
            return ESP_ERR_INVALID_ARG;

        if (config->int_gpio_num != GPIO_NUM_NC && !GPIO_IS_VALID_GPIO(config->int_gpio_num))
        {
            log_e("Invalid GPIO INT pin: %d", config->int_gpio_num);
            return ESP_ERR_INVALID_ARG;
        }

        if (config->rst_gpio_num != GPIO_NUM_NC && !GPIO_IS_VALID_GPIO(config->rst_gpio_num))
        {
            log_e("Invalid GPIO RST pin: %d", config->rst_gpio_num);
            return ESP_ERR_INVALID_ARG;
        }

        esp_err_t res;
        const esp_lcd_touch_handle_t th = heap_caps_calloc(1, sizeof(esp_lcd_touch_t), MALLOC_CAP_DEFAULT);
        if (th == NULL)
        {
            log_e("No memory available for esp_lcd_touch_t");
            return ESP_ERR_NO_MEM;
        }

        th->io = io;
        th->enter_sleep = cst816s_enter_sleep;
        th->read_data = cst816s_read_data;
        th->get_xy = cst816s_get_xy;
#if (CONFIG_ESP_LCD_TOUCH_MAX_BUTTONS > 0)
        th->get_button_state = cst816s_get_button_state;
#endif
        th->del = cst816s_del;
        memcpy(&th->config, config, sizeof(esp_lcd_touch_config_t));
        portMUX_INITIALIZE(&th->data.lock);

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

            if (config->rst_gpio_num != GPIO_NUM_NC)
            {
                esp_rom_gpio_pad_select_gpio(config->rst_gpio_num);
                const gpio_config_t cfg = {
                    .pin_bit_mask = BIT64(config->rst_gpio_num),
                    .mode = GPIO_MODE_OUTPUT};
                if ((res = gpio_config(&cfg)) != ESP_OK)
                {
                    if (th->config.int_gpio_num != GPIO_NUM_NC)
                    {
                        if (config->interrupt_callback != NULL)
                            gpio_isr_handler_remove(th->config.int_gpio_num);

                        gpio_reset_pin(th->config.int_gpio_num);
                    }

                    free(th);
                    log_e("Configuring or setting GPIO for RST failed");
                    return res;
                }
            }

            // Reset controller
            if ((res = cst816s_reset(th)) != ESP_OK)
            {
                log_e("GT911 reset failed");
                cst816s_del(th);
                return res;
            }

            // Read type and resolution
            if ((res = cst816s_read_info(th)) != ESP_OK)
            {
                log_e("GT911 read info failed");
                cst816s_del(th);
                return res;
            }
        }

        log_d("handle: 0x%08x", th);
        *handle = th;

        return ESP_OK;
    }

#ifdef __cplusplus
}
#endif

#endif // TOUCH_CST816S_I2C