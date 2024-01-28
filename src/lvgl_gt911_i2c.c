#ifdef TOUCH_GT911_I2C

#include <esp32_smartdisplay.h>
#include "driver/i2c.h"
#include "esp_lcd_touch.h"
#include "esp_lcd_touch_gt911.h"

// The driver should take care of setting the config, but does not.

struct __attribute__((packed)) GTInfo
{
    char productId[4];    // 0x8140 - 0x8143
    uint16_t fwId;        // 0x8144 - 0x8145
    uint16_t xResolution; // 0x8146 - 0x8147
    uint16_t yResolution; // 0x8148 - 0x8149
    uint8_t vendorId;     // 0x814A
};

struct __attribute__((packed)) GTPoint
{
    // 0x814F-0x8156, ... 0x8176 (5 points)
    uint8_t trackId;
    uint16_t x;
    uint16_t y;
    uint16_t area;
    uint8_t reserved;
};

struct __attribute__((packed)) GTConfig
{
    uint8_t configVersion;        // 0x8047
    uint16_t xResolution;         // 0x8048 - 0x8049
    uint16_t yResolution;         // 0x804A - 0x804B
    uint8_t touchNumber;          // 0x804C
    uint8_t moduleSwitch1;        // 0x804D
    uint8_t moduleSwitch2;        // 0x804E
    uint8_t shakeCount;           // 0x804F
    uint8_t filter;               // 0x8050
    uint8_t largeTouch;           // 0x8051
    uint8_t noiseReduction;       // 0x8052
    uint8_t screenTouchLevel;     // 0x8053
    uint8_t screenLeaveLevel;     // 0x8054
    uint8_t lowPowerControl;      // 0x8055
    uint8_t refreshRate;          // 0x8056
    uint8_t xThreshold;           // 0x8057
    uint8_t yThreshold;           // 0x8058
    uint8_t xSpeedLimit;          // 0x8059 - reserved
    uint8_t ySpeedLimit;          // 0x805A - reserved
    uint8_t vSpace;               // 0x805B
    uint8_t hSpace;               // 0x805C
    uint8_t miniFilter;           // 0x805D
    uint8_t stretchR0;            // 0x805E
    uint8_t stretchR1;            // 0x805F
    uint8_t stretchR2;            // 0x8060
    uint8_t stretchRM;            // 0x8061
    uint8_t drvGroupANum;         // 0x8062
    uint8_t drvGroupBNum;         // 0x8063
    uint8_t sensorNum;            // 0x8064
    uint8_t freqAFactor;          // 0x8065
    uint8_t freqBFactor;          // 0x8066
    uint16_t pannelBitFreq;       // 0x8067 - 0x8068
    uint16_t pannelSensorTime;    // 0x8069 - 0x806A
    uint8_t pannelTxGain;         // 0x806B
    uint8_t pannelRxGain;         // 0x806C
    uint8_t pannelDumpShift;      // 0x806D
    uint8_t drvFrameControl;      // 0x806E
    uint8_t chargingLevelUp;      // 0x806F
    uint8_t moduleSwitch3;        // 0x8070
    uint8_t gestureDis;           // 0x8071
    uint8_t gestureLongPressTime; // 0x8072
    uint8_t xySlopeAdjust;        // 0x8073
    uint8_t gestureControl;       // 0x8074
    uint8_t gestureSwitch1;       // 0x8075
    uint8_t gestureSwitch2;       // 0x8076
    uint8_t gestureRefreshRate;   // 0x8077
    uint8_t gestureTouchLevel;    // 0x8078
    uint8_t newGreenWakeUpLevel;  // 0x8079
    uint8_t freqHoppingStart;     // 0x807A
    uint8_t freqHoppingEnd;       // 0x807B
    uint8_t noiseDetectTimes;     // 0x807C
    uint8_t hoppingFlag;          // 0x807D
    uint8_t hoppingThreshold;     // 0x807E
    uint8_t noiseThreshold;       // 0x807F
    uint8_t noiseMinThreshold;    // 0x8080
    uint8_t NC_1;                 // 0x8081
    uint8_t hoppingSensorGroup;   // 0x8082
    uint8_t hoppingSeg1Normalize; // 0x8083
    uint8_t hoppingSeg1Factor;    // 0x8084
    uint8_t mainClockAjdust;      // 0x8085
    uint8_t hoppingSeg2Normalize; // 0x8086
    uint8_t hoppingSeg2Factor;    // 0x8087
    uint8_t NC_2;                 // 0x8088
    uint8_t hoppingSeg3Normalize; // 0x8089
    uint8_t hoppingSeg3Factor;    // 0x808A
    uint8_t NC_3;                 // 0x808B
    uint8_t hoppingSeg4Normalize; // 0x808C
    uint8_t hoppingSeg4Factor;    // 0x808D
    uint8_t NC_4;                 // 0x808E
    uint8_t hoppingSeg5Normalize; // 0x808F
    uint8_t hoppingSeg5Factor;    // 0x8090
    uint8_t NC_5;                 // 0x8091
    uint8_t hoppingSeg6Normalize; // 0x8092
    uint8_t key[4];               // 0x8093 - 0x8096
    uint8_t keyArea;              // 0x8097
    uint8_t keyTouchLevel;        // 0x8098
    uint8_t keyLeaveLevel;        // 0x8099
    uint8_t keySens[2];           // 0x809A - 0x809B
    uint8_t keyRestrain;          // 0x809C
    uint8_t keyRestrainTime;      // 0x809D
    uint8_t gestureLargeTouch;    // 0x809E
    uint8_t NC_6[2];              // 0x809F - 0x80A0
    uint8_t hotknotNoiseMap;      // 0x80A1
    uint8_t linkThreshold;        // 0x80A2
    uint8_t pxyThreshold;         // 0x80A3
    uint8_t gHotDumpShift;        // 0x80A4
    uint8_t gHotRxGain;           // 0x80A5
    uint8_t freqGain[4];          // 0x80A6 - 0x80A9
    uint8_t NC_7[9];              // 0x80AA - 0x80B2
    uint8_t combineDis;           // 0x80B3
    uint8_t splitSet;             // 0x80B4
    uint8_t NC_8[2];              // 0x80B5 - 0x80B6
    uint8_t sensorCH[14];         // 0x80B7 - 0x80C4
    uint8_t NC_9[16];             // 0x80C5 - 0x80D4
    uint8_t driverCH[26];         // 0x80D5 - 0x80EE
    uint8_t NC_10[16];            // 0x80EF - 0x80FE
};

