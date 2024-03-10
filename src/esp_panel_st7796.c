#ifdef LCD_ST7796_SPI

#include <esp_panel_st7796.h>
#include <esp32-hal-log.h>
#include <esp_rom_gpio.h>
#include <esp_heap_caps.h>
#include <memory.h>
#include <esp_lcd_panel_commands.h>
#include <esp_lcd_types.h>
#include <esp_lcd_panel_interface.h>
#include <esp_lcd_panel_io.h>

typedef struct
{
    esp_lcd_panel_t base;
    esp_lcd_panel_io_handle_t io;
    esp_lcd_panel_dev_config_t config;
    // Data
    int x_gap;
    int y_gap;
    uint8_t madctl;
} st7796_panel_t;

const lcd_init_cmd_t st7796_vendor_specific_init_default[] = {
    {0xf0, (const uint8_t[]){0xc3}, 1, 0},
    {0xf0, (const uint8_t[]){0x96}, 1, 0},
    {0xb4, (const uint8_t[]){0x01}, 1, 0},
    {0xb7, (const uint8_t[]){0xc6}, 1, 0},
    {0xe8, (const uint8_t[]){0x40, 0x8a, 0x00, 0x00, 0x29, 0x19, 0xa5, 0x33}, 8, 0},
    {0xc1, (const uint8_t[]){0x06}, 1, 0},
    {0xc2, (const uint8_t[]){0xa7}, 1, 0},
    {0xc5, (const uint8_t[]){0x18}, 1, 0},
    {0xe0, (const uint8_t[]){0xf0, 0x09, 0x0b, 0x06, 0x04, 0x15, 0x2f, 0x54, 0x42, 0x3c, 0x17, 0x14, 0x18, 0x1b}, 14, 0},
    {0xe1, (const uint8_t[]){0xf0, 0x09, 0x0b, 0x06, 0x04, 0x03, 0x2d, 0x43, 0x42, 0x3b, 0x16, 0x14, 0x17, 0x1b}, 14, 0},
    {0xf0, (const uint8_t[]){0x3c}, 1, 0},
    {0xf0, (const uint8_t[]){0x69}, 1, 0},
};

