#pragma once

// RGB LED (inverted)
#define PIN_LED_R 4
#define PIN_LED_G 16
#define PIN_LED_B 17

// Photo resistor
#define PIN_CDS 34 // ANALOG_PIN_0

// AUDIO out
#define PIN_AUDIO_OUT 26
// TF Card
#define PIN_TF_CS 5
#define PIN_TS_MOSI 23
#define PIN_TF_SCLK 18
#define PIN_TF_MISC 19
// TFT
#define TFT_WIDTH 320
#define TFT_HEIGHT 480
// TFT ST7796 SPI
#define PIN_TFT_SCLK 14
#define PIN_TFT_MOSI 13
#define PIN_TFT_MISO 12
#define PIN_TFT_DC 2
// TFT Panel
#define PIN_TFT_CS 15
// TFT backlight
#define PIN_TFT_BL 27
// XPT2046 Resistive touch
#define XPT2046_PIN_INT 36
#define XPT2046_PIN_CS 33
// GT911 Capacitive touch
#define GT911_ADDRESS 0x5d
#define PIN_GT911_INT 21
#define PIN_GT911_RST 25
#define PIN_GT911_SDA 33
#define PIN_GT911_SCL 32
