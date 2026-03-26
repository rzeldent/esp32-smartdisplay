#ifdef DISPLAY_NV3041A_QSPI

#include <esp_panel_nv3041a.h>
#include <esp32-hal-log.h>
#include <esp_rom_gpio.h>
#include <esp_heap_caps.h>
#include <memory.h>
#include <driver/gpio.h>
#include <esp_lcd_panel_commands.h>
#include <esp_lcd_types.h>
#include <esp_lcd_panel_interface.h>

// NV3041A QSPI opcodes
// 0x02 = single-line register write  (cmd:8-bit, addr:24-bit, data:N bytes, all 1-bit SPI)
// 0x32 = quad pixel write            (cmd:8-bit, addr:24-bit, data:N bytes, data phase in 4-bit QIO)
#define NV3041A_OP_REG_WRITE  0x02
#define NV3041A_OP_QUAD_WRITE 0x32
// RAMWRC register address used with 0x32 opcode (continue pixel stream)
#define NV3041A_ADDR_RAMWRC   0x003C00

// Maximum pixel data per SPI transaction (matches reference Arduino_GFX driver)
#define NV3041A_MAX_PIXELS_PER_TRANS 1024  // 2048 bytes for RGB565

#define NV3041A_CS_LOW(pin)  gpio_set_level((gpio_num_t)(pin), 0)
#define NV3041A_CS_HIGH(pin) gpio_set_level((gpio_num_t)(pin), 1)

typedef struct
{
    esp_lcd_panel_t base;
    spi_device_handle_t spi_dev;
    gpio_num_t cs_gpio;
    esp_lcd_panel_dev_config_t panel_dev_config;
    // Data
    int x_gap;
    int y_gap;
    uint8_t madctl;
} nv3041a_panel_t;

// Send a register write command via NV3041A QSPI 0x02 protocol.
// SPI device must have command_bits=8 and address_bits=24.
// CS is toggled manually because spics_io_num=-1.
static esp_err_t nv3041a_send_cmd(spi_device_handle_t spi_dev, gpio_num_t cs_gpio,
                                   uint8_t lcd_cmd, const uint8_t *data, size_t len)
{
    spi_transaction_t t = {
        .cmd  = NV3041A_OP_REG_WRITE,
        .addr = ((uint32_t)lcd_cmd) << 8,   // register in bits [23:8]
        .flags = SPI_TRANS_MULTILINE_CMD | SPI_TRANS_MULTILINE_ADDR,
    };
    if (len > 0 && data != NULL)
    {
        if (len <= 4)
        {
            t.flags |= SPI_TRANS_USE_TXDATA;
            t.length = len * 8;
            memcpy(t.tx_data, data, len);
        }
        else
        {
            t.length = len * 8;
            t.tx_buffer = data;
        }
    }
    NV3041A_CS_LOW(cs_gpio);
    esp_err_t res = spi_device_polling_transmit(spi_dev, &t);
    NV3041A_CS_HIGH(cs_gpio);
    return res;
}

