#ifdef LCD_ST7701_SPI

#include <esp_lcd_panel_st7701.h>
#include <esp32-hal-log.h>
#include <esp_rom_gpio.h>
#include <esp_heap_caps.h>
#include <memory.h>
#include <esp_lcd_panel_commands.h>
#include <esp_lcd_types.h>
#include <esp_lcd_panel_interface.h>
#include <esp_lcd_panel_io.h>

//#define ST7701_CMD_SDIR (0xC7)
//#define ST7701_CMD_SS_BIT (1 << 2)

typedef struct
{
    esp_lcd_panel_t base;
    esp_lcd_panel_io_handle_t io;
    esp_lcd_panel_dev_config_t config;
    // Data
    int x_gap;
    int y_gap;
    uint8_t madctl;
} st7701_panel_t;

const lcd_init_cmd_t st7701_vendor_specific_init_default[] = {
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
    {0x29, NULL, 0, 0}};

esp_err_t st7701_reset(esp_lcd_panel_t *panel)
{
    log_v("panel:0x%08x", panel);
    if (panel == NULL)
        return ESP_ERR_INVALID_ARG;

    const st7701_panel_t *ph = (st7701_panel_t *)panel;

    if (ph->config.reset_gpio_num != GPIO_NUM_NC)
    {
        // Hardware reset
        gpio_set_level(ph->config.reset_gpio_num, ph->config.flags.reset_active_high);
        vTaskDelay(pdMS_TO_TICKS(1));
        gpio_set_level(ph->config.reset_gpio_num, !ph->config.flags.reset_active_high);
    }
    else
    {
        esp_err_t res;
        // Software reset
        if ((res = esp_lcd_panel_io_tx_param(ph->io, LCD_CMD_SWRESET, NULL, 0)) != ESP_OK)
        {
            log_e("Sending LCD_CMD_SWRESET failed");
            return res;
        }
    }

    vTaskDelay(pdMS_TO_TICKS(5));

    return ESP_OK;
}

esp_err_t st7701_init(esp_lcd_panel_t *panel)
{
    log_v("panel:0x%08x", panel);
    if (panel == NULL)
        return ESP_ERR_INVALID_ARG;

    const st7701_panel_t *ph = (st7701_panel_t *)panel;

    esp_err_t res;
    if ((res = esp_lcd_panel_io_tx_param(ph->io, LCD_CMD_SLPOUT, NULL, 0)) != ESP_OK)
    {
        log_e("Sending SLPOUT failed");
        return res;
    }

    vTaskDelay(pdMS_TO_TICKS(100));

    uint8_t colmod;
    switch (ph->config.bits_per_pixel)
    {
    case 16: // RGB565
        colmod = 0x50;
        break;
    case 18: // RGB666
        colmod = 0x60;
        break;
    case 24: // RGB888
        colmod = 0x70;
        break;
    default:
        log_e("Invalid bits per pixel: %d. Only RGB565, RGB666 and RGB888 are supported", ph->config.bits_per_pixel);
        return ESP_ERR_INVALID_ARG;
    }

    if ((res = esp_lcd_panel_io_tx_param(ph->io, LCD_CMD_MADCTL, &ph->madctl, 1)) != ESP_OK ||
        (res = esp_lcd_panel_io_tx_param(ph->io, LCD_CMD_COLMOD, &colmod, 1)) != ESP_OK)
    {
        log_e("Sending MADCTL/COLMOD failed");
        return res;
    }

    const lcd_init_cmd_t *cmd = st7701_vendor_specific_init_default;
    uint16_t cmds_size = sizeof(st7701_vendor_specific_init_default) / sizeof(lcd_init_cmd_t);
    if (ph->config.vendor_config != NULL)
    {
        cmd = ((st7701_vendor_config_t *)ph->config.vendor_config)->init_cmds;
        cmds_size = ((st7701_vendor_config_t *)ph->config.vendor_config)->init_cmds_size;
    }

    while (cmds_size-- > 0)
    {
        if ((res = esp_lcd_panel_io_tx_param(ph->io, cmd->cmd, cmd->data, cmd->bytes)) != ESP_OK)
        {
            log_e("Sending command: 0x%02x failed", cmd->cmd);
            return res;
        }

        vTaskDelay(pdMS_TO_TICKS(cmd->delay_ms));
        cmd++;
    }

    return ESP_OK;
}

