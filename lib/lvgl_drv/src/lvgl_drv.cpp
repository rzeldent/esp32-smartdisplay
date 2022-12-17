#include <Arduino.h>
#include <lvgl_drv.h>

// Functions to be defined in the tft driver
extern void lvgl_tft_init();
extern void lvgl_tft_flush(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_map);

// Functions to be defined in the touch driver
extern void lvgl_touch_init();
extern void lvgl_touch_read(lv_indev_drv_t *drv, lv_indev_data_t *data);

// Hardware interfaces
#ifdef ESP32_2432S028R
SPIClass spi_ili9431;
SPIClass spi_xpt2046;
#endif

#ifdef ESP32_3248S028R
SPIClass spi_st7796;
#endif

#ifdef ESP32_3248S028C
SPIClass spi_st7796;
TwoWire i2c_gt911 = TwoWire(1); // Bus number 1
#endif

#if LV_USE_LOG
void lvgl_log(const char *buf)
{
    log_printf("%s", buf);
}
#endif

void lvgl_init()
{
    // Setup RGB LED.  High is off
    // Use channel 0=R, 1=G, 2=B, 5kHz,  8 bit resolution
    pinMode(LED_PIN_R, OUTPUT);
    ledcSetup(0, 5000, 8);
    ledcAttachPin(LED_PIN_R, 0);
    digitalWrite(LED_PIN_R, true);

    pinMode(LED_PIN_G, OUTPUT);
    ledcSetup(1, 5000, 8);
    ledcAttachPin(LED_PIN_R, 1);
    digitalWrite(LED_PIN_G, true);

    pinMode(LED_PIN_B, OUTPUT);
    ledcSetup(2, 5000, 8);
    ledcAttachPin(LED_PIN_R, 2);
    digitalWrite(LED_PIN_B, true);

    // Setup CDS Light sensor
    analogSetAttenuation(ADC_0db); // 0dB(1.0x) 0~800mV
    pinMode(CDS_PIN, INPUT);

    // Audio
    pinMode(AUDIO_PIN, INPUT); // Set high impedance

#if LV_USE_LOG
    lv_log_register_print_cb(lvgl_log);
#endif
    lv_init();

// Setup interfaces
#ifdef ESP32_2432S028R
    spi_ili9431.begin(ILI9431_SPI_SCLK, ILI9431_SPI_MISO, ILI9431_SPI_MOSI);
    spi_xpt2046.begin(XPT2046_SPI_SCLK, XPT2046_SPI_MISO, XPT2046_SPI_MOSI);
#endif

#ifdef ESP32_3248S028R
    spi_st7796.begin(ST7796_SPI_SCLK, ST7796_SPI_MISO, ST7796_SPI_MOSI);
    // xpy2046 uses same SPI bus
#endif

#ifdef ESP32_3248S028C
    spi_st7796.begin(ST7796_SPI_SCLK, ST7796_SPI_MISO, ST7796_SPI_MOSI);
    i2c_gt911.begin(GT911_IIC_SDA, GT911_IIC_SCL);
#endif

    // Setup TFT display
    lvgl_tft_init();
    static lv_disp_draw_buf_t draw_buf;
    static lv_color_t buf[TFT_WIDTH * 10];
    lv_disp_draw_buf_init(&draw_buf, buf, NULL, TFT_WIDTH * 10);

    // Setup TFT display
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
#if defined(TFT_ORIENTATION_PORTRAIT) || defined(TFT_ORIENTATION_PORTRAIT_INV)
    disp_drv.hor_res = TFT_WIDTH;
    disp_drv.ver_res = TFT_HEIGHT;
#else
#if defined(TFT_ORIENTATION_LANDSCAPE) || defined(TFT_ORIENTATION_LANDSCAPE_INV)
    disp_drv.hor_res = TFT_HEIGHT;
    disp_drv.ver_res = TFT_WIDTH;
#else
#error TFT_ORIENTATION not defined!
#endif
#endif
    disp_drv.flush_cb = lvgl_tft_flush;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register(&disp_drv);

    // Setup touch
    lvgl_touch_init();
    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = lvgl_touch_read;
    lv_indev_drv_register(&indev_drv);
}

void setLedColor(uint8_t r, uint8_t g, uint8_t b)
{
    ledcWrite(0, r - 0xFF);
    ledcWrite(1, g - 0xFF);
    ledcWrite(2, b - 0xFF);
}

int getLightIntensity()
{
    return analogRead(CDS_PIN);
}

void beep(unsigned int frequency, unsigned long duration)
{
    // Only set low impedance when used. Creates a hissing sound
    pinMode(AUDIO_PIN, OUTPUT);
    tone(AUDIO_PIN, frequency, duration);
    diditalWrite(AUDIO_PIN, LOW);
    pinMode(AUDIO_PIN, INPUT);
}   