// NV3041A vendor-specific initialization sequence.
// Ported from Arduino_GFX Arduino_NV3041A driver (JC4827W543 reference board).
// Registers are programmed in vendor-unlock mode (0xff=0xa5 ... 0xff=0x00).
const lcd_init_cmd_t nv3041a_vendor_specific_init_default[] = {
    // Enter vendor command mode
    {0xff, (const uint8_t[]){0xa5}, 1, 0},
    // QSPI interface pixel format: 0x01=16-bit RGB565, 0x00=18-bit RGB666
    {0x3a, (const uint8_t[]){0x01}, 1, 0},
    // Interface mode: 16-bit (03), 8-bit (01)
    {0x41, (const uint8_t[]){0x03}, 1, 0},
    // VBP
    {0x44, (const uint8_t[]){0x15}, 1, 0},
    // VFP
    {0x45, (const uint8_t[]){0x15}, 1, 0},
    // vdds_trim
    {0x7d, (const uint8_t[]){0x03}, 1, 0},
    // Power supply settings
    {0xc1, (const uint8_t[]){0xab}, 1, 0},
    {0xc2, (const uint8_t[]){0x17}, 1, 0},
    {0xc3, (const uint8_t[]){0x10}, 1, 0},
    {0xc6, (const uint8_t[]){0x3a}, 1, 0},
    {0xc7, (const uint8_t[]){0x25}, 1, 0},
    {0xc8, (const uint8_t[]){0x11}, 1, 0},
    // user_vgsp
    {0x7a, (const uint8_t[]){0x49}, 1, 0},
    // user_gvdd
    {0x6f, (const uint8_t[]){0x2f}, 1, 0},
    // user_gvcl
    {0x78, (const uint8_t[]){0x4b}, 1, 0},
    {0xc9, (const uint8_t[]){0x00}, 1, 0},
    {0x67, (const uint8_t[]){0x33}, 1, 0},
    // Gate timing
    {0x51, (const uint8_t[]){0x4b}, 1, 0},
    {0x52, (const uint8_t[]){0x7c}, 1, 0},
    {0x53, (const uint8_t[]){0x1c}, 1, 0},
    {0x54, (const uint8_t[]){0x77}, 1, 0},
    // Source timing
    {0x46, (const uint8_t[]){0x0a}, 1, 0},
    {0x47, (const uint8_t[]){0x2a}, 1, 0},
    {0x48, (const uint8_t[]){0x0a}, 1, 0},
    {0x49, (const uint8_t[]){0x1a}, 1, 0},
    {0x56, (const uint8_t[]){0x43}, 1, 0},
    {0x57, (const uint8_t[]){0x42}, 1, 0},
    {0x58, (const uint8_t[]){0x3c}, 1, 0},
    {0x59, (const uint8_t[]){0x64}, 1, 0},
    {0x5a, (const uint8_t[]){0x41}, 1, 0},
    {0x5b, (const uint8_t[]){0x3c}, 1, 0},
    {0x5c, (const uint8_t[]){0x02}, 1, 0},
    {0x5d, (const uint8_t[]){0x3c}, 1, 0},
    {0x5e, (const uint8_t[]){0x1f}, 1, 0},
    {0x60, (const uint8_t[]){0x80}, 1, 0},
    {0x61, (const uint8_t[]){0x3f}, 1, 0},
    {0x62, (const uint8_t[]){0x21}, 1, 0},
    {0x63, (const uint8_t[]){0x07}, 1, 0},
    {0x64, (const uint8_t[]){0xe0}, 1, 0},
    {0x65, (const uint8_t[]){0x01}, 1, 0},
    // AVDD mux timing
    {0xca, (const uint8_t[]){0x20}, 1, 0},
    {0xcb, (const uint8_t[]){0x52}, 1, 0},
    {0xcc, (const uint8_t[]){0x10}, 1, 0},
    {0xcd, (const uint8_t[]){0x42}, 1, 0},
    // AVCL mux timing
    {0xd0, (const uint8_t[]){0x20}, 1, 0},
    {0xd1, (const uint8_t[]){0x52}, 1, 0},
    {0xd2, (const uint8_t[]){0x10}, 1, 0},
    {0xd3, (const uint8_t[]){0x42}, 1, 0},
    // VGH mux timing
    {0xd4, (const uint8_t[]){0x0a}, 1, 0},
    {0xd5, (const uint8_t[]){0x32}, 1, 0},
    // Gamma positive
    {0x80, (const uint8_t[]){0x04}, 1, 0},
    {0x81, (const uint8_t[]){0x07}, 1, 0},
    {0x82, (const uint8_t[]){0x06}, 1, 0},
    {0x83, (const uint8_t[]){0x39}, 1, 0},
    {0x84, (const uint8_t[]){0x3a}, 1, 0},
    {0x85, (const uint8_t[]){0x3f}, 1, 0},
    {0x86, (const uint8_t[]){0x2c}, 1, 0},
    {0x87, (const uint8_t[]){0x46}, 1, 0},
    {0x88, (const uint8_t[]){0x08}, 1, 0},
    {0x89, (const uint8_t[]){0x0f}, 1, 0},
    {0x8a, (const uint8_t[]){0x17}, 1, 0},
    {0x8b, (const uint8_t[]){0x10}, 1, 0},
    {0x8c, (const uint8_t[]){0x16}, 1, 0},
    {0x8d, (const uint8_t[]){0x14}, 1, 0},
    {0x8e, (const uint8_t[]){0x11}, 1, 0},
    {0x8f, (const uint8_t[]){0x14}, 1, 0},
    {0x90, (const uint8_t[]){0x06}, 1, 0},
    {0x91, (const uint8_t[]){0x0f}, 1, 0},
    {0x92, (const uint8_t[]){0x16}, 1, 0},
    // Gamma negative
    {0xa0, (const uint8_t[]){0x00}, 1, 0},
    {0xa1, (const uint8_t[]){0x05}, 1, 0},
    {0xa2, (const uint8_t[]){0x04}, 1, 0},
    {0xa3, (const uint8_t[]){0x39}, 1, 0},
    {0xa4, (const uint8_t[]){0x3a}, 1, 0},
    {0xa5, (const uint8_t[]){0x3f}, 1, 0},
    {0xa6, (const uint8_t[]){0x2a}, 1, 0},
    {0xa7, (const uint8_t[]){0x44}, 1, 0},
    {0xa8, (const uint8_t[]){0x08}, 1, 0},
    {0xa9, (const uint8_t[]){0x0f}, 1, 0},
    {0xaa, (const uint8_t[]){0x17}, 1, 0},
    {0xab, (const uint8_t[]){0x10}, 1, 0},
    {0xac, (const uint8_t[]){0x16}, 1, 0},
    {0xad, (const uint8_t[]){0x14}, 1, 0},
    {0xae, (const uint8_t[]){0x11}, 1, 0},
    {0xaf, (const uint8_t[]){0x14}, 1, 0},
    {0xb0, (const uint8_t[]){0x06}, 1, 0},
    {0xb1, (const uint8_t[]){0x0f}, 1, 0},
    {0xb2, (const uint8_t[]){0x16}, 1, 0},
    // Exit vendor command mode
    {0xff, (const uint8_t[]){0x00}, 1, 0},
};

