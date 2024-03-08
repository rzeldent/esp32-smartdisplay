#ifdef TOUCH_GT911_I2C

#include <esp_touch_gt911.h>
#include <string.h>
#include <esp_rom_gpio.h>
#include <esp32-hal-log.h>

// Registers
const uint16_t GT911_KEYS_REG = 0x8093;
const uint16_t GT911_BUFFER_STATUS_REG = 0x814E;
const uint16_t GT911_TOUCH_POINTS_REG = 0x814F;
const uint16_t GT911_CONFIG_REG = 0x8047;
const uint16_t GT911_PRODUCT_ID_REG = 0x8140;
const uint16_t GT911_CONTROL_REG = 0x8040;

const char productId[] = "911";

// Limits of points / buttons
#define GT911_KEYS_MAX 4
#define GT911_TOUCH_POINTS_MAX 5

#if (CONFIG_ESP_LCD_TOUCH_MAX_BUTTONS > GT911_TOUCH_MAX_BUTTONS)
#error more buttons than available
#endif

typedef struct __attribute__((packed))
{
    uint16_t x;
    uint16_t y;
} gt911_point;

typedef struct __attribute__((packed))
{
    char productId[4];      // 0x8140 - 0x8143
    uint16_t fwId;          // 0x8144 - 0x8145
    gt911_point resolution; // 0x8146 - 0x8147 (x), 0x8148 - 0x8149 (y)
    uint8_t vendorId;       // 0x814A
} gt911_info;

typedef struct
{
    uint8_t number_points : 4; // Bit 0-3 (0x0F): Number of touch points
    uint8_t have_key : 1;      // Bit 4 (0x10): Have touch key
    uint8_t unused : 1;        // Bit 5
    uint8_t large_detect : 1;  // Bit 6 (0x40): Large-area touch on touch pad
    uint8_t buffer_status : 1; // Bit 7 (0x80): Coordinate (or key) is ready for host to read
} buffer_status_flags;

// Touch events
enum gt911_touch_event
{
    none = 0,
    down = 1,
    up = 2,
    slide = 3,
    slide_end = 4
};

typedef struct __attribute__((packed))
{
    // 0x814F-0x8156, ... 0x8176 (5 points)
    uint8_t event;
    gt911_point point;
    uint16_t area;
    uint8_t reserved;
} gt911_touch_event;

typedef struct __attribute__((packed))
{
    union esp_lcd_touch_gt911
    {
        uint8_t keys[GT911_KEYS_MAX];
        gt911_touch_event touch_points[GT911_TOUCH_POINTS_MAX];
    } data;
} gt911_key_touch_data;

gt911_point gt911_resolution;

esp_err_t gt911_reset(esp_lcd_touch_handle_t th)
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

// This function is called if the coordinates do not match the returned coordinates. This is the case for display having another form factor, e.g. 472x320
void gt911_process_coordinates(esp_lcd_touch_handle_t th, uint16_t *x, uint16_t *y, uint16_t *strength, uint8_t *point_num, uint8_t max_point_num)
{
    log_v("th:0x%08x, x:0x%08x, y:0x%08x, strength:0x%08x, point_num:0x%08x, max_point_num:%d", th, x, y, strength, point_num, max_point_num);

    portENTER_CRITICAL(&th->data.lock);
    uint8_t points_available = *point_num > max_point_num ? max_point_num : *point_num;
    for (uint8_t i = 0; i < points_available; i++)
    {
        // Correct the points for the info obtained from the GT911 and configured resolution
        x[i] = (x[i] * th->config.x_max) / gt911_resolution.x;
        y[i] = (y[i] * th->config.y_max) / gt911_resolution.y;
        log_d("Processed coordinates: (%d,%d), area:%d", x[i], y[i], strength[i]);
    }

    portEXIT_CRITICAL(&th->data.lock);
}

