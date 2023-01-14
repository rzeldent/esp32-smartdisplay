#include <Arduino.h>
#include <ArduinoOTA.h>

#include <esp32_smartdisplay.h>

// LVGL Objects
static lv_obj_t *label_cds;
static lv_obj_t *label_date;
static lv_obj_t *label_ipaddress;

void display_update()
{
  char time_buffer[32];
  struct tm timeinfo;
  getLocalTime(&timeinfo);
  strftime(time_buffer, sizeof(time_buffer), "%c", &timeinfo);

  lv_label_set_text(label_date, time_buffer);
  lv_label_set_text(label_ipaddress, WiFi.localIP().toString().c_str());
  lv_label_set_text(label_cds, String(smartdisplay_get_light_intensity()).c_str());
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

  // Create a buttom
  auto btn = lv_btn_create(lv_scr_act());
  lv_obj_set_pos(btn, 10, 10);
  lv_obj_set_size(btn, 120, 50);
  lv_obj_add_event_cb(btn, btn_event_cb, LV_EVENT_ALL, NULL);
  // Set label to Button
  auto label = lv_label_create(btn);
  lv_label_set_text(label, "Button");
  lv_obj_center(label);

  // Create a label for the date
  label_date = lv_label_create(lv_scr_act());
  lv_obj_set_style_text_font(label_date, &lv_font_montserrat_22, LV_STATE_DEFAULT);
  lv_obj_align(label_date, LV_ALIGN_BOTTOM_MID, 0, -50);

  // Create a label for the IP Address
  label_ipaddress = lv_label_create(lv_scr_act());
  lv_obj_set_style_text_font(label_ipaddress, &lv_font_montserrat_22, LV_STATE_DEFAULT);
  lv_obj_align(label_ipaddress, LV_ALIGN_BOTTOM_MID, 0, -80);

  // Create a label for the CDS Sensor
  label_cds = lv_label_create(lv_scr_act());
  lv_obj_set_style_text_font(label_cds, &lv_font_montserrat_22, LV_STATE_DEFAULT);
  lv_obj_align(label_cds, LV_ALIGN_TOP_RIGHT, 0, 0);
}

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);

  smartdisplay_init();

  WiFi.begin();

  // Set the time servers
  configTime(0, 0, "pool.ntp.org");

  mainscreen();
}

void loop()
{
  // put your main code here, to run repeatedly:

  // Red if no wifi, otherwise green
  bool connected = WiFi.isConnected();
  smartdisplay_set_led_color(connected ? { 0x00, 0xFF, 0x00 } : { 0xFF, 0x00, 0x00 });

  ArduinoOTA.handle();

  display_update();
  lv_timer_handler();
}