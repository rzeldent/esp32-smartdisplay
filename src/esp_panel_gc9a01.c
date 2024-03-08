#ifdef LCD_GC9A01_SPI

#include <esp_panel_gc9a01.h>
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
    esp_lcd_panel_dev_config_t panel_dev_config;
    // Data
    int x_gap;
    int y_gap;
    uint8_t madctl;
} gc9a01_panel_t;

const lcd_init_cmd_t gc9a01_vendor_specific_init_default[] = {
    // Enable Inter Register
    {0xfe, (const uint8_t[]){0x00}, 0, 0},
    {0xef, (const uint8_t[]){0x00}, 0, 0},
    {0xeb, (const uint8_t[]){0x14}, 1, 0},
    {0x84, (const uint8_t[]){0x60}, 1, 0},
    {0x85, (const uint8_t[]){0xff}, 1, 0},
    {0x86, (const uint8_t[]){0xff}, 1, 0},
    {0x87, (const uint8_t[]){0xff}, 1, 0},
    {0x8e, (const uint8_t[]){0xff}, 1, 0},
    {0x8f, (const uint8_t[]){0xff}, 1, 0},
    {0x88, (const uint8_t[]){0x0a}, 1, 0},
    {0x89, (const uint8_t[]){0x23}, 1, 0},
    {0x8a, (const uint8_t[]){0x00}, 1, 0},
    {0x8b, (const uint8_t[]){0x80}, 1, 0},
    {0x8c, (const uint8_t[]){0x01}, 1, 0},
    {0x8d, (const uint8_t[]){0x03}, 1, 0},
    {0x90, (const uint8_t[]){0x08, 0x08, 0x08, 0x08}, 4, 0},
    {0xff, (const uint8_t[]){0x60, 0x01, 0x04}, 3, 0},
    {0xC3, (const uint8_t[]){0x13}, 1, 0},
    {0xC4, (const uint8_t[]){0x13}, 1, 0},
    {0xC9, (const uint8_t[]){0x30}, 1, 0},
    {0xbe, (const uint8_t[]){0x11}, 1, 0},
    {0xe1, (const uint8_t[]){0x10, 0x0e}, 2, 0},
    {0xdf, (const uint8_t[]){0x21, 0x0c, 0x02}, 3, 0},
    // Set gamma
    {0xF0, (const uint8_t[]){0x45, 0x09, 0x08, 0x08, 0x26, 0x2a}, 6, 0},
    {0xF1, (const uint8_t[]){0x43, 0x70, 0x72, 0x36, 0x37, 0x6f}, 6, 0},
    {0xF2, (const uint8_t[]){0x45, 0x09, 0x08, 0x08, 0x26, 0x2a}, 6, 0},
    {0xF3, (const uint8_t[]){0x43, 0x70, 0x72, 0x36, 0x37, 0x6f}, 6, 0},
    {0xed, (const uint8_t[]){0x1b, 0x0b}, 2, 0},
    {0xae, (const uint8_t[]){0x77}, 1, 0},
    {0xcd, (const uint8_t[]){0x63}, 1, 0},
    {0x70, (const uint8_t[]){0x07, 0x07, 0x04, 0x0e, 0x0f, 0x09, 0x07, 0x08, 0x03}, 9, 0},
    {0xE8, (const uint8_t[]){0x34}, 1, 0}, // 4 dot inversion
    {0x60, (const uint8_t[]){0x38, 0x0b, 0x6D, 0x6D, 0x39, 0xf0, 0x6D, 0x6D}, 8, 0},
    {0x61, (const uint8_t[]){0x38, 0xf4, 0x6D, 0x6D, 0x38, 0xf7, 0x6D, 0x6D}, 8, 0},
    {0x62, (const uint8_t[]){0x38, 0x0D, 0x71, 0xED, 0x70, 0x70, 0x38, 0x0F, 0x71, 0xEF, 0x70, 0x70}, 12, 0},
    {0x63, (const uint8_t[]){0x38, 0x11, 0x71, 0xF1, 0x70, 0x70, 0x38, 0x13, 0x71, 0xF3, 0x70, 0x70}, 12, 0},
    {0x64, (const uint8_t[]){0x28, 0x29, 0xF1, 0x01, 0xF1, 0x00, 0x07}, 7, 0},
    {0x66, (const uint8_t[]){0x3C, 0x00, 0xCD, 0x67, 0x45, 0x45, 0x10, 0x00, 0x00, 0x00}, 10, 0},
    {0x67, (const uint8_t[]){0x00, 0x3C, 0x00, 0x00, 0x00, 0x01, 0x54, 0x10, 0x32, 0x98}, 10, 0},
    {0x74, (const uint8_t[]){0x10, 0x45, 0x80, 0x00, 0x00, 0x4E, 0x00}, 7, 0},
    {0x98, (const uint8_t[]){0x3e, 0x07}, 2, 0},
    {0x99, (const uint8_t[]){0x3e, 0x07}, 2, 0}};