esp_err_t st7796_reset(esp_lcd_panel_t *panel)
{
    log_v("panel:0x%08x", panel);
    if (panel == NULL)
        return ESP_ERR_INVALID_ARG;

    const st7796_panel_t *ph = (st7796_panel_t *)panel;

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

esp_err_t st7796_init(esp_lcd_panel_t *panel)
{
    log_v("panel:0x%08x", panel);
    if (panel == NULL)
        return ESP_ERR_INVALID_ARG;

    const st7796_panel_t *ph = (st7796_panel_t *)panel;

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
        colmod = 0x05;
        break;
    case 18: // RGB666
        colmod = 0x06;
        break;
    case 24: // RGB888
        colmod = 0x07;
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

    const lcd_init_cmd_t *cmd = st7796_vendor_specific_init_default;
    uint16_t cmds_size = sizeof(st7796_vendor_specific_init_default) / sizeof(lcd_init_cmd_t);
    if (ph->config.vendor_config != NULL)
    {
        cmd = ((st7796_vendor_config_t *)ph->config.vendor_config)->init_cmds;
        cmds_size = ((st7796_vendor_config_t *)ph->config.vendor_config)->init_cmds_size;
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

esp_err_t st7796_draw_bitmap(esp_lcd_panel_t *panel, int x_start, int y_start, int x_end, int y_end, const void *color_data)
{
    log_v("panel:0x%08x, x_start:%d, y_start:%d, x_end:%d, y_end:%d, color_data:0x%08x", panel, x_start, y_start, x_end, y_end, color_data);
    if (panel == NULL || color_data == NULL)
        return ESP_ERR_INVALID_ARG;

    const st7796_panel_t *ph = (st7796_panel_t *)panel;

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

esp_err_t st7796_invert_color(esp_lcd_panel_t *panel, bool invert)
{
    log_v("panel:0x%08x, invert:%d", panel, invert);
    if (panel == NULL)
        return ESP_ERR_INVALID_ARG;

    const st7796_panel_t *ph = (st7796_panel_t *)panel;

    esp_err_t res;
    if ((res = esp_lcd_panel_io_tx_param(ph->io, invert ? LCD_CMD_INVON : LCD_CMD_INVOFF, NULL, 0)) != ESP_OK)
    {
        log_e("Sending LCD_CMD_INVON/LCD_CMD_INVOFF failed");
        return res;
    }

    return ESP_OK;
}

esp_err_t st7796_update_madctl(st7796_panel_t *ph)
{
    esp_err_t res;
    if ((res = esp_lcd_panel_io_tx_param(ph->io, LCD_CMD_MADCTL, &ph->madctl, 1)) != ESP_OK)
    {
        log_e("Sending LCD_CMD_MADCTL failed");
        return res;
    }

    return ESP_OK;
}

esp_err_t st7796_mirror(esp_lcd_panel_t *panel, bool mirror_x, bool mirror_y)
{
    log_v("panel:0x%08x, mirror_x:%d, mirror_y:%d", panel, mirror_x, mirror_y);
    if (panel == NULL)
        return ESP_ERR_INVALID_ARG;

    st7796_panel_t *ph = (st7796_panel_t *)panel;

    if (mirror_x)
        ph->madctl |= LCD_CMD_MX_BIT;
    else
        ph->madctl &= ~LCD_CMD_MX_BIT;

    if (mirror_y)
        ph->madctl |= LCD_CMD_MY_BIT;
    else
        ph->madctl &= ~LCD_CMD_MY_BIT;

    return st7796_update_madctl(ph);
}

esp_err_t st7796_swap_xy(esp_lcd_panel_t *panel, bool swap_xy)
{
    log_v("panel:0x%08x, swap_xy:%d", panel, swap_xy);
    if (panel == NULL)
        return ESP_ERR_INVALID_ARG;

    st7796_panel_t *ph = (st7796_panel_t *)panel;

    if (swap_xy)
        ph->madctl |= LCD_CMD_MV_BIT;
    else
        ph->madctl &= ~LCD_CMD_MV_BIT;

    return st7796_update_madctl(ph);
}

esp_err_t st7796_set_gap(esp_lcd_panel_t *panel, int x_gap, int y_gap)
{
    log_v("panel:0x%08x, x_gap:%d, y_gap:%d", panel, x_gap, y_gap);
    if (panel == NULL)
        return ESP_ERR_INVALID_ARG;

    st7796_panel_t *ph = (st7796_panel_t *)panel;

    ph->x_gap = x_gap;
    ph->y_gap = y_gap;

    return ESP_OK;
}

esp_err_t st7796_disp_off(esp_lcd_panel_t *panel, bool off)
{
    log_v("panel:0x%08x, off:%d", panel, off);
    if (panel == NULL)
        return ESP_ERR_INVALID_ARG;

    const st7796_panel_t *ph = (st7796_panel_t *)panel;

    esp_err_t res;
    if ((res = esp_lcd_panel_io_tx_param(ph->io, off ? LCD_CMD_DISPOFF : LCD_CMD_DISPON, NULL, 0)) != ESP_OK)
    {
        log_e("Sending LCD_CMD_DISPOFF/LCD_CMD_DISPON failed");
        return res;
    }

    return ESP_OK;
}

esp_err_t st7796_del(esp_lcd_panel_t *panel)
{
    log_v("panel:0x%08x", panel);
    if (panel == NULL)
        return ESP_ERR_INVALID_ARG;

    st7796_panel_t *ph = (st7796_panel_t *)panel;

    // Reset RESET
    if (ph->config.reset_gpio_num != GPIO_NUM_NC)
        gpio_reset_pin(ph->config.reset_gpio_num);

    free(ph);

    return ESP_OK;
}

esp_err_t esp_lcd_new_panel_st7796(const esp_lcd_panel_io_handle_t io, const esp_lcd_panel_dev_config_t *config, esp_lcd_panel_handle_t *handle)
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

    st7796_panel_t *ph = heap_caps_calloc(1, sizeof(st7796_panel_t), MALLOC_CAP_DEFAULT);
    if (ph == NULL)
    {
        log_e("No memory available for st7796_panel_t");
        return ESP_ERR_NO_MEM;
    }

    ph->io = io;
    memcpy(&ph->config, config, sizeof(esp_lcd_panel_dev_config_t));
    ph->madctl = madctl;

    ph->base.del = st7796_del;
    ph->base.reset = st7796_reset;
    ph->base.init = st7796_init;
    ph->base.draw_bitmap = st7796_draw_bitmap;
    ph->base.invert_color = st7796_invert_color;
    ph->base.mirror = st7796_mirror;
    ph->base.swap_xy = st7796_swap_xy;
    ph->base.set_gap = st7796_set_gap;
    ph->base.disp_off = st7796_disp_off;

    log_d("handle: 0x%08x", ph);
    *handle = (esp_lcd_panel_handle_t)ph;

    return ESP_OK;
}

#endif