esp_err_t nv3041a_reset(esp_lcd_panel_t *panel)
{
    log_v("panel:0x%08x", panel);
    if (panel == NULL)
        return ESP_ERR_INVALID_ARG;

    const nv3041a_panel_t *ph = (nv3041a_panel_t *)panel;

    if (ph->panel_dev_config.reset_gpio_num != GPIO_NUM_NC)
    {
        // Hardware reset
        gpio_set_level(ph->panel_dev_config.reset_gpio_num, ph->panel_dev_config.flags.reset_active_high);
        vTaskDelay(pdMS_TO_TICKS(10));
        gpio_set_level(ph->panel_dev_config.reset_gpio_num, !ph->panel_dev_config.flags.reset_active_high);
        vTaskDelay(pdMS_TO_TICKS(120));
        gpio_set_level(ph->panel_dev_config.reset_gpio_num, ph->panel_dev_config.flags.reset_active_high);
        vTaskDelay(pdMS_TO_TICKS(120));
    }
    else
    {
        // Software reset via 0x02 protocol
        esp_err_t res;
        if ((res = nv3041a_send_cmd(ph->spi_dev, ph->cs_gpio, LCD_CMD_SWRESET, NULL, 0)) != ESP_OK)
        {
            log_e("Sending LCD_CMD_SWRESET failed");
            return res;
        }
        vTaskDelay(pdMS_TO_TICKS(120));
    }

    return ESP_OK;
}

