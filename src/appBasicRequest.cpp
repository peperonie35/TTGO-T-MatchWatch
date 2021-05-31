#include "config.h"
#include "MWatch.h"

int disp_update_time;
bool need_wifi = false;

void appBasicRequest(AppState s) {
  handle_wifi_for_app(s, need_wifi);
  if(s == HANDLE) {
    if (disp_update_time + 1000 < millis()) {
      disp_update_time = millis();
      tft->setCursor(0, 40);
      tft->print("USB Power");
    }
    //prevent app fo going sleep to keep wifi connection up
    m_idle();
  }
  else if (s == DELETE) {
    tft->fillScreen(TFT_BLACK);
  } else if (s == INIT) {
    tft->fillScreen(TFT_BLACK);
  }
  else if(s == WIFI_CONNECTED) {
    Serial.println("app basic request wifi connected");
    need_wifi = false;
  }
}