struct GTInfo gt_info;

static void gt911_lvgl_touch_cb(lv_indev_drv_t *drv, lv_indev_data_t *data)
{
    esp_lcd_touch_handle_t touch_handle = drv->user_data;

    uint16_t touch_x[1] = {0};
    uint16_t touch_y[1] = {0};
    uint16_t touch_strength[1] = {0};
    uint8_t touch_cnt = 0;

    // Read touch controller data
    ESP_ERROR_CHECK(esp_lcd_touch_read_data(touch_handle));
    // Get coordinates
    bool pressed = esp_lcd_touch_get_coordinates(touch_handle, touch_x, touch_y, touch_strength, &touch_cnt, 1);
    if (pressed && touch_cnt > 0)
    {
        data->point.x = (touch_x[0] * GT911_TOUCH_CONFIG_X_MAX) / gt_info.xResolution;
        data->point.y = (touch_y[0] * GT911_TOUCH_CONFIG_Y_MAX) / gt_info.yResolution;
        data->state = LV_INDEV_STATE_PRESSED;
        log_d("Pressed at: (%d,%d), strength: %d", data->point.x, data->point.y, touch_strength);
    }
    else
        data->state = LV_INDEV_STATE_RELEASED;
}

/*
uint8_t calculate_checksum_8(uint8_t *buffer, ushort length)
{
    uint8_t checksum = 0;
    while (length--)
        checksum += *buffer++;

    return ~checksum + 1;
}
*/