esp_err_t nv3041a_init(esp_lcd_panel_t *panel)
{
    log_v("panel:0x%08x", panel);
    if (panel == NULL)
        return ESP_ERR_INVALID_ARG;

    const nv3041a_panel_t *ph = (nv3041a_panel_t *)panel;

    esp_err_t res;
    // Sleep-out — NV3041A must be awake before further register writes
    if ((res = nv3041a_send_cmd(ph->spi_dev, ph->cs_gpio, LCD_CMD_SLPOUT, NULL, 0)) != ESP_OK)
    {
        log_e("Sending SLPOUT failed");
        return res;
    }
    vTaskDelay(pdMS_TO_TICKS(120));

    // Set MADCTL (memory access / color order)
    if ((res = nv3041a_send_cmd(ph->spi_dev, ph->cs_gpio, LCD_CMD_MADCTL, &ph->madctl, 1)) != ESP_OK)
    {
        log_e("Sending MADCTL failed");
        return res;
    }

    // Execute vendor-specific init (includes 0x3a pixel-format and gamma tables)
    const lcd_init_cmd_t *cmd = nv3041a_vendor_specific_init_default;
    uint16_t cmds_size = sizeof(nv3041a_vendor_specific_init_default) / sizeof(lcd_init_cmd_t);
    if (ph->panel_dev_config.vendor_config != NULL)
    {
        cmd = ((nv3041a_vendor_config_t *)ph->panel_dev_config.vendor_config)->init_cmds;
        cmds_size = ((nv3041a_vendor_config_t *)ph->panel_dev_config.vendor_config)->init_cmds_size;
    }

    while (cmds_size-- > 0)
    {
        if ((res = nv3041a_send_cmd(ph->spi_dev, ph->cs_gpio, cmd->cmd, cmd->data, cmd->bytes)) != ESP_OK)
        {
            log_e("Sending command: 0x%02x failed", cmd->cmd);
            return res;
        }

        if (cmd->delay_ms > 0)
            vTaskDelay(pdMS_TO_TICKS(cmd->delay_ms));
        cmd++;
    }

    return ESP_OK;
}

esp_err_t nv3041a_draw_bitmap(esp_lcd_panel_t *panel, int x_start, int y_start, int x_end, int y_end, const void *color_data)
{
    log_v("panel:0x%08x, x_start:%d, y_start:%d, x_end:%d, y_end:%d, color_data:0x%08x", panel, x_start, y_start, x_end, y_end, color_data);
    if (panel == NULL || color_data == NULL)
        return ESP_ERR_INVALID_ARG;

    const nv3041a_panel_t *ph = (nv3041a_panel_t *)panel;

    if (x_start >= x_end)
    {
        log_w("X-start >= x-end");
        return ESP_ERR_INVALID_ARG;
    }
    if (y_start >= y_end)
    {
        log_w("Y-start >= y-end");
        return ESP_ERR_INVALID_ARG;
    }

    // Apply gap offsets
    x_start += ph->x_gap;
    x_end   += ph->x_gap;
    y_start += ph->y_gap;
    y_end   += ph->y_gap;

    esp_err_t res;

    // CASET — column address set
    const uint8_t caset[4] = {x_start >> 8, x_start & 0xff, (x_end - 1) >> 8, (x_end - 1) & 0xff};
    if ((res = nv3041a_send_cmd(ph->spi_dev, ph->cs_gpio, LCD_CMD_CASET, caset, sizeof(caset))) != ESP_OK)
    {
        log_e("Sending CASET failed");
        return res;
    }

    // RASET — row address set
    const uint8_t raset[4] = {y_start >> 8, y_start & 0xff, (y_end - 1) >> 8, (y_end - 1) & 0xff};
    if ((res = nv3041a_send_cmd(ph->spi_dev, ph->cs_gpio, LCD_CMD_RASET, raset, sizeof(raset))) != ESP_OK)
    {
        log_e("Sending RASET failed");
        return res;
    }

    // RAMWR — start memory write (no data, just initiates the write cycle)
    if ((res = nv3041a_send_cmd(ph->spi_dev, ph->cs_gpio, LCD_CMD_RAMWR, NULL, 0)) != ESP_OK)
    {
        log_e("Sending RAMWR failed");
        return res;
    }

    // Pixel data — sent via 0x32 (quad-SPI write to RAMWRC=0x3C).
    // First transaction includes cmd=0x32 + addr=0x003C00 (1-bit) then data in 4-bit QIO.
    // Subsequent transactions skip cmd/addr (zero bits) and continue the data stream.
    // CS stays LOW for the entire pixel burst.
    size_t total_pixels = (x_end - x_start) * (y_end - y_start);
    const uint8_t *px = (const uint8_t *)color_data;
    bool first_chunk = true;

    NV3041A_CS_LOW(ph->cs_gpio);
    while (total_pixels > 0)
    {
        size_t chunk_pixels = (total_pixels < NV3041A_MAX_PIXELS_PER_TRANS) ? total_pixels : NV3041A_MAX_PIXELS_PER_TRANS;
        size_t chunk_bytes = chunk_pixels * 2; // RGB565 = 2 bytes/pixel

        if (first_chunk)
        {
            spi_transaction_t t = {
                .cmd    = NV3041A_OP_QUAD_WRITE,
                .addr   = NV3041A_ADDR_RAMWRC,
                .flags  = SPI_TRANS_MODE_QIO,
                .length = chunk_bytes * 8,
                .tx_buffer = px,
            };
            res = spi_device_polling_transmit(ph->spi_dev, &t);
            first_chunk = false;
        }
        else
        {
            // Continuation: skip cmd and addr phases entirely
            spi_transaction_ext_t t = {
                .command_bits = 0,
                .address_bits = 0,
                .dummy_bits   = 0,
                .base = {
                    .flags  = SPI_TRANS_MODE_QIO | SPI_TRANS_VARIABLE_CMD | SPI_TRANS_VARIABLE_ADDR | SPI_TRANS_VARIABLE_DUMMY,
                    .length = chunk_bytes * 8,
                    .tx_buffer = px,
                },
            };
            res = spi_device_polling_transmit(ph->spi_dev, (spi_transaction_t *)&t);
        }

        if (res != ESP_OK)
        {
            NV3041A_CS_HIGH(ph->cs_gpio);
            log_e("Pixel chunk transfer failed");
            return res;
        }

        px += chunk_bytes;
        total_pixels -= chunk_pixels;
    }
    NV3041A_CS_HIGH(ph->cs_gpio);

    return ESP_OK;
}

