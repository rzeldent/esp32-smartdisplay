#include <Arduino.h>
#include <esp32_smartdisplay.h>

void setup()
{
  smartdisplay_init();
}

void loop()
{
  lv_timer_handler();
}