esp_err_t st7701_draw_bitmap(esp_lcd_panel_t *panel, int x_start, int y_start, int x_end, int y_end, const void *color_data)
{
    log_v("panel:0x%08x, x_start:%d, y_start:%d, x_end:%d, y_end:%d, color_data:0x%08x", panel, x_start, y_start, x_end, y_end, color_data);
    if (panel == NULL || color_data == NULL)
        return ESP_ERR_INVALID_ARG;

    const st7701_panel_t *ph = (st7701_panel_t *)panel;

    if (x_start >= x_end)
    {
        log_w("X-start greater than the x-end");
        return ESP_ERR_INVALID_ARG;
    }

    if (y_start >= y_end)
    {
        log_w("Y-start greater than the y-end");
        return ESP_ERR_INVALID_ARG;
    }

    // Correct for gap
    x_start += ph->x_gap;
    x_end += ph->x_gap;
    y_start += ph->y_gap;
    y_end += ph->y_gap;

    esp_err_t res;
    const uint8_t caset[4] = {x_start >> 8, x_start, (x_end - 1) >> 8, (x_end - 1)};
    const uint8_t raset[4] = {y_start >> 8, y_start, (y_end - 1) >> 8, (y_end - 1)};
    if ((res = esp_lcd_panel_io_tx_param(ph->io, LCD_CMD_CASET, caset, sizeof(caset))) != ESP_OK ||
        (res = esp_lcd_panel_io_tx_param(ph->io, LCD_CMD_RASET, raset, sizeof(raset))) != ESP_OK)
    {
        log_e("Sending CASET/RASET failed");
        return res;
    }

    uint8_t bytes_per_pixel = (ph->config.bits_per_pixel + 0x7) >> 3;
    size_t len = (x_end - x_start) * (y_end - y_start) * bytes_per_pixel;
    if ((res = esp_lcd_panel_io_tx_color(ph->io, LCD_CMD_RAMWR, color_data, len)) != ESP_OK)
    {
        log_e("Sending RAMWR failed");
        return res;
    }

    return ESP_OK;
}

esp_err_t st7701_invert_color(esp_lcd_panel_t *panel, bool invert)
{
    log_v("panel:0x%08x, invert:%d", panel, invert);
    if (panel == NULL)
        return ESP_ERR_INVALID_ARG;

    const st7701_panel_t *ph = (st7701_panel_t *)panel;

    esp_err_t res;
    if ((res = esp_lcd_panel_io_tx_param(ph->io, invert ? LCD_CMD_INVON : LCD_CMD_INVOFF, NULL, 0)) != ESP_OK)
    {
        log_e("Sending LCD_CMD_INVON/LCD_CMD_INVOFF failed");
        return res;
    }

    return ESP_OK;
}

esp_err_t st7701_update_madctl(st7701_panel_t *ph)
{
    esp_err_t res;
    if ((res = esp_lcd_panel_io_tx_param(ph->io, LCD_CMD_MADCTL, &ph->madctl, 1)) != ESP_OK)
    {
        log_e("Sending LCD_CMD_MADCTL failed");
        return res;
    }

    return ESP_OK;
}

esp_err_t st7701_mirror(esp_lcd_panel_t *panel, bool mirror_x, bool mirror_y)
{
    log_v("panel:0x%08x, mirror_x:%d, mirror_y:%d", panel, mirror_x, mirror_y);
    if (panel == NULL)
        return ESP_ERR_INVALID_ARG;

    st7701_panel_t *ph = (st7701_panel_t *)panel;

    if (mirror_x)
        ph->madctl |= LCD_CMD_MX_BIT;
    else
        ph->madctl &= ~LCD_CMD_MX_BIT;

    if (mirror_y)
        ph->madctl |= LCD_CMD_MY_BIT;
    else
        ph->madctl &= ~LCD_CMD_MY_BIT;

    return st7701_update_madctl(ph);
}

esp_err_t st7701_swap_xy(esp_lcd_panel_t *panel, bool swap_xy)
{
    log_v("panel:0x%08x, swap_xy:%d", panel, swap_xy);
    if (panel == NULL)
        return ESP_ERR_INVALID_ARG;

    st7701_panel_t *ph = (st7701_panel_t *)panel;

    if (swap_xy)
        ph->madctl |= LCD_CMD_MV_BIT;
    else
        ph->madctl &= ~LCD_CMD_MV_BIT;

    return st7701_update_madctl(ph);
}

