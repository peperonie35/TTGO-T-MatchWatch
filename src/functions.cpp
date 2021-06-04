#include "config.h"
#include "MWatch.h"

#include <AceTime.h>

using namespace ace_time;

static const int CACHE_SIZE = 3;
static BasicZoneManager<CACHE_SIZE> manager(zonedb::kZoneRegistrySize, zonedb::kZoneRegistry);

//change tft brightness depending on wheather or not usb is pluged in
void bright_check (void) {
  static uint8_t old_brightness = 0;
  /*if (power->isVBUSPlug()) {
    screen_brightness = 255;
    charge_cable_connected = true;
  }
  else {
    screen_brightness = on_battery_screen_brightness;
    charge_cable_connected = false;
  }*/
  if (old_brightness != screen_brightness) {
    ttgo->setBrightness(screen_brightness);       // 0-255
    old_brightness = screen_brightness;
    //ttgo->setBrightness(255);       // 0-255
  }
}

//update last_activity and screen brightness, call this to prevent watch from going to light sleep
void m_idle(void) {
  last_activity = millis();
  bright_check();
}

//makes a vibration
void quickBuzz(void) {
  digitalWrite(4, HIGH);
  delay(50);
  digitalWrite(4, LOW);
}

void disable_rtc_alarm(void) {
  ttgo->rtc->disableAlarm();
  ttgo->rtc->resetAlarm();
  Serial.println(F("disable_rtc_alarm()"));
}

//request date from web and set the rtc date and to it, not done yet
void set_rtc_time_from_web() {
  
}

//return current date using the rtc and the m_tz time zone
Date getDate(uint32_t tz) {
  Date tt;
  auto GMT_tz = manager.createForZoneId(TZ_GMT);
  RTC_Date ttnow = ttgo->rtc->getDateTime();
  auto GMT_Time = ZonedDateTime::forComponents( ttnow.year, ttnow.month, ttnow.day, ttnow.hour, ttnow.minute, ttnow.second, GMT_tz);
  auto localTz = manager.createForZoneId(tz);
  auto localTime = GMT_Time.convertToTimeZone(localTz);
  tt.hh     = localTime.hour();
  tt.mm     = localTime.minute();
  tt.ss     = localTime.second();
  tt.dday   = localTime.day();
  tt.mmonth = localTime.month();
  tt.yyear  = localTime.year();
  tt.gdow   = localTime.dayOfWeek();
  return tt;
}

//change the current application
void changeCurrentApp(void(*new_app)(AppState)) {
  current_app(AppState::DELETE);
  Serial.println();
  Serial.print("switching app with function prt (App name unknown)");
  new_app(AppState::INIT);
  current_app = new_app;
}

//change the current application
void changeCurrentApp(String appName) {
  void(*new_app)(AppState) = current_app;
  for (int i = 0; i < NB_APP; i ++) {
    if (Applications[i].appName == appName) {
      new_app = Applications[i].app_ptr;
    }
  }
  current_app(AppState::DELETE);
  Serial.println();
  Serial.print("switching app to: " + appName);
  new_app(AppState::INIT);
  current_app = new_app;
}

//draws one items of a keyboard
void flash_keyboard_item (uint8_t num_keys, const char **b_labels, uint8_t font, bool leave_room_for_label, int row, int col) {
  uint8_t yvals[4], yh;
  uint16_t icolor;
  if(leave_room_for_label) {
    yvals[0] = 35;
    yvals[1] = 85;
    yvals[2] = 135;
    yvals[3] = 185;
    yh = 50;
  }
  else {
    yvals[0] =   0;
    yvals[1] =  60;
    yvals[2] = 120;
    yvals[3] = 180;
    yh = 60;
  }
  int16_t xvals[4];
  int16_t xtvals[4];
  xvals[0] = 0;
  if(num_keys == 12) {
    for(int i = 0 ; i < 3 ; i++) {
      xvals[i]  = i * (246 / 3);
      xtvals[i] = xvals[i] + (246 / 6);
    }
  }
  else {
    for(int i = 0 ; i < 4 ; i++) {
      xvals[i]  = i * (248 / 4);
      xtvals[i] = xvals[i] + (248 / 8);
    }
  }
  // note: space at the top do display what is typed
  // was ttgo->tft->fillRect(0, 35, 80, 50, TFT_BLUE);
  // number keys are 80 x 50, four rows of three
  // x=0, 81, 161, y=35, 85, 135, 185
  ttgo->tft->setTextColor(TFT_GREEN);
  int ino = col + (row * (num_keys / 4));
  icolor = (!strcmp(b_labels[ino], "CANCEL") || !strcmp(b_labels[ino], "DONE")) ? TFT_DARKGREY : TFT_BLUE ;
  ttgo->tft->fillRoundRect(xvals[col], yvals[row], 75, yh-5, 6, icolor);
  ttgo->tft->drawCentreString(b_labels[ino], xtvals[col], yvals[row]+5, font);
}


