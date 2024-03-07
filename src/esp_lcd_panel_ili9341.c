#ifdef LCD_ILI9341_SPI

#include <esp_lcd_panel_ili9341.h>
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
} ili9341_panel_t;

const lcd_init_cmd_t ili9341_vendor_specific_init_default[] = {
    / Power contorl B, power control = 0, DC_ENA = 1
    {0xCF, (uint8_t[]){0x00, 0xAA, 0XE0}, 3, 0},
    /* Power on sequence control,
     * cp1 keeps 1 frame, 1st frame enable
     * vcl = 0, ddvdh=3, vgh=1, vgl=2
     * DDVDH_ENH=1
     */
    {0xED, (uint8_t[]){0x67, 0x03, 0X12, 0X81}, 4, 0},
    /* Driver timing control A,
     * non-overlap=default +1
     * EQ=default - 1, CR=default
     * pre-charge=default - 1
     */
    {0xE8, (uint8_t[]){0x8A, 0x01, 0x78}, 3, 0},
    /* Power control A, Vcore=1.6V, DDVDH=5.6V */
    {0xCB, (uint8_t[]){0x39, 0x2C, 0x00, 0x34, 0x02}, 5, 0},
    /* Pump ratio control, DDVDH=2xVCl */
    {0xF7, (uint8_t[]){0x20}, 1, 0},

    {0xF7, (uint8_t[]){0x20}, 1, 0},
    /* Driver timing control, all=0 unit */
    {0xEA, (uint8_t[]){0x00, 0x00}, 2, 0},
    /* Power control 1, GVDD=4.75V */
    {0xC0, (uint8_t[]){0x23}, 1, 0},
    /* Power control 2, DDVDH=VCl*2, VGH=VCl*7, VGL=-VCl*3 */
    {0xC1, (uint8_t[]){0x11}, 1, 0},
    /* VCOM control 1, VCOMH=4.025V, VCOML=-0.950V */
    {0xC5, (uint8_t[]){0x43, 0x4C}, 2, 0},
    /* VCOM control 2, VCOMH=VMH-2, VCOML=VML-2 */
    {0xC7, (uint8_t[]){0xA0}, 1, 0},
    /* Frame rate control, f=fosc, 70Hz fps */
    {0xB1, (uint8_t[]){0x00, 0x1B}, 2, 0},
    /* Enable 3G, disabled */
    {0xF2, (uint8_t[]){0x00}, 1, 0},
    /* Gamma set, curve 1 */
    {0x26, (uint8_t[]){0x01}, 1, 0},
    /* Positive gamma correction */
    {0xE0, (uint8_t[]){0x1F, 0x36, 0x36, 0x3A, 0x0C, 0x05, 0x4F, 0X87, 0x3C, 0x08, 0x11, 0x35, 0x19, 0x13, 0x00}, 15, 0},
    /* Negative gamma correction */
    {0xE1, (uint8_t[]){0x00, 0x09, 0x09, 0x05, 0x13, 0x0A, 0x30, 0x78, 0x43, 0x07, 0x0E, 0x0A, 0x26, 0x2C, 0x1F}, 15, 0},
    /* Entry mode set, Low vol detect disabled, normal display */
    {0xB7, (uint8_t[]){0x07}, 1, 0},
    /* Display function control */
    {0xB6, (uint8_t[]){0x08, 0x82, 0x27}, 3, 0}};

esp_err_t ili9341_reset(esp_lcd_panel_t *panel)
{
    log_v("panel:0x%08x", panel);
    if (panel == NULL)
        return ESP_ERR_INVALID_ARG;

    const ili9341_panel_t *ph = (ili9341_panel_t *)panel;

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

    vTaskDelay(pdMS_TO_TICKS(120));

    return ESP_OK;
}

