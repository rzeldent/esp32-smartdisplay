#include <esp_lcd_touch_gt911.h>
#include <string.h>
#include <esp_rom_gpio.h>
#include <esp32-hal-log.h>

// Registers
#define ESP_LCD_TOUCH_GT911_READ_KEY_REG 0x8093
#define ESP_LCD_TOUCH_GT911_READ_XY_REG 0x814E
#define ESP_LCD_TOUCH_GT911_READ_XY_REG_POINTS 0x814F
#define ESP_LCD_TOUCH_GT911_CONFIG_REG 0x8047
#define ESP_LCD_TOUCH_GT911_PRODUCT_ID_REG 0x8140
#define ESP_LCD_TOUCH_GT911_CONTROL_REG 0x8040

// Limits of points / buttons
#define ESP_LCD_TOUCH_GT911_MAX_BUTTONS 4
#define ESP_LCD_TOUCH_GT911_TOUCH_MAX_POINTS 5

#if (CONFIG_ESP_LCD_TOUCH_MAX_BUTTONS > ESP_LCD_TOUCH_GT911_TOUCH_MAX_BUTTONS)
#error more buttons than available
#endif

// Touch events
#define ESP_LCD_TOUCH_GT911_TOUCH_EVENT_NONE 0
#define ESP_LCD_TOUCH_GT911_TOUCH_EVENT_DOWN 1
#define ESP_LCD_TOUCH_GT911_TOUCH_EVENT_UP 2
#define ESP_LCD_TOUCH_GT911_TOUCH_EVENT_SLIDE 3
#define ESP_LCD_TOUCH_GT911_TOUCH_EVENT_SLIDE_END 4

// Two I2C Addresses possible
#define GT911_IO_I2C_CONFIG_DEV_ADDRESS_5D 0x5D // 0x5D (0xBA/0xBB): INT High during reset or
#define GT911_IO_I2C_CONFIG_DEV_ADDRESS_14 0x14 // 0x14 (0x28/0x29): INT Low during reset

#define FLAGS_BUFFER_STATUS 0x80    // coordinate (or key) is ready for host to read
#define FLAGS_LARGE_DETECT 0x40     // large-area touch on TP
#define FLAGS_HAVE_KEY 0x10         //  Have touch key
#define FLAGS_NUM_TOUCH_POINTS 0x0F // Number of touch points

typedef struct __attribute__((packed))
{
    char productId[4];    // 0x8140 - 0x8143
    uint16_t fwId;        // 0x8144 - 0x8145
    uint16_t xResolution; // 0x8146 - 0x8147
    uint16_t yResolution; // 0x8148 - 0x8149
    uint8_t vendorId;     // 0x814A
} GTInfo;

typedef struct __attribute__((packed))
{
    // 0x814F-0x8156, ... 0x8176 (5 points)
    uint8_t event;
    uint16_t x;
    uint16_t y;
    uint16_t area;
    uint8_t reserved;
} GTPoint;

typedef struct __attribute__((packed))
{
    uint8_t flags;
    union esp_lcd_touch_gt911
    {
        uint8_t buttons[ESP_LCD_TOUCH_GT911_MAX_BUTTONS];
        GTPoint points[ESP_LCD_TOUCH_GT911_TOUCH_MAX_POINTS];
    } data;
} GTTouchData;

