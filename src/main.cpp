#include <Arduino.h>
#include <ArduinoOTA.h>

#include <esp32_smartdisplay.h>

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

// LVGL Objects
static lv_obj_t *label_cds;
static lv_obj_t *label_date;
static lv_obj_t *label_status;
static lv_obj_t *label_ipaddress;

void display_update()
{
  lv_label_set_text(label_date, get_localtime("%c").c_str());
  lv_label_set_text(label_ipaddress, WiFi.localIP().toString().c_str());
  lv_label_set_text(label_cds, String(smartdisplay_getLightIntensity()).c_str());
}

void btn_event_cb(lv_event_t *e)
{
  auto code = lv_event_get_code(e);
  auto btn = lv_event_get_target(e);
  if (code == LV_EVENT_CLICKED)
  {
    static uint8_t cnt = 0;
    cnt++;

    smartdisplay_beep(1000, 50);

    auto label = lv_obj_get_child(btn, 0);
    lv_label_set_text_fmt(label, "Button: %d", cnt);
  }
}

void mainscreen()
{
  // Clear screen
  lv_obj_clean(lv_scr_act());

  auto btn = lv_btn_create(lv_scr_act());
  lv_obj_set_pos(btn, 10, 10);
  lv_obj_set_size(btn, 120, 50);
  lv_obj_add_event_cb(btn, btn_event_cb, LV_EVENT_ALL, NULL);

  auto label = lv_label_create(btn);
  lv_label_set_text(label, "Button");
  lv_obj_center(label);

  label_date = lv_label_create(lv_scr_act());
  lv_obj_set_style_text_font(label_date, &lv_font_montserrat_22, LV_STATE_DEFAULT);
  lv_obj_align(label_date, LV_ALIGN_BOTTOM_MID, 0, -50);

  label_ipaddress = lv_label_create(lv_scr_act());
  lv_obj_set_style_text_font(label_ipaddress, &lv_font_montserrat_22, LV_STATE_DEFAULT);
  lv_obj_align(label_ipaddress, LV_ALIGN_BOTTOM_MID, 0, -80);

  label_cds = lv_label_create(lv_scr_act());
  lv_obj_set_style_text_font(label_cds, &lv_font_montserrat_22, LV_STATE_DEFAULT);
  lv_obj_align(label_cds, LV_ALIGN_TOP_RIGHT, 0, 0);

  label_status = lv_label_create(lv_scr_act());
  lv_obj_set_style_text_font(label_status, &lv_font_montserrat_22, LV_STATE_DEFAULT);
  lv_obj_align(label_status, LV_ALIGN_TOP_MID, 0, 40);
}

void setup()
{
  // put your setup code here, to run once:
  setCpuFrequencyMhz(160);
  Serial.begin(115200);
  log_i("CPU Freq = %d Mhz", getCpuFrequencyMhz());
  log_i("Free heap: %d bytes", ESP.getFreeHeap());

  smartdisplay_init();

  WiFi.begin(WIFI_SSDID, WIFI_PASSWORD);
  ArduinoOTA.begin();
  // Set the time servers
  configTime(0, 0, "nl.pool.ntp.org");
  setenv("TZ", "Europe/Amsterdam", 1);
  tzset();

  mainscreen();
}

void loop()
{
  // Red if no wifi, otherwise green
  bool connected = WiFi.isConnected();
  smartdisplay_setLedColor(connected ? 0 : 25, connected ? 25 : 0, 0);

  // put your main code here, to run repeatedly:
  ArduinoOTA.handle();

  lv_timer_handler();

  display_update();

  yield();
}