esp_err_t nv3041a_invert_color(esp_lcd_panel_t *panel, bool invert)
{
    log_v("panel:0x%08x, invert:%d", panel, invert);
    if (panel == NULL)
        return ESP_ERR_INVALID_ARG;

    const nv3041a_panel_t *ph = (nv3041a_panel_t *)panel;

    esp_err_t res;
    if ((res = nv3041a_send_cmd(ph->spi_dev, ph->cs_gpio, invert ? LCD_CMD_INVON : LCD_CMD_INVOFF, NULL, 0)) != ESP_OK)
    {
        log_e("Sending INVON/INVOFF failed");
        return res;
    }

    return ESP_OK;
}

esp_err_t nv3041a_update_madctl(nv3041a_panel_t *ph)
{
    esp_err_t res;
    if ((res = nv3041a_send_cmd(ph->spi_dev, ph->cs_gpio, LCD_CMD_MADCTL, &ph->madctl, 1)) != ESP_OK)
    {
        log_e("Sending MADCTL failed");
        return res;
    }
    return ESP_OK;
}

esp_err_t nv3041a_mirror(esp_lcd_panel_t *panel, bool mirror_x, bool mirror_y)
{
    log_v("panel:0x%08x, mirror_x:%d, mirror_y:%d", panel, mirror_x, mirror_y);
    if (panel == NULL)
        return ESP_ERR_INVALID_ARG;

    nv3041a_panel_t *ph = (nv3041a_panel_t *)panel;

    if (mirror_x)
        ph->madctl |= LCD_CMD_MX_BIT;
    else
        ph->madctl &= ~LCD_CMD_MX_BIT;

    if (mirror_y)
        ph->madctl |= LCD_CMD_MY_BIT;
    else
        ph->madctl &= ~LCD_CMD_MY_BIT;

    return nv3041a_update_madctl(ph);
}

esp_err_t nv3041a_swap_xy(esp_lcd_panel_t *panel, bool swap_xy)
{
    log_v("panel:0x%08x, swap_xy:%d", panel, swap_xy);
    if (panel == NULL)
        return ESP_ERR_INVALID_ARG;

    nv3041a_panel_t *ph = (nv3041a_panel_t *)panel;

    if (swap_xy)
        ph->madctl |= LCD_CMD_MV_BIT;
    else
        ph->madctl &= ~LCD_CMD_MV_BIT;

    return nv3041a_update_madctl(ph);
}

esp_err_t nv3041a_set_gap(esp_lcd_panel_t *panel, int x_gap, int y_gap)
{
    log_v("panel:0x%08x, x_gap:%d, y_gap:%d", panel, x_gap, y_gap);
    if (panel == NULL)
        return ESP_ERR_INVALID_ARG;

    nv3041a_panel_t *ph = (nv3041a_panel_t *)panel;
    ph->x_gap = x_gap;
    ph->y_gap = y_gap;

    return ESP_OK;
}