esp_err_t gt911_read_info(esp_lcd_touch_handle_t th)
{
    log_v("th:0x%08x", th);
    if (th == NULL)
        return ESP_ERR_INVALID_ARG;

    esp_err_t res;
    gt911_info info;
    if ((res = esp_lcd_panel_io_rx_param(th->io, GT911_PRODUCT_ID_REG, &info, sizeof(info))) != ESP_OK)
    {
        log_e("Unable to read GT911_PRODUCT_ID_REG");
        return res;
    }

    if (strcmp((char *)&info.productId, productId) != 0)
    {
        log_e("GT911 chip not found");
        return ESP_FAIL;
    }

    log_d("GT911 productId: %s", info.productId);                                          // 0x8140 - 0x8143
    log_d("GT911 fwId: 0x%04x", info.fwId);                                                // 0x8144 - 0x8145
    log_d("GT911 xResolution/yResolution: (%d,%d)", info.resolution.x, info.resolution.y); // 0x8146 - 0x8147 // 0x8148 - 0x8149
    log_d("GT911 vendorId: 0x%02x", info.vendorId);                                        // 0x814A

    // Save resolution for processing to scale touch to the display
    gt911_resolution = info.resolution;
    if (info.resolution.x > 0 && info.resolution.y > 0 && (info.resolution.x != th->config.x_max || info.resolution.y != th->config.y_max))
    {
        log_w("Resolution obtained from GT911 (%d,%d) does not match resolution (%d,%d). Enabled coordinate adjustment.", info.resolution.x, info.resolution.y, th->config.x_max, th->config.y_max);
        th->config.process_coordinates = gt911_process_coordinates;
    }

    return ESP_OK;
}

esp_err_t gt911_enter_sleep(esp_lcd_touch_handle_t th)
{
    log_v("th:0x%08x", th);
    if (th == NULL)
        return ESP_ERR_INVALID_ARG;

    esp_err_t res;
    const uint8_t data[] = {0x05}; // Sleep
    if ((res = esp_lcd_panel_io_tx_param(th->io, GT911_CONTROL_REG, data, sizeof(data))) != ESP_OK)
        log_e("Unable to write GT911_CONTROL_REG");

    return res;
}

esp_err_t gt911_exit_sleep(esp_lcd_touch_handle_t th)
{
    log_v("th:0x%08x", th);
    if (th == NULL)
        return ESP_ERR_INVALID_ARG;

    esp_err_t res;
    if (th->config.int_gpio_num == GPIO_NUM_NC)
    {
        log_w("No INT pin defined");
        return ESP_OK;
    }

    gpio_config_t cfg = {
        .pin_bit_mask = BIT64(th->config.int_gpio_num),
        .mode = GPIO_MODE_OUTPUT};
    if ((res = gpio_config(&cfg)) != ESP_OK)
    {
        log_e("Setting INT pin high failed");
        return res;
    }

    gpio_set_level(th->config.int_gpio_num, 1);
    vTaskDelay(pdMS_TO_TICKS(5));
    cfg.mode = GPIO_MODE_OUTPUT_OD;
    if ((res = gpio_config(&cfg)) != ESP_OK)
    {
        log_e("Setting INT pin float failed");
        return res;
    }

    return ESP_OK;
}

esp_err_t gt911_read_data(esp_lcd_touch_handle_t th)
{
    log_v("th:0x%08x", th);
    if (th == NULL)
        return ESP_ERR_INVALID_ARG;

    esp_err_t res;
    buffer_status_flags flags;
    gt911_key_touch_data buffer;

    // Read only the XY register
    if ((res = esp_lcd_panel_io_rx_param(th->io, GT911_BUFFER_STATUS_REG, &flags, sizeof(flags))) != ESP_OK)
    {
        log_e("Unable to read GT911_BUFFER_STATUS_REG");
        return res;
    }

    if (flags.buffer_status)
    {
#if (CONFIG_ESP_LCD_TOUCH_MAX_BUTTONS > 0)
        if (flags.have_key)
        {
            log_v("Buttons available");
            if ((res = esp_lcd_panel_io_rx_param(th->io, GT911_KEYS_REG, &buffer.data.keys, CONFIG_ESP_LCD_TOUCH_MAX_BUTTONS)) != ESP_OK)
            {
                log_e("Unable to read GT911_KEYS_REG");
                return res;
            }

            portENTER_CRITICAL(&th->data.lock);
            th->data.buttons = CONFIG_ESP_LCD_TOUCH_MAX_BUTTONS;
            for (uint8_t i = 0; i < CONFIG_ESP_LCD_TOUCH_MAX_BUTTONS; i++)
                th->data.button[i].status = buffer.data.keys[i];

            portEXIT_CRITICAL(&th->data.lock);
        }
#endif
        //  Check if data is present
        if (flags.number_points > 0)
        {
            log_v("Points available: %d", flags.number_points);
            // Read the number of touch points
            if (flags.number_points <= GT911_TOUCH_POINTS_MAX)
            {
                // Read the points
                if ((res = esp_lcd_panel_io_rx_param(th->io, GT911_TOUCH_POINTS_REG, &buffer.data.touch_points, flags.number_points * sizeof(gt911_touch_event))) != ESP_OK)
                {
                    log_e("Unable to read GT911_TOUCH_POINTS_REG");
                    return res;
                }

                portENTER_CRITICAL(&th->data.lock);
                for (uint8_t i = 0; i < flags.number_points; i++)
                {
                    log_d("Point: #%d, event:%d, point:(%d,%d), area:%d", i, buffer.data.touch_points[i].event, buffer.data.touch_points[i].point.x, buffer.data.touch_points[i].point.y, buffer.data.touch_points[i].area);
                    th->data.coords[i].x = buffer.data.touch_points[i].point.x;
                    th->data.coords[i].y = buffer.data.touch_points[i].point.y;
                    th->data.coords[i].strength = buffer.data.touch_points[i].area;
                }

                th->data.points = flags.number_points;
                portEXIT_CRITICAL(&th->data.lock);
            }
        }
    }

    uint8_t clear[] = {0};
    if ((res = esp_lcd_panel_io_tx_param(th->io, GT911_BUFFER_STATUS_REG, clear, sizeof(clear))) != ESP_OK)
    {
        log_e("Unable to write GT911_BUFFER_STATUS_REG");
        return res;
    }

    return ESP_OK;
}