esp_err_t st7701_set_gap(esp_lcd_panel_t *panel, int x_gap, int y_gap)
{
    log_v("panel:0x%08x, x_gap:%d, y_gap:%d", panel, x_gap, y_gap);
    if (panel == NULL)
        return ESP_ERR_INVALID_ARG;

    st7701_panel_t *ph = (st7701_panel_t *)panel;

    ph->x_gap = x_gap;
    ph->y_gap = y_gap;

    return ESP_OK;
}

esp_err_t st7701_disp_off(esp_lcd_panel_t *panel, bool off)
{
    log_v("panel:0x%08x, off:%d", panel, off);
    if (panel == NULL)
        return ESP_ERR_INVALID_ARG;

    const st7701_panel_t *ph = (st7701_panel_t *)panel;

    esp_err_t res;
    if ((res = esp_lcd_panel_io_tx_param(ph->io, off ? LCD_CMD_DISPOFF : LCD_CMD_DISPON, NULL, 0)) != ESP_OK)
    {
        log_e("Sending LCD_CMD_DISPOFF/LCD_CMD_DISPON failed");
        return res;
    }

    return ESP_OK;
}

esp_err_t st7701_del(esp_lcd_panel_t *panel)
{
    log_v("panel:0x%08x", panel);
    if (panel == NULL)
        return ESP_ERR_INVALID_ARG;

    st7701_panel_t *ph = (st7701_panel_t *)panel;

    // Reset RESET
    if (ph->config.reset_gpio_num != GPIO_NUM_NC)
        gpio_reset_pin(ph->config.reset_gpio_num);

    free(ph);

    return ESP_OK;
}

esp_err_t esp_lcd_new_panel_st7701(const esp_lcd_panel_io_handle_t io, const esp_lcd_panel_dev_config_t *config, esp_lcd_panel_handle_t *handle)
{
    log_v("io:0x%08x, config:0x%08x, handle:0x%08x", io, config, handle);
    if (io == NULL || config == NULL || handle == NULL)
        return ESP_ERR_INVALID_ARG;

    if (config->reset_gpio_num != GPIO_NUM_NC && !GPIO_IS_VALID_GPIO(config->reset_gpio_num))
    {
        log_e("Invalid GPIO RST pin: %d", config->reset_gpio_num);
        return ESP_ERR_INVALID_ARG;
    }

    uint8_t madctl;
    switch (config->color_space)
    {
    case ESP_LCD_COLOR_SPACE_RGB:
        madctl = 0;
        break;
    case ESP_LCD_COLOR_SPACE_BGR:
        madctl = LCD_CMD_BGR_BIT;
        break;
    default:
        log_e("Invalid color space: %d. Only RGB and BGR are supported", config->color_space);
        return ESP_ERR_INVALID_ARG;
    }

    if (config->reset_gpio_num != GPIO_NUM_NC)
    {
        esp_err_t res;
        const gpio_config_t cfg = {
            .pin_bit_mask = BIT64(config->reset_gpio_num),
            .mode = GPIO_MODE_OUTPUT};
        if ((res = gpio_config(&cfg)) != ESP_OK)
        {
            log_e("Configuring GPIO for RST failed");
            return res;
        }
    }

    st7701_panel_t *ph = heap_caps_calloc(1, sizeof(st7701_panel_t), MALLOC_CAP_DEFAULT);
    if (ph == NULL)
    {
        log_e("No memory available for st7701_panel_t");
        return ESP_ERR_NO_MEM;
    }

    ph->io = io;
    memcpy(&ph->config, config, sizeof(esp_lcd_panel_dev_config_t));
    ph->madctl = madctl;

    ph->base.del = st7701_del;
    ph->base.reset = st7701_reset;
    ph->base.init = st7701_init;
    ph->base.draw_bitmap = st7701_draw_bitmap;
    ph->base.invert_color = st7701_invert_color;
    ph->base.mirror = st7701_mirror;
    ph->base.swap_xy = st7701_swap_xy;
    ph->base.set_gap = st7701_set_gap;
    ph->base.disp_off = st7701_disp_off;

    log_d("handle: 0x%08x", ph);
    *handle = (esp_lcd_panel_handle_t)ph;

    return ESP_OK;
}

#endif