esp_err_t nv3041a_disp_off(esp_lcd_panel_t *panel, bool off)
{
    log_v("panel:0x%08x, off:%d", panel, off);
    if (panel == NULL)
        return ESP_ERR_INVALID_ARG;

    const nv3041a_panel_t *ph = (nv3041a_panel_t *)panel;

    esp_err_t res;
    if ((res = nv3041a_send_cmd(ph->spi_dev, ph->cs_gpio, off ? LCD_CMD_DISPOFF : LCD_CMD_DISPON, NULL, 0)) != ESP_OK)
    {
        log_e("Sending DISPOFF/DISPON failed");
        return res;
    }

    return ESP_OK;
}

esp_err_t nv3041a_del(esp_lcd_panel_t *panel)
{
    log_v("panel:0x%08x", panel);
    if (panel == NULL)
        return ESP_ERR_INVALID_ARG;

    nv3041a_panel_t *ph = (nv3041a_panel_t *)panel;

    if (ph->panel_dev_config.reset_gpio_num != GPIO_NUM_NC)
        gpio_reset_pin(ph->panel_dev_config.reset_gpio_num);

    gpio_reset_pin(ph->cs_gpio);
    spi_bus_remove_device(ph->spi_dev);
    free(ph);

    return ESP_OK;
}

esp_err_t esp_lcd_new_panel_nv3041a(spi_device_handle_t spi_dev, gpio_num_t cs_gpio,
                                     const esp_lcd_panel_dev_config_t *panel_dev_config,
                                     esp_lcd_panel_handle_t *panel_handle)
{
    log_v("spi_dev:0x%08x, cs_gpio:%d, panel_dev_config:0x%08x, panel_handle:0x%08x",
          spi_dev, cs_gpio, panel_dev_config, panel_handle);
    if (spi_dev == NULL || panel_dev_config == NULL || panel_handle == NULL)
        return ESP_ERR_INVALID_ARG;

    if (!GPIO_IS_VALID_OUTPUT_GPIO(cs_gpio))
    {
        log_e("Invalid GPIO CS pin: %d", cs_gpio);
        return ESP_ERR_INVALID_ARG;
    }

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

    // Configure CS GPIO as output, initially HIGH (deselected)
    {
        esp_err_t res;
        const gpio_config_t cs_cfg = {
            .pin_bit_mask = BIT64(cs_gpio),
            .mode = GPIO_MODE_OUTPUT,
        };
        if ((res = gpio_config(&cs_cfg)) != ESP_OK)
        {
            log_e("Configuring GPIO for CS failed");
            return res;
        }
        gpio_set_level(cs_gpio, 1);
    }

    if (panel_dev_config->reset_gpio_num != GPIO_NUM_NC)
    {
        esp_err_t res;
        const gpio_config_t rst_cfg = {
            .pin_bit_mask = BIT64(panel_dev_config->reset_gpio_num),
            .mode = GPIO_MODE_OUTPUT,
        };
        if ((res = gpio_config(&rst_cfg)) != ESP_OK)
        {
            log_e("Configuring GPIO for RST failed");
            return res;
        }
    }

    nv3041a_panel_t *ph = heap_caps_calloc(1, sizeof(nv3041a_panel_t), MALLOC_CAP_DEFAULT);
    if (ph == NULL)
    {
        log_e("No memory available for nv3041a_panel_t");
        return ESP_ERR_NO_MEM;
    }

    ph->spi_dev = spi_dev;
    ph->cs_gpio = cs_gpio;
    memcpy(&ph->panel_dev_config, panel_dev_config, sizeof(esp_lcd_panel_dev_config_t));
    ph->madctl = madctl;

    ph->base.del         = nv3041a_del;
    ph->base.reset       = nv3041a_reset;
    ph->base.init        = nv3041a_init;
    ph->base.draw_bitmap = nv3041a_draw_bitmap;
    ph->base.invert_color = nv3041a_invert_color;
    ph->base.mirror      = nv3041a_mirror;
    ph->base.swap_xy     = nv3041a_swap_xy;
    ph->base.set_gap     = nv3041a_set_gap;
    ph->base.disp_off    = nv3041a_disp_off;

    log_d("panel_handle: 0x%08x", ph);
    *panel_handle = (esp_lcd_panel_handle_t)ph;

    return ESP_OK;
}

#endif
