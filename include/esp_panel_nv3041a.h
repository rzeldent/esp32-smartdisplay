#pragma once

#include <esp_lcd.h>
#include <esp_lcd_types.h>
#include <esp_lcd_panel_vendor.h>
#include <driver/spi_master.h>
#include <hal/gpio_types.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct
    {
        const lcd_init_cmd_t *init_cmds;
        uint16_t init_cmds_size;
    } nv3041a_vendor_config_t;

    // Uses spi_device_handle_t directly and manual GPIO CS to support QSPI (SPI_TRANS_MODE_QIO)
    // pixel data transfers, which the NV3041A requires but esp_lcd_panel_io_spi cannot provide
    // in ESP-IDF v4.4.  cs_gpio is the GPIO number used as chip-select (spics_io_num must be -1).
    esp_err_t esp_lcd_new_panel_nv3041a(spi_device_handle_t spi_dev, gpio_num_t cs_gpio,
                                        const esp_lcd_panel_dev_config_t *config, esp_lcd_panel_handle_t *handle);

#ifdef __cplusplus
}
#endif