void flash_keyboard_item_txt (uint8_t num_keys, String text, uint8_t font, bool leave_room_for_label, int row, int col) {
  uint8_t yvals[4], yh;
  uint16_t icolor;
  if(leave_room_for_label) {
    yvals[0] = 35;
    yvals[1] = 85;
    yvals[2] = 135;
    yvals[3] = 185;
    yh = 50;
  }
  else {
    yvals[0] =   0;
    yvals[1] =  60;
    yvals[2] = 120;
    yvals[3] = 180;
    yh = 60;
  }
  int16_t xvals[4];
  int16_t xtvals[4];
  xvals[0] = 0;
  if(num_keys == 12) {
    for(int i = 0 ; i < 3 ; i++) {
      xvals[i]  = i * (246 / 3);
      xtvals[i] = xvals[i] + (246 / 6);
    }
  }
  else {
    for(int i = 0 ; i < 4 ; i++) {
      xvals[i]  = i * (248 / 4);
      xtvals[i] = xvals[i] + (248 / 8);
    }
  }
  // note: space at the top do display what is typed
  // was ttgo->tft->fillRect(0, 35, 80, 50, TFT_BLUE);
  // number keys are 80 x 50, four rows of three
  // x=0, 81, 161, y=35, 85, 135, 185
  ttgo->tft->setTextColor(TFT_GREEN);
  int ino = col + (row * (num_keys / 4));
  icolor = (!strcmp(text.c_str(), "CANCEL") || !strcmp(text.c_str(), "DONE")) ? TFT_DARKGREY : TFT_BLUE ;
  ttgo->tft->fillRoundRect(xvals[col], yvals[row], 75, yh-5, 6, icolor);
  ttgo->tft->drawCentreString(text.c_str(), xtvals[col], yvals[row]+5, font);
}

//draws a keyboard on the screen
void draw_keyboard (uint8_t num_keys, const char **b_labels, uint8_t font, bool leave_room_for_label, char *top_label) {
  uint8_t yvals[4], yh, row, col;
  uint16_t icolor;
  ttgo->tft->fillScreen(TFT_BLACK);
  if(leave_room_for_label) {
    yvals[0] = 35;
    yvals[1] = 85;
    yvals[2] = 135;
    yvals[3] = 185;
    yh = 50;
    if(top_label) {
      ttgo->tft->setTextColor(TFT_GREEN);
      ttgo->tft->drawCentreString( top_label, half_width, 5, font);
    }
  }
  else {
    yvals[0] =   0;
    yvals[1] =  60;
    yvals[2] = 120;
    yvals[3] = 180;
    yh = 60;
  }
  int16_t xvals[4];
  int16_t xtvals[4];
  xvals[0] = 0;
  if(num_keys == 12) {
    for(int i = 0 ; i < 3 ; i++) {
      xvals[i]  = i * (246 / 3);
      xtvals[i] = xvals[i] + (246 / 6);
    }
  }
  else {
    for(int i = 0 ; i < 4 ; i++) {
      xvals[i]  = i * (248 / 4);
      xtvals[i] = xvals[i] + (248 / 8);
    }
  }
  // note: space at the top do display what is typed
  // was ttgo->tft->fillRect(0, 35, 80, 50, TFT_BLUE);
  // number keys are 80 x 50, four rows of three
  // x=0, 81, 161, y=35, 85, 135, 185
  ttgo->tft->setTextColor(TFT_GREEN);
  for(row = 0 ; row < 4 ; row++) {
    for(col = 0 ; col < (num_keys / 4) ; col++) {
      int ino = col + (row * (num_keys / 4));
      icolor = (!strcmp(b_labels[ino], "CANCEL") || !strcmp(b_labels[ino], "DONE")) ? TFT_DARKGREY : TFT_BLUE ;
      ttgo->tft->fillRoundRect(xvals[col], yvals[row], (num_keys == 12) ? 75 : 55, yh-5, 6, icolor);
      ttgo->tft->drawCentreString(b_labels[ino], xtvals[col], yvals[row]+5, font);
    }
  }
}
