#ifdef _LCD_ST7701_PAR

/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Changed made bij rzeldent for esp32-4848S040:
 *  - Adapter to not use user_data (not available < IDF 5)
 *  - Use panel_ops for calling original functions
 *  - Replaced default initialization
 */

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_check.h"
#include "esp_lcd_panel_commands.h"
#include "esp_lcd_panel_interface.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_rgb.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_log.h"

#include "esp_lcd_st7701.h"

#define ST7701_CMD_SDIR     (0xC7)
#define ST7701_CMD_SS_BIT   (1 << 2)

#define ST7701_CMD_CND2BKxSEL       (0xFF)
#define ST7701_CMD_BKxSEL_BYTE0     (0x77)
#define ST7701_CMD_BKxSEL_BYTE1     (0x01)
#define ST7701_CMD_BKxSEL_BYTE2     (0x00)
#define ST7701_CMD_BKxSEL_BYTE3     (0x00)
#define ST7701_CMD_CN2_BIT          (1 << 4)

typedef struct {
    esp_lcd_panel_t base;
    esp_lcd_panel_io_handle_t io;
    int reset_gpio_num;
    uint8_t madctl_val; // Save current value of LCD_CMD_MADCTL register
    uint8_t colmod_val; // Save current value of LCD_CMD_COLMOD register
    const st7701_lcd_init_cmd_t *init_cmds;
    uint16_t init_cmds_size;
    struct {
        unsigned int mirror_by_cmd: 1;
        unsigned int auto_del_panel_io: 1;
        unsigned int display_on_off_use_cmd: 1;
        unsigned int reset_level: 1;
    } flags;
    // To save the original functions of RGB panel
    esp_lcd_panel_t* original;
} st7701_panel_t;

static const char *TAG = "st7701";

static esp_err_t panel_st7701_send_init_cmds(st7701_panel_t *st7701);

static esp_err_t panel_st7701_init(esp_lcd_panel_t *panel);
static esp_err_t panel_st7701_del(esp_lcd_panel_t *panel);
static esp_err_t panel_st7701_reset(esp_lcd_panel_t *panel);
static esp_err_t panel_st7701_draw_bitmap(esp_lcd_panel_t *panel, int x_start, int y_start, int x_end, int y_end, const void *color_data);
static esp_err_t panel_st7701_mirror(esp_lcd_panel_t *panel, bool mirror_x, bool mirror_y);
static esp_err_t panel_st7701_swap_xy(esp_lcd_panel_t *panel, bool swap_axes);
static esp_err_t panel_st7701_set_gap(esp_lcd_panel_t *panel, int x_gap, int y_gap);
static esp_err_t panel_st7701_invert_color(esp_lcd_panel_t *panel, bool invert_color_data);
static esp_err_t panel_st7701_disp_on_off(esp_lcd_panel_t *panel, bool off);