bool gt911_get_xy(esp_lcd_touch_handle_t th, uint16_t *x, uint16_t *y, uint16_t *strength, uint8_t *point_num, uint8_t max_point_num)
{
    log_v("th:0x%08x, x:0x%08x, y:0x%08x, strength:0x%08x, point_num:0x%08x, max_point_num:%d", th, x, y, strength, point_num, max_point_num);
    if (th == NULL || x == NULL || y == NULL || point_num == NULL)
        return ESP_ERR_INVALID_ARG;

    portENTER_CRITICAL(&th->data.lock);
    *point_num = th->data.points > max_point_num ? max_point_num : th->data.points;
    for (uint8_t i = 0; i < *point_num; i++)
    {
        x[i] = th->data.coords[i].x;
        y[i] = th->data.coords[i].y;
        if (strength != NULL)
            strength[i] = th->data.coords[i].strength;

        log_d("Touch data: x:%d, y:%d, area:%d", x[i], y[i], strength != NULL ? strength[i] : 0);
    }

    th->data.points = 0;
    portEXIT_CRITICAL(&th->data.lock);

    return *point_num > 0;
}

#if (CONFIG_ESP_LCD_TOUCH_MAX_BUTTONS > 0)
esp_err_t gt911_get_button_state(esp_lcd_touch_handle_t th, uint8_t n, uint8_t *state)
{
    log_v("th:0x%08x, n:%d, state:0x%08x", th, n, state);
    if (th == NULL)
        return ESP_ERR_INVALID_ARG;

    if (n > th->data.buttons)
    {
        log_e("Button out of range");
        return ESP_ERR_INVALID_ARG;
    }

    portENTER_CRITICAL(&th->data.lock);
    *state = th->data.button[n].status;
    portEXIT_CRITICAL(&th->data.lock);

    return ESP_OK;
}
#endif

esp_err_t gt911_del(esp_lcd_touch_handle_t th)
{
    log_v("th:0x%08x", th);
    if (th == NULL)
        return ESP_ERR_INVALID_ARG;

    portENTER_CRITICAL(&th->data.lock);
    // Remove gt911_info
    if (th->config.user_data != NULL)
        free(th->config.user_data);

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

esp_err_t esp_lcd_touch_new_i2c_gt911(const esp_lcd_panel_io_handle_t io, const esp_lcd_touch_config_t *config, esp_lcd_touch_handle_t *handle)
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
    th->enter_sleep = gt911_enter_sleep;
    th->exit_sleep = gt911_exit_sleep;
    th->read_data = gt911_read_data;
    th->get_xy = gt911_get_xy;
#if (CONFIG_ESP_LCD_TOUCH_MAX_BUTTONS > 0)
    th->get_button_state = gt911_get_button_state;
#endif
    th->del = gt911_del;
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
    }

    // Reset controller
    if ((res = gt911_reset(th)) != ESP_OK)
    {
        log_e("GT911 reset failed");
        gt911_del(th);
        return res;
    }

    // Read type and resolution
    if ((res = gt911_read_info(th)) != ESP_OK)
    {
        log_e("GT911 read info failed");
        gt911_del(th);
        return res;
    }

    log_d("handle: 0x%08x", th);
    *handle = th;

    return ESP_OK;
}

#endif // TOUCH_GT911_I2C