esp_err_t ili9341_init(esp_lcd_panel_t *panel)
{
    log_v("panel:0x%08x", panel);
    if (panel == NULL)
        return ESP_ERR_INVALID_ARG;

    const ili9341_panel_t *ph = (ili9341_panel_t *)panel;

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
        colmod = 0x55;
        break;
    case 18: // RGB666
        colmod = 0x66;
        break;
    default:
        log_e("Invalid bits per pixel: %d. Only RGB565 and RGB666 are supported", ph->config.bits_per_pixel);
        return ESP_ERR_INVALID_ARG;
    }

    if ((res = esp_lcd_panel_io_tx_param(ph->io, LCD_CMD_MADCTL, &ph->madctl, 1)) != ESP_OK ||
        (res = esp_lcd_panel_io_tx_param(ph->io, LCD_CMD_COLMOD, &colmod, 1)) != ESP_OK)
    {
        log_e("Sending MADCTL/COLMOD failed");
        return res;
    }

    const lcd_init_cmd_t *cmd = ili9341_vendor_specific_init_default;
    uint16_t cmds_size = sizeof(ili9341_vendor_specific_init_default) / sizeof(lcd_init_cmd_t);
    if (ph->config.vendor_config != NULL)
    {
        cmd = ((ili9341_vendor_config_t *)ph->config.vendor_config)->init_cmds;
        cmds_size = ((ili9341_vendor_config_t *)ph->config.vendor_config)->init_cmds_size;
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

esp_err_t ili9341_draw_bitmap(esp_lcd_panel_t *panel, int x_start, int y_start, int x_end, int y_end, const void *color_data)
{
    log_v("panel:0x%08x, x_start:%d, y_start:%d, x_end:%d, y_end:%d, color_data:0x%08x", panel, x_start, y_start, x_end, y_end, color_data);
    if (panel == NULL || color_data == NULL)
        return ESP_ERR_INVALID_ARG;

    const ili9341_panel_t *ph = (ili9341_panel_t *)panel;

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

    uint8_t bytes_per_pixel = (ph->config.bits_per_pixel + 0x7) >> 3;
    size_t len = (x_end - x_start) * (y_end - y_start) * bytes_per_pixel;
    if ((res = esp_lcd_panel_io_tx_color(ph->io, LCD_CMD_RAMWR, color_data, len)) != ESP_OK)
    {
        log_e("Sending RAMWR failed");
        return res;
    }

    return ESP_OK;
}

esp_err_t ili9341_invert_color(esp_lcd_panel_t *panel, bool invert)
{
    log_v("panel:0x%08x, invert:%d", panel, invert);
    if (panel == NULL)
        return ESP_ERR_INVALID_ARG;

    const ili9341_panel_t *ph = (ili9341_panel_t *)panel;

    esp_err_t res;
    if ((res = esp_lcd_panel_io_tx_param(ph->io, invert ? LCD_CMD_INVON : LCD_CMD_INVOFF, NULL, 0)) != ESP_OK)
    {
        log_e("Sending LCD_CMD_INVON/LCD_CMD_INVOFF failed");
        return res;
    }

    return ESP_OK;
}

esp_err_t ili9341_update_madctl(ili9341_panel_t *ph)
{
    esp_err_t res;
    if ((res = esp_lcd_panel_io_tx_param(ph->io, LCD_CMD_MADCTL, &ph->madctl, 1)) != ESP_OK)
    {
        log_e("Sending LCD_CMD_MADCTL failed");
        return res;
    }

    return ESP_OK;
}

esp_err_t ili9341_mirror(esp_lcd_panel_t *panel, bool mirror_x, bool mirror_y)
{
    log_v("panel:0x%08x, mirror_x:%d, mirror_y:%d", panel, mirror_x, mirror_y);
    if (panel == NULL)
        return ESP_ERR_INVALID_ARG;

    ili9341_panel_t *ph = (ili9341_panel_t *)panel;

    if (mirror_x)
        ph->madctl |= LCD_CMD_MX_BIT;
    else
        ph->madctl &= ~LCD_CMD_MX_BIT;

    if (mirror_y)
        ph->madctl |= LCD_CMD_MY_BIT;
    else
        ph->madctl &= ~LCD_CMD_MY_BIT;

    return ili9341_update_madctl(ph);
}

esp_err_t ili9341_swap_xy(esp_lcd_panel_t *panel, bool swap_xy)
{
    log_v("panel:0x%08x, swap_xy:%d", panel, swap_xy);
    if (panel == NULL)
        return ESP_ERR_INVALID_ARG;

    ili9341_panel_t *ph = (ili9341_panel_t *)panel;

    if (swap_xy)
        ph->madctl |= LCD_CMD_MV_BIT;
    else
        ph->madctl &= ~LCD_CMD_MV_BIT;

    return ili9341_update_madctl(ph);
}

esp_err_t ili9341_set_gap(esp_lcd_panel_t *panel, int x_gap, int y_gap)
{
    log_v("panel:0x%08x, x_gap:%d, y_gap:%d", panel, x_gap, y_gap);
    if (panel == NULL)
        return ESP_ERR_INVALID_ARG;

    ili9341_panel_t *ph = (ili9341_panel_t *)panel;

    ph->x_gap = x_gap;
    ph->y_gap = y_gap;

    return ESP_OK;
}

esp_err_t ili9341_disp_off(esp_lcd_panel_t *panel, bool off)
{
    log_v("panel:0x%08x, off:%d", panel, off);
    if (panel == NULL)
        return ESP_ERR_INVALID_ARG;

    const ili9341_panel_t *ph = (ili9341_panel_t *)panel;

    esp_err_t res;
    if ((res = esp_lcd_panel_io_tx_param(ph->io, off ? LCD_CMD_DISPOFF : LCD_CMD_DISPON, NULL, 0)) != ESP_OK)
    {
        log_e("Sending LCD_CMD_DISPOFF/LCD_CMD_DISPON failed");
        return res;
    }

    return ESP_OK;
}

esp_err_t ili9341_del(esp_lcd_panel_t *panel)
{
    log_v("panel:0x%08x", panel);
    if (panel == NULL)
        return ESP_ERR_INVALID_ARG;

    ili9341_panel_t *ph = (ili9341_panel_t *)panel;

    // Reset RESET
    if (ph->config.reset_gpio_num != GPIO_NUM_NC)
        gpio_reset_pin(ph->config.reset_gpio_num);

    free(ph);

    return ESP_OK;
}

esp_err_t esp_lcd_new_panel_ili9341(const esp_lcd_panel_io_handle_t io, const esp_lcd_panel_dev_config_t *config, esp_lcd_panel_handle_t *handle)
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

    ili9341_panel_t *ph = heap_caps_calloc(1, sizeof(ili9341_panel_t), MALLOC_CAP_DEFAULT);
    if (ph == NULL)
    {
        log_e("No memory available for ili9341_panel_t");
        return ESP_ERR_NO_MEM;
    }

    ph->io = io;
    memcpy(&ph->config, config, sizeof(esp_lcd_panel_dev_config_t));
    ph->madctl = madctl;

    ph->base.del = ili9341_del;
    ph->base.reset = ili9341_reset;
    ph->base.init = ili9341_init;
    ph->base.draw_bitmap = ili9341_draw_bitmap;
    ph->base.invert_color = ili9341_invert_color;
    ph->base.mirror = ili9341_mirror;
    ph->base.swap_xy = ili9341_swap_xy;
    ph->base.set_gap = ili9341_set_gap;
    ph->base.disp_off = ili9341_disp_off;

    log_d("handle: 0x%08x", ph);
    *handle = (esp_lcd_panel_handle_t)ph;

    return ESP_OK;
}

#endif