esp_err_t esp_lcd_new_panel_st7701(const esp_lcd_panel_io_handle_t io, const esp_lcd_panel_dev_config_t *panel_dev_config,
                                   esp_lcd_panel_handle_t *ret_panel)
{
    ESP_RETURN_ON_FALSE(io && panel_dev_config && ret_panel, ESP_ERR_INVALID_ARG, TAG, "invalid arguments");
    st7701_vendor_config_t *vendor_config = (st7701_vendor_config_t *)panel_dev_config->vendor_config;
    ESP_RETURN_ON_FALSE(vendor_config && vendor_config->rgb_config, ESP_ERR_INVALID_ARG, TAG, "`vendor_config` and `rgb_config` are necessary");
    ESP_RETURN_ON_FALSE(!vendor_config->flags.auto_del_panel_io || !vendor_config->flags.mirror_by_cmd,
                        ESP_ERR_INVALID_ARG, TAG, "`mirror_by_cmd` and `auto_del_panel_io` cannot work together");


    esp_err_t ret = ESP_OK;
    st7701_panel_t *st7701 = (st7701_panel_t *)calloc(1, sizeof(st7701_panel_t));
    ESP_RETURN_ON_FALSE(st7701, ESP_ERR_NO_MEM, TAG, "no mem for st7701 panel");

    if (panel_dev_config->reset_gpio_num >= 0) {
        gpio_config_t io_conf = {
            .mode = GPIO_MODE_OUTPUT,
            .pin_bit_mask = 1ULL << panel_dev_config->reset_gpio_num,
        };
        ESP_GOTO_ON_ERROR(gpio_config(&io_conf), err, TAG, "configure GPIO for RST line failed");
    }
#if ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(5, 0, 0)
    switch (panel_dev_config->color_space) {
    case ESP_LCD_COLOR_SPACE_RGB:
        st7701->madctl_val = 0;
        break;
    case ESP_LCD_COLOR_SPACE_BGR:
        st7701->madctl_val |= LCD_CMD_BGR_BIT;
        break;
    default:
        ESP_GOTO_ON_FALSE(false, ESP_ERR_NOT_SUPPORTED, err, TAG, "unsupported color space");
        break;
    }
#else
    switch (panel_dev_config->rgb_endian) {
    case LCD_RGB_ENDIAN_RGB:
        st7701->madctl_val = 0;
        break;
    case LCD_RGB_ENDIAN_BGR:
        st7701->madctl_val |= LCD_CMD_BGR_BIT;
        break;
    default:
        ESP_GOTO_ON_FALSE(false, ESP_ERR_NOT_SUPPORTED, err, TAG, "unsupported rgb endian");
        break;
    }
#endif
    st7701->colmod_val = 0;
    switch (panel_dev_config->bits_per_pixel) {
    case 16: // RGB565
        st7701->colmod_val = 0x50;
        break;
    case 18: // RGB666
        st7701->colmod_val = 0x60;
        break;
    case 24: // RGB888
        st7701->colmod_val = 0x70;
        break;
    default:
        ESP_GOTO_ON_FALSE(false, ESP_ERR_NOT_SUPPORTED, err, TAG, "unsupported pixel width");
        break;
    }

    st7701->io = io;
    st7701->init_cmds = vendor_config->init_cmds;
    st7701->init_cmds_size = vendor_config->init_cmds_size;
    st7701->reset_gpio_num = panel_dev_config->reset_gpio_num;
    st7701->flags.mirror_by_cmd = vendor_config->flags.mirror_by_cmd;
    st7701->flags.display_on_off_use_cmd = (vendor_config->rgb_config->disp_gpio_num >= 0) ? 0 : 1;
    st7701->flags.auto_del_panel_io = vendor_config->flags.auto_del_panel_io;
    st7701->flags.reset_level = panel_dev_config->flags.reset_active_high;

    if (st7701->flags.auto_del_panel_io) {
        if (st7701->reset_gpio_num >= 0) {  // Perform hardware reset
            gpio_set_level(st7701->reset_gpio_num, st7701->flags.reset_level);
            vTaskDelay(pdMS_TO_TICKS(10));
            gpio_set_level(st7701->reset_gpio_num, !st7701->flags.reset_level);
        } else { // Perform software reset
            ESP_GOTO_ON_ERROR(esp_lcd_panel_io_tx_param(io, LCD_CMD_SWRESET, NULL, 0), err, TAG, "send command failed");
        }
        vTaskDelay(pdMS_TO_TICKS(120));

        /**
         * In order to enable the 3-wire SPI interface pins (such as SDA and SCK) to share other pins of the RGB interface
         * (such as HSYNC) and save GPIOs, we need to send LCD initialization commands via the 3-wire SPI interface before
         * `esp_lcd_new_rgb_panel()` is called.
         */
        ESP_GOTO_ON_ERROR(panel_st7701_send_init_cmds(st7701), err, TAG, "send init commands failed");
        // After sending the initialization commands, the 3-wire SPI interface can be deleted
        ESP_GOTO_ON_ERROR(esp_lcd_panel_io_del(io), err, TAG, "delete panel IO failed");
        st7701->io = NULL;
        ESP_LOGD(TAG, "delete panel IO");
    }

    // Create RGB panel
    ESP_GOTO_ON_ERROR(esp_lcd_new_rgb_panel(vendor_config->rgb_config, &st7701->original), err, TAG, "create RGB panel failed");
    ESP_LOGD(TAG, "new RGB panel @%p", st7701->original);

    st7701->base.reset = panel_st7701_reset;
    st7701->base.init = panel_st7701_init;
    st7701->base.del = panel_st7701_del;
    st7701->base.draw_bitmap = panel_st7701_draw_bitmap;
    st7701->base.mirror = panel_st7701_mirror;
    st7701->base.swap_xy = panel_st7701_swap_xy;
    st7701->base.set_gap = panel_st7701_set_gap;
    st7701->base.invert_color = panel_st7701_invert_color;
    st7701->base.disp_off = panel_st7701_disp_on_off;

    *ret_panel = &(st7701->base);
    ESP_LOGD(TAG, "new st7701 panel @%p", st7701);

    ESP_LOGI(TAG, "LCD panel create success, version: %d.%d.%d", ESP_LCD_ST7701_VER_MAJOR, ESP_LCD_ST7701_VER_MINOR,
             ESP_LCD_ST7701_VER_PATCH);

    return ESP_OK;

err:
    if (st7701) {
        if (panel_dev_config->reset_gpio_num >= 0) {
            gpio_reset_pin(panel_dev_config->reset_gpio_num);
        }
        free(st7701);
    }
    return ret;
}