#ifdef __cplusplus
extern "C"
{
#endif

    esp_err_t gt911_set_swap_xy(esp_lcd_touch_handle_t tp, bool swap)
    {
        log_v("gt911_set_swap_xy. tp:%08x, swap:%d", tp, swap);
        if (tp == NULL)
        {
            log_e("tp can not be null");
            return ESP_ERR_INVALID_ARG;
        }

        tp->config.flags.swap_xy = swap;
        return ESP_OK;
    }

    esp_err_t gt911_get_swap_xy(esp_lcd_touch_handle_t tp, bool *swap)
    {
        log_v("gt911_get_swap_xy. tp:%08x", tp);
        if (tp == NULL || swap == NULL)
        {
            log_e("tp or swap can not be null");
            return ESP_ERR_INVALID_ARG;
        }

        *swap = tp->config.flags.swap_xy;
        return ESP_OK;
    }

    esp_err_t gt911_set_mirror_x(esp_lcd_touch_handle_t tp, bool mirror)
    {
        log_v("gt911_set_mirror_x. tp:%08x, mirror:%d", tp, mirror);
        if (tp == NULL)
        {
            log_e("tp can not be null");
            return ESP_ERR_INVALID_ARG;
        }

        tp->config.flags.mirror_x = mirror;
        return ESP_OK;
    }

    esp_err_t gt911_get_mirror_x(esp_lcd_touch_handle_t tp, bool *mirror)
    {
        log_v("gt911_get_mirror_x. tp:%08x", tp);
        if (tp == NULL || mirror == NULL)
        {
            log_e("tp or mirror can not be null");
            return ESP_ERR_INVALID_ARG;
        }

        *mirror = tp->config.flags.mirror_x;
        return ESP_OK;
    }

    esp_err_t gt911_set_mirror_y(esp_lcd_touch_handle_t tp, bool mirror)
    {
        log_v("gt911_set_mirror_y. tp:%08x, mirror:%d", tp, mirror);
        if (tp == NULL)
        {
            log_e("tp can not be null");
            return ESP_ERR_INVALID_ARG;
        }
        tp->config.flags.mirror_y = mirror;
        return ESP_OK;
    }

    esp_err_t gt911_get_mirror_y(esp_lcd_touch_handle_t tp, bool *mirror)
    {
        log_v("gt911_get_mirror_y. tp:%08x", tp);
        if (tp == NULL || mirror == NULL)
        {
            log_e("tp or mirror can not be null");
            return ESP_ERR_INVALID_ARG;
        }

        *mirror = tp->config.flags.mirror_y;
        return ESP_OK;
    }

    esp_err_t gt911_reset(esp_lcd_touch_handle_t tp)
    {
        assert(tp != NULL);
        esp_err_t res;

        // Set RST active
        if ((res = gpio_set_level(tp->config.rst_gpio_num, tp->config.levels.reset)) != ESP_OK)
        {
            log_e("Setting RST failed");
            return res;
        }

        // Wait at least 100us
        vTaskDelay(pdMS_TO_TICKS(1));

        // Set RST high
        if ((res = gpio_set_level(tp->config.rst_gpio_num, !tp->config.levels.reset)) != ESP_OK)
        {
            log_e("Resetting RST failed");
            return res;
        }

        // Wait at least 5ms
        vTaskDelay(pdMS_TO_TICKS(5));

        vTaskDelay(pdMS_TO_TICKS(50));

        return ESP_OK;
    }

    esp_err_t gt911_read_info(esp_lcd_touch_handle_t tp)
    {
        assert(tp != NULL);
        esp_err_t res;

        // Info is stored in the user_data
        if (tp->config.user_data != NULL)
        {
            free(tp->config.user_data);
            tp->config.user_data = NULL;
        }

        GTInfo *info = heap_caps_calloc(1, sizeof(GTInfo), MALLOC_CAP_DEFAULT);
        if (info == NULL)
        {
            log_e("No memory available for GTInfo");
            return ESP_ERR_NO_MEM;
        }

        if ((res = esp_lcd_panel_io_rx_param(tp->io, ESP_LCD_TOUCH_GT911_PRODUCT_ID_REG, info, sizeof(GTInfo))) != ESP_OK)
        {
            free(info);
            log_e("Unable to read GT911 Info");
            return res;
        }

        if (strcmp((char *)&info->productId, "911") != 0)
        {
            log_e("GT911 chip not found");
            return ESP_FAIL;
        }

        if (info->xResolution == 0 || info->yResolution == 0)
        {
            log_e("Invalid resolution in GT911");
            return ESP_FAIL;
        }

        log_d("GT911 productId: %s", info->productId);                                          // 0x8140 - 0x8143
        log_d("GT911 fwId: %04x", info->fwId);                                                  // 0x8144 - 0x8145
        log_d("GT911 xResolution/yResolution: (%d, %d)", info->xResolution, info->yResolution); // 0x8146 - 0x8147 // 0x8148 - 0x8149
        log_d("GT911 vendorId: %02x", info->vendorId);                                          // 0x814A

        tp->config.user_data = info;

        return ESP_OK;
    }

    esp_err_t gt911_enter_sleep(esp_lcd_touch_handle_t tp)
    {
        log_v("gt911_enter_sleep. tp:%08x", tp);
        if (tp == NULL)
        {
            log_e("tp can not be null");
            return ESP_ERR_INVALID_ARG;
        }

        esp_err_t res;
        const uint8_t data[] = {0x05}; // Sleep
        if ((res = esp_lcd_panel_io_tx_param(tp->io, ESP_LCD_TOUCH_GT911_CONTROL_REG, data, sizeof(data))) != ESP_OK)
            log_e("Unable to write GT911_ENTER_SLEEP");

        return res;
    }

    esp_err_t gt911_exit_sleep(esp_lcd_touch_handle_t tp)
    {
        log_v("gt911_exit_sleep. tp:%08x", tp);
        if (tp == NULL)
        {
            log_e("tp can not be null");
            return ESP_ERR_INVALID_ARG;
        }

        esp_err_t res;
        if (tp->config.int_gpio_num == GPIO_NUM_NC)
        {
            log_w("No INT pin defined");
            return ESP_OK;
        }

        gpio_config_t cfg = {
            .pin_bit_mask = BIT64(tp->config.int_gpio_num),
            .mode = GPIO_MODE_OUTPUT};
        if ((res = gpio_config(&cfg)) != ESP_OK)
        {
            log_e("Setting INT pin high failed");
            return res;
        }

        gpio_set_level(tp->config.int_gpio_num, 1);
        vTaskDelay(pdMS_TO_TICKS(5));
        cfg.mode = GPIO_MODE_OUTPUT_OD;
        if ((res = gpio_config(&cfg)) != ESP_OK)
        {
            log_e("Setting INT pin float failed");
            return res;
        }

        return ESP_OK;
    }

    esp_err_t gt911_read_data(esp_lcd_touch_handle_t tp)
    {
        log_v("gt911_read_data. tp:%08x", tp);
        esp_err_t res;
        GTTouchData buffer;
        uint8_t i;
        if (tp == NULL)
        {
            log_e("tp can not be null");
            return ESP_ERR_INVALID_ARG;
        }

        // Read only the XY register
        if ((res = esp_lcd_panel_io_rx_param(tp->io, ESP_LCD_TOUCH_GT911_READ_XY_REG, &buffer.flags, sizeof(buffer.flags))) != ESP_OK)
        {
            log_e("Unable to read GT911_READ_XY_REG");
            return res;
        }

        if ((buffer.flags & FLAGS_BUFFER_STATUS) > 0)
        {
#if (CONFIG_ESP_LCD_TOUCH_MAX_BUTTONS > 0)
            if ((buffer.flags & FLAGS_HAVE_KEY) > 0)
            {
                log_v("buttons available");
                if ((res = esp_lcd_panel_io_rx_param(tp->io, ESP_LCD_TOUCH_GT911_READ_KEY_REG, &buffer.data.buttons, CONFIG_ESP_LCD_TOUCH_MAX_BUTTONS)) != ESP_OK)
                {
                    log_e("Unable to read ESP_LCD_TOUCH_GT911_READ_KEY_REG");
                    return res;
                }

                portENTER_CRITICAL(&tp->data.lock);
                tp->data.buttons = CONFIG_ESP_LCD_TOUCH_MAX_BUTTONS;
                for (i = 0; i < CONFIG_ESP_LCD_TOUCH_MAX_BUTTONS; i++)
                    tp->data.button[i].status = buffer.data.buttons[i];

                portEXIT_CRITICAL(&tp->data.lock);
            }
#endif
            uint8_t points_available = buffer.flags & FLAGS_NUM_TOUCH_POINTS;
            //  Check if data is present
            if (points_available > 0)
            {
                log_v("points_available: %d", points_available);
                // Read the number of touches
                if (points_available <= ESP_LCD_TOUCH_GT911_TOUCH_MAX_POINTS)
                {
                    // Read the points
                    if ((res = esp_lcd_panel_io_rx_param(tp->io, ESP_LCD_TOUCH_GT911_READ_XY_REG_POINTS, &buffer.data.points, points_available * sizeof(GTPoint))) != ESP_OK)
                    {
                        log_e("Unable to read ESP_LCD_TOUCH_GT911_READ_XY_REG_POINTS");
                        return res;
                    }

                    log_v("event:%d, x:%d, y:%d, area:%d", buffer.data.points[0].event, buffer.data.points[0].x, buffer.data.points[0].y, buffer.data.points[0].area);

                    portENTER_CRITICAL(&tp->data.lock);
                    GTInfo *info = tp->config.user_data;
                    assert(info);
                    tp->data.points = points_available;
                    for (i = 0; i < points_available; i++)
                    {
                        // Correct the points for the info returned from the GT911 and configured resolution
                        tp->data.coords[i].x = (double)buffer.data.points[i].x * tp->config.x_max / info->xResolution;
                        tp->data.coords[i].y = (double)buffer.data.points[i].y * tp->config.y_max / info->yResolution;
                        tp->data.coords[i].strength = buffer.data.points[i].area;
                    }

                    portEXIT_CRITICAL(&tp->data.lock);
                }
            }
        }

        uint8_t clear[] = {0};
        if ((res = esp_lcd_panel_io_tx_param(tp->io, ESP_LCD_TOUCH_GT911_READ_XY_REG, clear, sizeof(clear))) != ESP_OK)
        {
            log_e("Unable to write T911_READ_XY_REG");
            return res;
        }

        return ESP_OK;
    }

    bool gt911_get_xy(esp_lcd_touch_handle_t tp, uint16_t *x, uint16_t *y, uint16_t *strength, uint8_t *point_num, uint8_t max_point_num)
    {
        log_v("gt911_get_xy. tp:%08x, x:0x%08x, y:0x%08x, strength:0x%08x, point_num:0x%08x, max_point_num:%d", tp, x, y, strength, point_num, max_point_num);
        if (tp == NULL || x == NULL || y == NULL || point_num == NULL)
        {
            log_e("tp, x, y or point_num can not be null");
            return ESP_ERR_INVALID_ARG;
        }

        portENTER_CRITICAL(&tp->data.lock);
        *point_num = tp->data.points > max_point_num ? max_point_num : tp->data.points;
        for (uint8_t i = 0; i < *point_num; i++)
        {
            if (tp->config.flags.swap_xy)
            {
                x[i] = tp->config.flags.mirror_y ? tp->config.y_max - tp->data.coords[i].y : tp->data.coords[i].y;
                y[i] = tp->config.flags.mirror_x ? tp->config.x_max - tp->data.coords[i].x : tp->data.coords[i].x;
            }
            else
            {
                x[i] = tp->config.flags.mirror_x ? tp->config.x_max - tp->data.coords[i].x : tp->data.coords[i].x;
                y[i] = tp->config.flags.mirror_y ? tp->config.y_max - tp->data.coords[i].y : tp->data.coords[i].y;
            }

            if (strength != NULL)
                strength[i] = tp->data.coords[i].strength;
        }

        tp->data.points = 0;
        portEXIT_CRITICAL(&tp->data.lock);

        return *point_num > 0;
    }

#if (CONFIG_ESP_LCD_TOUCH_MAX_BUTTONS > 0)
    esp_err_t gt911_get_button_state(esp_lcd_touch_handle_t tp, uint8_t n, uint8_t *state)
    {
        log_v("gt911_get_xy. tp:%08x, n:%d, state:0x%08x", tp, n, state);
        if (tp == NULL || state == NULL)
        {
            log_e("tp or state can not be null");
            return ESP_ERR_INVALID_ARG;
        }

        if (n > tp->data.buttons)
        {
            log_e("Button out of range");
            return ESP_ERR_INVALID_ARG;
        }

        portENTER_CRITICAL(&tp->data.lock);
        *state = tp->data.button[n].status;
        portEXIT_CRITICAL(&tp->data.lock);

        return ESP_OK;
    }
#endif

    esp_err_t gt911_del(esp_lcd_touch_handle_t tp)
    {
        log_v("gt911_del. tp:%08x", tp);
        if (tp != NULL)
        {
            portENTER_CRITICAL(&tp->data.lock);
            // Remove GTInfo
            if (tp->config.user_data != NULL)
                free(tp->config.user_data);

            // Remove interrupts and reset INT
            if (tp->config.int_gpio_num != GPIO_NUM_NC)
            {
                if (tp->config.interrupt_callback)
                    gpio_isr_handler_remove(tp->config.int_gpio_num);

                gpio_reset_pin(tp->config.int_gpio_num);
            }
            // Reset RST
            if (tp->config.rst_gpio_num != GPIO_NUM_NC)
                gpio_reset_pin(tp->config.rst_gpio_num);

            free(tp);
        }

        return ESP_OK;
    }

    esp_err_t esp_lcd_touch_new_i2c_gt911(const esp_lcd_panel_io_handle_t io, const esp_lcd_touch_config_t *config, esp_lcd_touch_handle_t *handle)
    {
        log_v("esp_lcd_touch_new_spi_gt911. io:%08x, config:%08x, handle:%08x", io, config, handle);
        if (io == NULL || config == NULL || handle == NULL)
        {
            log_e("io, config or handle can not be null");
            return ESP_ERR_INVALID_ARG;
        }

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
        const esp_lcd_touch_handle_t tp = heap_caps_aligned_alloc(1, sizeof(esp_lcd_touch_t), MALLOC_CAP_DEFAULT);
        if (tp == NULL)
        {
            log_e("No memory available for esp_lcd_touch_t");
            return ESP_ERR_NO_MEM;
        }

        tp->io = io;
        tp->enter_sleep = gt911_enter_sleep;
        tp->exit_sleep = gt911_exit_sleep;
        tp->read_data = gt911_read_data;
        tp->set_swap_xy = gt911_set_swap_xy;
        tp->get_swap_xy = gt911_get_swap_xy;
        tp->set_mirror_x = gt911_set_mirror_x;
        tp->get_mirror_x = gt911_get_mirror_x;
        tp->set_mirror_y = gt911_set_mirror_y;
        tp->get_mirror_y = gt911_get_mirror_y;
        tp->get_xy = gt911_get_xy;
#if (CONFIG_ESP_LCD_TOUCH_MAX_BUTTONS > 0)
        tp->get_button_state = gt911_get_button_state;
#endif
        tp->del = gt911_del;
        memcpy(&tp->config, config, sizeof(esp_lcd_touch_config_t));
        portMUX_INITIALIZE(&tp->data.lock);

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
                free(tp);
                log_e("Configuring GPIO for INT failed");
                return res;
            }

            if (config->interrupt_callback != NULL)
            {
                if ((res = esp_lcd_touch_register_interrupt_callback(tp, config->interrupt_callback)) != ESP_OK)
                {
                    gpio_reset_pin(tp->config.int_gpio_num);
                    free(tp);
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
                    if (tp->config.int_gpio_num != GPIO_NUM_NC)
                    {
                        if (config->interrupt_callback != NULL)
                            gpio_isr_handler_remove(tp->config.int_gpio_num);

                        gpio_reset_pin(tp->config.int_gpio_num);
                    }

                    free(tp);
                    log_e("Configuring or setting GPIO for RST failed");
                    return res;
                }
            }

            // Reset controller
            if ((res = gt911_reset(tp)) != ESP_OK)
            {
                log_e("GT911 reset failed");
                gt911_del(tp);
                return res;
            }

            // Read type and resolution
            if ((res = gt911_read_info(tp)) != ESP_OK)
            {
                log_e("GT911 read info failed");
                gt911_del(tp);
                return res;
            }
        }

        *handle = tp;

        return ESP_OK;
    }

#ifdef __cplusplus
}
#endif