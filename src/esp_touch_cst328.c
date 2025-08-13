// Copied from esp_touch_cst816s.c and modified some values and renamed things and modified cst328_read_data to match the example file for waveshare esp32-s3 2.8's implementation of this touch driver 
// This is not a fully implemented perfect driver for the cst328 and may have some broken functionality
// See:
// Wiki for waveshare esp32-s3 2.8:http://www.waveshare.com/wiki/ESP32-S3-Touch-LCD-2.8
// Demo: https://files.waveshare.com/wiki/ESP32-S3-Touch-LCD-2.8/ESP32-S3-Touch-LCD-2.8-Demo.zip
// File containing cst328 driver in demo: /Arduino/examples/LVGL_Arduino/Touch_CST328.cpp - see function uint8_t Touch_Read_Data(void)

#ifdef TOUCH_CST328_I2C

#include <esp_touch_cst328.h>
#include <string.h>
#include <esp_rom_gpio.h>
#include <esp32-hal-log.h>

// // Registers

#define CST328_GESTURE_REG    (0xD005)
#define ESP_LCD_TOUCH_CST328_READ_XY_REG        (0xD000)

#define CST328_LCD_TOUCH_MAX_POINTS             (5)      


/* these are not updated to 328 yet!!! */
#define CST816S_SLEEP_REG 0xA5
#define CST816S_CHIPID_REG 0xA7



// Touch events


typedef struct __attribute__((packed))
{
    uint8_t id;        // 0xA7
    uint8_t projectId; // 0xA8
    uint8_t fwVersion; // 0xA9
} cst816s_info;



typedef struct
{
  uint8_t points;    // Number of touch points
  struct {
    uint16_t x; /*!< X coordinate */
    uint16_t y; /*!< Y coordinate */
    uint16_t strength; /*!< Strength */
  }coords[CST328_LCD_TOUCH_MAX_POINTS];
 } CST328_Touch;


esp_err_t cst328_reset(esp_lcd_touch_handle_t th)
{
    log_v("th:0x%08x", th);
    if (th == NULL)
        return ESP_ERR_INVALID_ARG;

    if (th->config.rst_gpio_num == GPIO_NUM_NC)
    {
        log_w("No RST pin defined");
        return ESP_OK;
    }

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

esp_err_t cst328_read_info(esp_lcd_touch_handle_t th)
{
    log_v("th:0x%08x", th);
    if (th == NULL)
        return ESP_ERR_INVALID_ARG;

    esp_err_t res;
    cst816s_info info;
    if ((res = esp_lcd_panel_io_rx_param(th->io, CST816S_CHIPID_REG, &info, sizeof(info))) != ESP_OK)
    {
        log_e("Unable to read CST328 info");
        return res;
    }

    log_d("CST328 Id: 0x%02X", info.id);
    log_d("CST328 Project id: %d", info.projectId);
    log_d("CST328 Firmware version: %d", info.fwVersion);

    return ESP_OK;
}

esp_err_t cst328_enter_sleep(esp_lcd_touch_handle_t th)
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

CST328_Touch touch_data = {0};
esp_err_t cst328_read_data(esp_lcd_touch_handle_t th) {
  uint8_t buf[41];
  uint8_t touch_cnt = 0;
  uint8_t clear = 0;
  uint8_t Over = 0xAB;
  size_t i = 0,num=0;
  int res;
  res = esp_lcd_panel_io_rx_param(th->io, CST328_GESTURE_REG, &buf, sizeof(buf));
  if (res != ESP_OK)
    {
        log_e("Unable to read CST328_GESTURE_REG");
        return res;
    }
  if ((buf[0] & 0x0F) == 0x00) {                                              
    // No touch data
    if ((res = esp_lcd_panel_io_tx_param(th->io, CST328_GESTURE_REG, &clear, 1)) != ESP_OK)
        log_e("Unable to write GT911_CONTROL_REG");
    
  } else {
    /* Count of touched points */
    touch_cnt = buf[0] & 0x0F;
    if (touch_cnt > CST328_LCD_TOUCH_MAX_POINTS || touch_cnt == 0) {
      if ((res = esp_lcd_panel_io_tx_param(th->io, CST328_GESTURE_REG, &clear, 1)) != ESP_OK)
        log_e("Unable to write GT911_CONTROL_REG");
      return true;
    }
    /* Read all points */
    res = esp_lcd_panel_io_rx_param(th->io, ESP_LCD_TOUCH_CST328_READ_XY_REG, &buf[1], 27);
    if (res != ESP_OK)
    {
        log_e("Unable to read ESP_LCD_TOUCH_CST328_READ_XY_REG");
        return res;
    }
    /* Clear all */
    if ((res = esp_lcd_panel_io_tx_param(th->io, CST328_GESTURE_REG, &clear, 1)) != ESP_OK)
        log_e("Unable to write GT911_CONTROL_REG");
    // printf(" points=%d \r\n",touch_cnt);
    portENTER_CRITICAL(&th->data.lock);
    /* Number of touched points */
    if(touch_cnt > CST328_LCD_TOUCH_MAX_POINTS)
        touch_cnt = CST328_LCD_TOUCH_MAX_POINTS;
    touch_data.points = (uint8_t)touch_cnt;
    //log_i("points: %d", touch_data.points);
    th->data.points = touch_data.points;
    /* Fill all coordinates */
    for (i = 0; i < touch_cnt; i++) {
      if(i>0) num = 2;
      touch_data.coords[i].x = (uint16_t)(((uint16_t)buf[(i * 5) + 2 + num] << 4) + ((buf[(i * 5) + 4 + num] & 0xF0)>> 4));               
      touch_data.coords[i].y = (uint16_t)(((uint16_t)buf[(i * 5) + 3 + num] << 4) + ( buf[(i * 5) + 4 + num] & 0x0F));
      touch_data.coords[i].strength = ((uint16_t)buf[(i * 5) + 5 + num]);

      th->data.coords[i].x = touch_data.coords[i].x;
      th->data.coords[i].y = touch_data.coords[i].y;
      th->data.coords[i].strength = touch_data.coords[i].strength;

      log_i("x:%d, y:%d, strength:%d", th->data.coords[0].x, th->data.coords[0].y, th->data.coords[0].strength);
      
    }
    portEXIT_CRITICAL(&th->data.lock);
    // printf(" points=%d \r\n",touch_data.points);
  }
    return ESP_OK;
}

bool cst328_get_xy(esp_lcd_touch_handle_t th, uint16_t *x, uint16_t *y, uint16_t *strength, uint8_t *point_num, uint8_t max_point_num)
{
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
    }

    th->data.points = 0;
    portEXIT_CRITICAL(&th->data.lock);

    return *point_num > 0;
}