// Init taken from Arduino_GFX as the stock st7701 (provided by EspressIf did not work)
static const st7701_lcd_init_cmd_t vendor_specific_init_default[] = {
//  {cmd, { data }, data_size, delay_ms}
    {0xFF, (uint8_t[]){0x77, 0x01, 0x00, 0x00, 0x10}, 5, 0},
    {0xC0, (uint8_t[]){0x3B, 0x00}, 2, 0},
    {0xC1, (uint8_t[]){0x0D, 0x02}, 2, 0},
    {0xC2, (uint8_t[]){0x31, 0x05}, 2, 0},
    {0xCD, (uint8_t[]){0x00}, 1, 0},
    // Positive Voltage Gamma Control
    {0xB0, (uint8_t[]){0x00, 0x11, 0x18, 0x0E, 0x11, 0x06, 0x07, 0x08, 0x07, 0x22, 0x04, 0x12, 0x0F, 0xAA, 0x31, 0x18}, 16, 0},
    // Negative Voltage Gamma Control
    {0xB1, (uint8_t[]){0x00, 0x11, 0x19, 0x0E, 0x12, 0x07, 0x08, 0x08, 0x08, 0x22, 0x04, 0x11, 0x11, 0xA9, 0x32, 0x18}, 16, 0},
    // PAGE1
    {0xFF, (uint8_t[]){0x77, 0x01, 0x00, 0x00, 0x11}, 5, 0},
    {0xB0, (uint8_t[]){0x60}, 1, 0}, // Vop=4.7375v
    {0xB1, (uint8_t[]){0x32}, 1, 0}, // VCOM=32
    {0xB2, (uint8_t[]){0x07}, 1, 0}, // VGH=15v
    {0xB3, (uint8_t[]){0x80}, 1, 0},
    {0xB5, (uint8_t[]){0x49}, 1, 0}, // VGL=-10.17v
    {0xB7, (uint8_t[]){0x85}, 1, 0},
    {0xB8, (uint8_t[]){0x21}, 1, 0}, // AVDD=6.6 & AVCL=-4.6
    {0xC1, (uint8_t[]){0x78}, 1, 0},
    {0xC2, (uint8_t[]){0x78}, 1, 0},
    {0xE0, (uint8_t[]){0x00, 0x1B, 0x02}, 3, 0},
    {0xE1, (uint8_t[]){0x08, 0xA0, 0x00, 0x00, 0x07, 0xA0, 0x00, 0x00, 0x00, 0x44, 0x44}, 11, 0},
    {0xE2, (uint8_t[]){0x11, 0x11, 0x44, 0x44, 0xED, 0xA0, 0x00, 0x00, 0xEC, 0xA0, 0x00, 0x00}, 12, 0},
    {0xE3, (uint8_t[]){0x00, 0x00, 0x11, 0x11}, 4, 0},
    {0xE4, (uint8_t[]){0x44, 0x44}, 2, 0},
    {0xE5, (uint8_t[]){0x0A, 0xE9, 0xD8, 0xA0, 0x0C, 0xEB, 0xD8, 0xA0, 0x0E, 0xED, 0xD8, 0xA0, 0x10, 0xEF, 0xD8, 0xA0}, 16, 0},
    {0xE6, (uint8_t[]){0x00, 0x00, 0x11, 0x11}, 4, 0},
    {0xE7, (uint8_t[]){0x44, 0x44}, 2, 0},
    {0xE8, (uint8_t[]){0x09, 0xE8, 0xD8, 0xA0, 0x0B, 0xEA, 0xD8, 0xA0, 0x0D, 0xEC, 0xD8, 0xA0, 0x0F, 0xEE, 0xD8, 0xA0}, 16, 0},
    {0xEB, (uint8_t[]){0x02, 0x00, 0xE4, 0xE4, 0x88, 0x00, 0x40}, 7, 0},
    {0xEC, (uint8_t[]){0x3C, 0x00}, 2, 0},
    {0xED, (uint8_t[]){0xAB, 0x89, 0x76, 0x54, 0x02, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x20, 0x45, 0x67, 0x98, 0xBA}, 16, 0},
    // VAP & VAN
    {0xFF, (uint8_t[]){0x77, 0x01, 0x00, 0x00, 0x13}, 5, 0},
    {0xE5, (uint8_t[]){0xE4}, 1, 0},
    {0xFF, (uint8_t[]){0x77, 0x01, 0x00, 0x00, 0x00}, 5, 0},
    // 0x70 RGB888, 0x60 RGB666, 0x50 RGB565
    {0x3A, (uint8_t[]){0x60}, 1, 0},
    // Sleep Out
    {0x11, NULL, 0, 120},
    // Display On
    {0x29, NULL, 0, 0}
};

