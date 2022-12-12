#include <Arduino.h>
#include <ArduinoOTA.h>

#include <lvgl_drv_tft.h>

#include ".secrets.h"

bool time_valid()
{
  // Value of time_t for 2000-01-01 00:00:00, used to detect invalid SNTP responses.
  constexpr time_t epoch_2000_01_01 = 946684800;
  return time(nullptr) > epoch_2000_01_01;
}

String get_localtime(const char *format)
{
  if (!time_valid())
    return "No time available";

  struct tm timeinfo;
  getLocalTime(&timeinfo);
  char time_buffer[32];
  strftime(time_buffer, sizeof(time_buffer), format, &timeinfo);
  return time_buffer;
}

void display_update()
{
  static lv_obj_t *label_date;
  if (label_date == nullptr)
  {
    label_date = lv_label_create(lv_scr_act());
    lv_obj_set_style_text_font(label_date, &lv_font_montserrat_22, LV_STATE_DEFAULT);
    lv_obj_align(label_date, LV_ALIGN_BOTTOM_MID, 0, -50);
  }
  lv_label_set_text(label_date, get_localtime("%c").c_str());

  static lv_obj_t *label_ipaddress;
  if (label_ipaddress == nullptr)
  {
    label_ipaddress = lv_label_create(lv_scr_act());
    lv_obj_set_style_text_font(label_ipaddress, &lv_font_montserrat_22, LV_STATE_DEFAULT);
    lv_obj_align(label_ipaddress, LV_ALIGN_BOTTOM_MID, 0, -80);
  }
  lv_label_set_text(label_ipaddress, WiFi.localIP().toString().c_str());
}

void btn_event_cb(lv_event_t *e)
{
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t *btn = lv_event_get_target(e);
  if (code == LV_EVENT_CLICKED)
  {
    static uint8_t cnt = 0;
    cnt++;

    lv_obj_t *label = lv_obj_get_child(btn, 0);
    lv_label_set_text_fmt(label, "Button: %d", cnt);
  }
}

void mainscreen()
{
  lv_obj_t *btn = lv_btn_create(lv_scr_act());
  lv_obj_set_pos(btn, 10, 10);
  lv_obj_set_size(btn, 120, 50);
  lv_obj_add_event_cb(btn, btn_event_cb, LV_EVENT_ALL, NULL);

  lv_obj_t *label = lv_label_create(btn);
  lv_label_set_text(label, "Button");
  lv_obj_center(label);
}

void setup()
{
  // put your setup code here, to run once:
  setCpuFrequencyMhz(160);
  Serial.begin(115200);
  log_i("CPU Freq = %d Mhz", getCpuFrequencyMhz());
  log_i("Free heap: %d bytes", ESP.getFreeHeap());

  lvgl_init();

  // Set LED. High is off
  pinMode(PIN_LED_R, OUTPUT);
  digitalWrite(PIN_LED_R, true);
  pinMode(PIN_LED_G, OUTPUT);
  digitalWrite(PIN_LED_G, true);
  pinMode(PIN_LED_B, OUTPUT);
  digitalWrite(PIN_LED_B, true);

  WiFi.begin(WIFI_SSDID, WIFI_PASSWORD);
  ArduinoOTA.begin();
  // Set the time servers
  configTime(0, 0, "nl.pool.ntp.org");
  setenv("TZ", "Europe/Amsterdam", 1);
  tzset();

  // Clear screen
  // lv_obj_clean(lv_scr_act());
  mainscreen();
}

void loop()
{
  // Red if no wifi, otherwise green
  bool connected = WiFi.isConnected();
  analogWrite(PIN_LED_R, connected ? 255 : 200);
  analogWrite(PIN_LED_G, connected ? 200 : 255);

  // put your main code here, to run repeatedly:
  ArduinoOTA.handle();

  lv_timer_handler();

  display_update();

  yield();
}
