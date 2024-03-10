#pragma once

#include <stdbool.h>
#include "sdkconfig.h"
#include "esp_err.h"
#include "driver/gpio.h"
#include "esp_lcd_panel_io.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

typedef struct
{
    uint8_t cmd;             // Command
    const uint8_t *data;     // Buffer to data for the command
    uint8_t bytes;           // Size of the data buffer for the command
    unsigned short delay_ms; // Delay in milliseconds after the command
} lcd_init_cmd_t;