static esp_err_t panel_st7701_send_init_cmds(st7701_panel_t *st7701)
{
    esp_lcd_panel_io_handle_t io = st7701->io;
    const st7701_lcd_init_cmd_t *init_cmds = NULL;
    uint16_t init_cmds_size = 0;
    bool is_command2_disable = true;
    bool is_cmd_overwritten = false;

    ESP_RETURN_ON_ERROR(esp_lcd_panel_io_tx_param(io, ST7701_CMD_CND2BKxSEL, (uint8_t []){
        ST7701_CMD_BKxSEL_BYTE0, ST7701_CMD_BKxSEL_BYTE1, ST7701_CMD_BKxSEL_BYTE2, ST7701_CMD_BKxSEL_BYTE3, 0x00
    }, 5), TAG, "Write cmd failed");
    // Set color format
    ESP_RETURN_ON_ERROR(esp_lcd_panel_io_tx_param(io, LCD_CMD_MADCTL, (uint8_t []){
        st7701->madctl_val
    }, 1), TAG, "Write cmd failed");
    ESP_RETURN_ON_ERROR(esp_lcd_panel_io_tx_param(io, LCD_CMD_COLMOD, (uint8_t []){
        st7701->colmod_val
    }, 1), TAG, "Write cmd failed");

    // vendor specific initialization, it can be different between manufacturers
    // should consult the LCD supplier for initialization sequence code
    if (st7701->init_cmds) {
        init_cmds = st7701->init_cmds;
        init_cmds_size = st7701->init_cmds_size;
    } else {
        init_cmds = vendor_specific_init_default;
        init_cmds_size = sizeof(vendor_specific_init_default) / sizeof(st7701_lcd_init_cmd_t);
    }

    for (int i = 0; i < init_cmds_size; i++) {
        // Check if the command has been used or conflicts with the internal only when command2 is disable
        if (is_command2_disable && (init_cmds[i].data_bytes > 0)) {
            switch (init_cmds[i].cmd) {
            case LCD_CMD_MADCTL:
                is_cmd_overwritten = true;
                st7701->madctl_val = ((uint8_t *)init_cmds[i].data)[0];
                break;
            case LCD_CMD_COLMOD:
                is_cmd_overwritten = true;
                st7701->colmod_val = ((uint8_t *)init_cmds[i].data)[0];
                break;
            default:
                is_cmd_overwritten = false;
                break;
            }

            if (is_cmd_overwritten) {
                is_cmd_overwritten = false;
                ESP_LOGW(TAG, "The %02Xh command has been used and will be overwritten by external initialization sequence",
                        init_cmds[i].cmd);
            }
        }

        // Send command
        ESP_RETURN_ON_ERROR(esp_lcd_panel_io_tx_param(io, init_cmds[i].cmd, init_cmds[i].data, init_cmds[i].data_bytes),
                            TAG, "send command failed");
        vTaskDelay(pdMS_TO_TICKS(init_cmds[i].delay_ms));

        // Check if the current cmd is the command2 disable cmd
        if ((init_cmds[i].cmd == ST7701_CMD_CND2BKxSEL) && (init_cmds[i].data_bytes > 4)) {
            is_command2_disable = !(((uint8_t *)init_cmds[i].data)[4] & ST7701_CMD_CN2_BIT);
        }
    }
    ESP_LOGD(TAG, "send init commands success");

    return ESP_OK;
}