esp_err_t cst328_del(esp_lcd_touch_handle_t th)
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

esp_err_t esp_lcd_touch_new_i2c_cst328(const esp_lcd_panel_io_handle_t io, const esp_lcd_touch_config_t *config, esp_lcd_touch_handle_t *handle)
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
    th->enter_sleep = cst328_enter_sleep;
    th->read_data = cst328_read_data;
    th->get_xy = cst328_get_xy;
#if (CONFIG_ESP_LCD_TOUCH_MAX_BUTTONS > 0)
    th->get_button_state = cst816s_get_button_state; // TODO: whoops may have accidentally deleted this. Definitely will need to re-add in the future
#endif
    th->del = cst328_del;
    memcpy(&th->config, config, sizeof(esp_lcd_touch_config_t));
    portMUX_INITIALIZE(&th->data.lock);

    // Reset controller
    if ((res = cst328_reset(th)) != ESP_OK)
    {
        log_e("GT911 reset failed");
        cst328_del(th);
        return res;
    }

    // Read type and resolution
    if ((res = cst328_read_info(th)) != ESP_OK)
    {
        log_e("GT911 read info failed");
        cst328_del(th);
        return res;
    }

    if (config->int_gpio_num != GPIO_NUM_NC)
    {
        esp_rom_gpio_pad_select_gpio(config->int_gpio_num);
        const gpio_config_t cfg = {
            .pin_bit_mask = BIT64(config->int_gpio_num),
            .mode = GPIO_MODE_INPUT,
            .intr_type = config->levels.interrupt ? GPIO_INTR_POSEDGE : GPIO_INTR_NEGEDGE};
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
                if (config->int_gpio_num != GPIO_NUM_NC)
                    gpio_reset_pin(th->config.int_gpio_num);

                free(th);
                log_e("Registering interrupt callback failed");
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

    log_d("handle:0x%08x", th);
    *handle = th;

    return ESP_OK;
}

#endif // TOUCH_CST328_I2C