esp_err_t gc9a01_reset(esp_lcd_panel_t *panel)
{
    log_v("panel:0x%08x", panel);
    if (panel == NULL)
        return ESP_ERR_INVALID_ARG;

    const gc9a01_panel_t *ph = (gc9a01_panel_t *)panel;

    if (ph->panel_dev_config.reset_gpio_num != GPIO_NUM_NC)
    {
        // Hardware reset
        gpio_set_level(ph->panel_dev_config.reset_gpio_num, ph->panel_dev_config.flags.reset_active_high);
        vTaskDelay(pdMS_TO_TICKS(1));
        gpio_set_level(ph->panel_dev_config.reset_gpio_num, !ph->panel_dev_config.flags.reset_active_high);
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

    vTaskDelay(pdMS_TO_TICKS(120));

    return ESP_OK;
}

esp_err_t gc9a01_init(esp_lcd_panel_t *panel)
{
    log_v("panel:0x%08x", panel);
    if (panel == NULL)
        return ESP_ERR_INVALID_ARG;

    const gc9a01_panel_t *ph = (gc9a01_panel_t *)panel;

    esp_err_t res;
    if ((res = esp_lcd_panel_io_tx_param(ph->io, LCD_CMD_SLPOUT, NULL, 0)) != ESP_OK)
    {
        log_e("Sending SLPOUT failed");
        return res;
    }

    vTaskDelay(pdMS_TO_TICKS(100));

    uint8_t colmod;
    switch (ph->panel_dev_config.bits_per_pixel)
    {
    case 16: // RGB565
        colmod = 0x55;
        break;
    case 18: // RGB666
        colmod = 0x66;
        break;
    default:
        log_e("Invalid bits per pixel: %d. Only RGB565 and RGB666 are supported", ph->panel_dev_config.bits_per_pixel);
        return ESP_ERR_INVALID_ARG;
    }

    if ((res = esp_lcd_panel_io_tx_param(ph->io, LCD_CMD_MADCTL, &ph->madctl, 1)) != ESP_OK ||
        (res = esp_lcd_panel_io_tx_param(ph->io, LCD_CMD_COLMOD, &colmod, 1)) != ESP_OK)
    {
        log_e("Sending MADCTL/COLMOD failed");
        return res;
    }

    const lcd_init_cmd_t *cmd = gc9a01_vendor_specific_init_default;
    uint16_t cmds_size = sizeof(gc9a01_vendor_specific_init_default) / sizeof(lcd_init_cmd_t);
    if (ph->panel_dev_config.vendor_config != NULL)
    {
        cmd = ((gc9a01_vendor_config_t *)ph->panel_dev_config.vendor_config)->init_cmds;
        cmds_size = ((gc9a01_vendor_config_t *)ph->panel_dev_config.vendor_config)->init_cmds_size;
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

esp_err_t gc9a01_draw_bitmap(esp_lcd_panel_t *panel, int x_start, int y_start, int x_end, int y_end, const void *color_data)
{
    log_v("panel:0x%08x, x_start:%d, y_start:%d, x_end:%d, y_end:%d, color_data:0x%08x", panel, x_start, y_start, x_end, y_end, color_data);
    if (panel == NULL || color_data == NULL)
        return ESP_ERR_INVALID_ARG;

    const gc9a01_panel_t *ph = (gc9a01_panel_t *)panel;

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
    const uint8_t caset[4] = {x_start >> 8, x_start, (x_end - 1) >> 8, x_end - 1};
    const uint8_t raset[4] = {y_start >> 8, y_start, (y_end - 1) >> 8, y_end - 1};
    if ((res = esp_lcd_panel_io_tx_param(ph->io, LCD_CMD_CASET, caset, sizeof(caset))) != ESP_OK ||
        (res = esp_lcd_panel_io_tx_param(ph->io, LCD_CMD_RASET, raset, sizeof(raset))) != ESP_OK)
    {
        log_e("Sending CASET/RASET failed");
        return res;
    }

    uint8_t bytes_per_pixel = (ph->panel_dev_config.bits_per_pixel + 0x7) >> 3;
    size_t len = (x_end - x_start) * (y_end - y_start) * bytes_per_pixel;
    if ((res = esp_lcd_panel_io_tx_color(ph->io, LCD_CMD_RAMWR, color_data, len)) != ESP_OK)
    {
        log_e("Sending RAMWR failed");
        return res;
    }

    return ESP_OK;
}

esp_err_t gc9a01_invert_color(esp_lcd_panel_t *panel, bool invert)
{
    log_v("panel:0x%08x, invert:%d", panel, invert);
    if (panel == NULL)
        return ESP_ERR_INVALID_ARG;

    const gc9a01_panel_t *ph = (gc9a01_panel_t *)panel;

    esp_err_t res;
    if ((res = esp_lcd_panel_io_tx_param(ph->io, invert ? LCD_CMD_INVON : LCD_CMD_INVOFF, NULL, 0)) != ESP_OK)
    {
        log_e("Sending LCD_CMD_INVON/LCD_CMD_INVOFF failed");
        return res;
    }

    return ESP_OK;
}

esp_err_t gc9a01_update_madctl(gc9a01_panel_t *ph)
{
    esp_err_t res;
    if ((res = esp_lcd_panel_io_tx_param(ph->io, LCD_CMD_MADCTL, &ph->madctl, 1)) != ESP_OK)
    {
        log_e("Sending LCD_CMD_MADCTL failed");
        return res;
    }

    return ESP_OK;
}

esp_err_t gc9a01_mirror(esp_lcd_panel_t *panel, bool mirror_x, bool mirror_y)
{
    log_v("panel:0x%08x, mirror_x:%d, mirror_y:%d", panel, mirror_x, mirror_y);
    if (panel == NULL)
        return ESP_ERR_INVALID_ARG;

    gc9a01_panel_t *ph = (gc9a01_panel_t *)panel;

    if (mirror_x)
        ph->madctl |= LCD_CMD_MX_BIT;
    else
        ph->madctl &= ~LCD_CMD_MX_BIT;

    if (mirror_y)
        ph->madctl |= LCD_CMD_MY_BIT;
    else
        ph->madctl &= ~LCD_CMD_MY_BIT;

    return gc9a01_update_madctl(ph);
}

esp_err_t gc9a01_swap_xy(esp_lcd_panel_t *panel, bool swap_xy)
{
    log_v("panel:0x%08x, swap_xy:%d", panel, swap_xy);
    if (panel == NULL)
        return ESP_ERR_INVALID_ARG;

    gc9a01_panel_t *ph = (gc9a01_panel_t *)panel;

    if (swap_xy)
        ph->madctl |= LCD_CMD_MV_BIT;
    else
        ph->madctl &= ~LCD_CMD_MV_BIT;

    return gc9a01_update_madctl(ph);
}

esp_err_t gc9a01_set_gap(esp_lcd_panel_t *panel, int x_gap, int y_gap)
{
    log_v("panel:0x%08x, x_gap:%d, y_gap:%d", panel, x_gap, y_gap);
    if (panel == NULL)
        return ESP_ERR_INVALID_ARG;

    gc9a01_panel_t *ph = (gc9a01_panel_t *)panel;

    ph->x_gap = x_gap;
    ph->y_gap = y_gap;

    return ESP_OK;
}

esp_err_t gc9a01_disp_off(esp_lcd_panel_t *panel, bool off)
{
    log_v("panel:0x%08x, off:%d", panel, off);
    if (panel == NULL)
        return ESP_ERR_INVALID_ARG;

    const gc9a01_panel_t *ph = (gc9a01_panel_t *)panel;

    esp_err_t res;
    if ((res = esp_lcd_panel_io_tx_param(ph->io, off ? LCD_CMD_DISPOFF : LCD_CMD_DISPON, NULL, 0)) != ESP_OK)
    {
        log_e("Sending LCD_CMD_DISPOFF/LCD_CMD_DISPON failed");
        return res;
    }

    return ESP_OK;
}

esp_err_t gc9a01_del(esp_lcd_panel_t *panel)
{
    log_v("panel:0x%08x", panel);
    if (panel == NULL)
        return ESP_ERR_INVALID_ARG;

    gc9a01_panel_t *ph = (gc9a01_panel_t *)panel;

    // Reset RESET
    if (ph->panel_dev_config.reset_gpio_num != GPIO_NUM_NC)
        gpio_reset_pin(ph->panel_dev_config.reset_gpio_num);

    free(ph);

    return ESP_OK;
}

esp_err_t esp_lcd_new_panel_gc9a01(const esp_lcd_panel_io_handle_t panel_io_handle, const esp_lcd_panel_dev_config_t *panel_dev_config, esp_lcd_panel_handle_t *panel_handle)
{
    log_v("panel_io_handle:0x%08x, panel_dev_config:0x%08x, panel_handle:0x%08x", panel_io_handle, panel_dev_config, panel_handle);
    if (panel_io_handle == NULL || panel_dev_config == NULL || panel_handle == NULL)
        return ESP_ERR_INVALID_ARG;

    if (panel_dev_config->reset_gpio_num != GPIO_NUM_NC && !GPIO_IS_VALID_GPIO(panel_dev_config->reset_gpio_num))
    {
        log_e("Invalid GPIO RST pin: %d", panel_dev_config->reset_gpio_num);
        return ESP_ERR_INVALID_ARG;
    }

    uint8_t madctl;
    switch (panel_dev_config->color_space)
    {
    case ESP_LCD_COLOR_SPACE_RGB:
        madctl = 0;
        break;
    case ESP_LCD_COLOR_SPACE_BGR:
        madctl = LCD_CMD_BGR_BIT;
        break;
    default:
        log_e("Invalid color space: %d. Only RGB and BGR are supported", panel_dev_config->color_space);
        return ESP_ERR_INVALID_ARG;
    }

    if (panel_dev_config->reset_gpio_num != GPIO_NUM_NC)
    {
        esp_err_t res;
        const gpio_config_t cfg = {
            .pin_bit_mask = BIT64(panel_dev_config->reset_gpio_num),
            .mode = GPIO_MODE_OUTPUT};
        if ((res = gpio_config(&cfg)) != ESP_OK)
        {
            log_e("Configuring GPIO for RST failed");
            return res;
        }
    }

    gc9a01_panel_t *ph = heap_caps_calloc(1, sizeof(gc9a01_panel_t), MALLOC_CAP_DEFAULT);
    if (ph == NULL)
    {
        log_e("No memory available for gc9a01_panel_t");
        return ESP_ERR_NO_MEM;
    }

    ph->io = panel_io_handle;
    memcpy(&ph->panel_dev_config, panel_dev_config, sizeof(esp_lcd_panel_dev_config_t));
    ph->madctl = madctl;

    ph->base.del = gc9a01_del;
    ph->base.reset = gc9a01_reset;
    ph->base.init = gc9a01_init;
    ph->base.draw_bitmap = gc9a01_draw_bitmap;
    ph->base.invert_color = gc9a01_invert_color;
    ph->base.mirror = gc9a01_mirror;
    ph->base.swap_xy = gc9a01_swap_xy;
    ph->base.set_gap = gc9a01_set_gap;
    ph->base.disp_off = gc9a01_disp_off;

    log_d("panel_handle: 0x%08x", ph);
    *panel_handle = (esp_lcd_panel_handle_t)ph;

    return ESP_OK;
}

#endif