static esp_err_t panel_st7701_init(esp_lcd_panel_t *panel)
{
    st7701_panel_t *st7701 = __containerof(panel, st7701_panel_t, base);

    if (!st7701->flags.auto_del_panel_io) {
        ESP_RETURN_ON_ERROR(panel_st7701_send_init_cmds(st7701), TAG, "send init commands failed");
    }
    // Init RGB panel
    ESP_RETURN_ON_ERROR(esp_lcd_panel_init(st7701->original), TAG, "init RGB panel failed");

    return ESP_OK;
}

static esp_err_t panel_st7701_del(esp_lcd_panel_t *panel)
{
    st7701_panel_t *st7701 = __containerof(panel, st7701_panel_t, base);

    if (st7701->reset_gpio_num >= 0) {
        gpio_reset_pin(st7701->reset_gpio_num);
    }
    // Delete RGB panel
    esp_lcd_panel_del(st7701->original);
    free(st7701);
    ESP_LOGD(TAG, "del st7701 panel @%p", st7701);
    return ESP_OK;
}

static esp_err_t panel_st7701_draw_bitmap(esp_lcd_panel_t *panel, int x_start, int y_start, int x_end, int y_end, const void *color_data)
{
    st7701_panel_t *st7701 = __containerof(panel, st7701_panel_t, base);
    return esp_lcd_panel_draw_bitmap(st7701->original, x_start, y_start, x_end, y_end, color_data);
}

static esp_err_t panel_st7701_reset(esp_lcd_panel_t *panel)
{
    st7701_panel_t *st7701 = __containerof(panel, st7701_panel_t, base);
    esp_lcd_panel_io_handle_t io = st7701->io;
    
    // Perform hardware reset
    if (st7701->reset_gpio_num >= 0) {
        gpio_set_level(st7701->reset_gpio_num, st7701->flags.reset_level);
        vTaskDelay(pdMS_TO_TICKS(10));
        gpio_set_level(st7701->reset_gpio_num, !st7701->flags.reset_level);
        vTaskDelay(pdMS_TO_TICKS(120));
    } else if (io) { // Perform software reset
        ESP_RETURN_ON_ERROR(esp_lcd_panel_io_tx_param(io, LCD_CMD_SWRESET, NULL, 0), TAG, "send command failed");
        vTaskDelay(pdMS_TO_TICKS(120));
    }
    // Reset RGB panel
    ESP_RETURN_ON_ERROR(esp_lcd_panel_reset(st7701->original), TAG, "reset RGB panel failed");

    return ESP_OK;
}