void lvgl_touch_init(lv_indev_drv_t *drv)
{
    log_d("lvgl_touch_init");
    // Create I2C bus
    const i2c_config_t i2c_config = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = GT911_I2C_CONFIG_SDA_IO_NUM,
        .scl_io_num = GT911_I2C_CONFIG_SCL_IO_NUM,
        .sda_pullup_en = GT911_I2C_CONFIG_SDA_PULLUP_EN,
        .scl_pullup_en = GT911_I2C_CONFIG_SCL_PULLUP_EN,
        .master = {
            .clk_speed = GT911_I2C_CONFIG_MASTER_CLK_SPEED},
        .clk_flags = GT911_I2C_CONFIG_CLK_FLAGS};
    ESP_ERROR_CHECK(i2c_param_config(GT911_I2C_HOST, &i2c_config));
    log_d("i2c_param_config. host: %d", GT911_I2C_HOST);
    ESP_ERROR_CHECK(i2c_driver_install(GT911_I2C_HOST, i2c_config.mode, 0, 0, 0));
    log_d("i2c_driver_install host: %d", GT911_I2C_HOST);

    // Create IO handle
    const esp_lcd_panel_io_i2c_config_t io_i2c_config = {
        .dev_addr = GT911_IO_I2C_CONFIG_DEV_ADDR,
        .control_phase_bytes = GT911_IO_I2C_CONFIG_CONTROL_PHASE_BYTES,
        .user_ctx = drv,
        .dc_bit_offset = GT911_IO_I2C_CONFIG_DC_BIT_OFFSET,
        .lcd_cmd_bits = GT911_IO_I2C_CONFIG_LCD_CMD_BITS,
        .lcd_param_bits = GT911_IO_I2C_CONFIG_LCD_PARAM_BITS,
        .flags = {
            .dc_low_on_data = GT911_IO_I2C_CONFIG_FLAGS_DC_LOW_ON_DATA,
            .disable_control_phase = GT911_IO_I2C_CONFIG_FLAGS_DISABLE_CONTROL_PHASE}};

    esp_lcd_panel_io_handle_t io_handle;
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_i2c((esp_lcd_i2c_bus_handle_t)GT911_I2C_HOST, &io_i2c_config, &io_handle));
    log_d("esp_lcd_new_panel_io_i2c. host: %d", GT911_I2C_HOST);

    // Read the information of the GT911
    ESP_ERROR_CHECK(esp_lcd_panel_io_rx_param(io_handle, 0x8140, &gt_info, sizeof(struct GTInfo)));
    log_d("GT911 productId: %s", gt_info.productId);                                            // 0x8140 - 0x8143
    log_d("GT911 fwId: %04x", gt_info.fwId);                                                    // 0x8144 - 0x8145
    log_d("GT911 xResolution/yResolution: (%d, %d)", gt_info.xResolution, gt_info.yResolution); // 0x8146 - 0x8147 // 0x8148 - 0x8149
    log_d("GT911 vendorId: %02x", gt_info.vendorId);                                                // 0x814A
    /*
    if (gt_info.xResolution != GT911_TOUCH_CONFIG_X_MAX || gt_info.yResolution != GT911_TOUCH_CONFIG_Y_MAX)
    {
        log_w("Resolution does not match configuration (%d,%d)", GT911_TOUCH_CONFIG_X_MAX, GT911_TOUCH_CONFIG_Y_MAX);
        struct GTConfig gt_config;
        log_i("GTConfig size: %d", sizeof(struct GTConfig));
        ESP_ERROR_CHECK(esp_lcd_panel_io_rx_param(io_handle, 0x8047, &gt_config, sizeof(struct GTConfig)));
        log_i("GTConfig version: %d", gt_config.configVersion);
        uint8_t checksum;
        ESP_ERROR_CHECK(esp_lcd_panel_io_rx_param(io_handle, 0x80FF, &checksum, sizeof(uint8_t)));
        if (checksum == calculate_checksum_8((uint8_t *)&gt_config, sizeof(struct GTConfig)))
        {
            log_i("Checksum OK. Updating configuration in GT911");
            gt_config.xResolution = GT911_TOUCH_CONFIG_X_MAX;
            gt_config.yResolution = GT911_TOUCH_CONFIG_Y_MAX;
            // Write new configuration
            ESP_ERROR_CHECK(esp_lcd_panel_io_tx_param(io_handle, 0x8047, &gt_config, sizeof(struct GTConfig)));
            checksum = calculate_checksum_8((uint8_t *)&gt_config, sizeof(struct GTConfig));
            //  Write checksum and set the "config_fresh" bit to 1
            uint8_t checksum_buffer[2] = {checksum, 1};
            ESP_ERROR_CHECK(esp_lcd_panel_io_tx_param(io_handle, 0x80FF, &checksum_buffer, sizeof(checksum_buffer)));
            log_w("Configuration in GT911 updated");
        }
        else
            log_e("Checksum failed. Will not touch configuration in GT911");
    }
    */

    // Create touch configuration
    const esp_lcd_touch_config_t touch_config = {
        .x_max = GT911_TOUCH_CONFIG_X_MAX,
        .y_max = GT911_TOUCH_CONFIG_Y_MAX,
        .rst_gpio_num = GT911_TOUCH_CONFIG_RST_GPIO_NUM,
        .int_gpio_num = GT911_TOUCH_CONFIG_INT_GPIO_NUM,
        .levels = {
            .reset = GT911_TOUCH_CONFIG_LEVELS_RESET,
            .interrupt = GT911_TOUCH_CONFIG_LEVELS_INTERRUPT},
        // Unfortunately not supported
        //.flags = {.swap_xy = GT911_TOUCH_CONFIG_FLAGS_SWAP_XY, .mirror_x = GT911_TOUCH_CONFIG_FLAGS_MIRROR_X, .mirror_y = GT911_TOUCH_CONFIG_FLAGS_MIRROR_Y},
        .user_data = io_handle};

    esp_lcd_touch_handle_t touch_handle;
    ESP_ERROR_CHECK(esp_lcd_touch_new_i2c_gt911(io_handle, &touch_config, &touch_handle));
    log_d("esp_lcd_touch_new_i2c_gt911. host: %d", GT911_I2C_HOST);

    drv->type = LV_INDEV_TYPE_POINTER;
    drv->user_data = touch_handle;
    drv->read_cb = gt911_lvgl_touch_cb;
}

#endif