static esp_err_t panel_st7701_mirror(esp_lcd_panel_t *panel, bool mirror_x, bool mirror_y)
{
    st7701_panel_t *st7701 = __containerof(panel, st7701_panel_t, base);
    esp_lcd_panel_io_handle_t io = st7701->io;
    uint8_t sdir_val = 0;

    if (st7701->flags.mirror_by_cmd) {
        ESP_RETURN_ON_FALSE(io, ESP_FAIL, TAG, "Panel IO is deleted, cannot send command");
        // Control mirror through LCD command
        if (mirror_x) {
            sdir_val = ST7701_CMD_SS_BIT;
        } else {
            sdir_val = 0;
        }
        if (mirror_y) {
            st7701->madctl_val |= LCD_CMD_ML_BIT;
        } else {
            st7701->madctl_val &= ~LCD_CMD_ML_BIT;
        }
        ESP_RETURN_ON_ERROR(esp_lcd_panel_io_tx_param(io, ST7701_CMD_SDIR, (uint8_t[]) {
            sdir_val,
        }, 1), TAG, "send command failed");;
        ESP_RETURN_ON_ERROR(esp_lcd_panel_io_tx_param(io, LCD_CMD_MADCTL, (uint8_t[]) {
            st7701->madctl_val,
        }, 1), TAG, "send command failed");;
    } else {
        // Control mirror through RGB panel
        ESP_RETURN_ON_ERROR(esp_lcd_panel_mirror(st7701->original, mirror_x, mirror_y), TAG, "RGB panel mirror failed");
    }
    return ESP_OK;
}

static esp_err_t panel_st7701_swap_xy(esp_lcd_panel_t *panel, bool swap_axes)
{
    st7701_panel_t *st7701 = __containerof(panel, st7701_panel_t, base);
    return esp_lcd_panel_swap_xy(st7701->original, swap_axes);
}

static esp_err_t panel_st7701_set_gap(esp_lcd_panel_t *panel, int x_gap, int y_gap)
{
    st7701_panel_t *st7701 = __containerof(panel, st7701_panel_t, base);
    return esp_lcd_panel_set_gap(st7701->original, x_gap, y_gap);
}

static esp_err_t panel_st7701_invert_color(esp_lcd_panel_t *panel, bool invert_color_data)
{
    st7701_panel_t *st7701 = __containerof(panel, st7701_panel_t, base);
    return esp_lcd_panel_invert_color(st7701->original, invert_color_data);
}

static esp_err_t panel_st7701_disp_on_off(esp_lcd_panel_t *panel, bool on_off)
{
    st7701_panel_t *st7701 = __containerof(panel, st7701_panel_t, base);
    esp_lcd_panel_io_handle_t io = st7701->io;
    int command = 0;

    if (st7701->flags.display_on_off_use_cmd) {
        ESP_RETURN_ON_FALSE(io, ESP_FAIL, TAG, "Panel IO is deleted, cannot send command");
        // Control display on/off through LCD command
        if (on_off) {
            command = LCD_CMD_DISPON;
        } else {
            command = LCD_CMD_DISPOFF;
        }
        ESP_RETURN_ON_ERROR(esp_lcd_panel_io_tx_param(io, command, NULL, 0), TAG, "send command failed");
    } else {
        // Control display on/off through display control signal
        ESP_RETURN_ON_ERROR(esp_lcd_panel_disp_off(st7701->original, on_off), TAG, "RGB panel disp_off failed");
    }
    return ESP_OK;
}

#endif