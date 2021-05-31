#include "config.h"
#include "MWatch.h"

#include <AceTime.h>

using namespace ace_time;

static const int CACHE_SIZE = 3;
static BasicZoneManager<CACHE_SIZE> manager(
  zonedb::kZoneRegistrySize, zonedb::kZoneRegistry);

//handle default app change with gestures
void defaultAppSwaperGestureHandler(int mSelect) {
  if (mSelect == DOWN) {
    if(defaultAppSwaperCurrentAppXPosition != 0) {
      defaultAppSwaperCurrentAppXPosition = defaultAppSwaperCurrentAppXPosition - 1;
    } else {
      return;
    }
  } else if (mSelect == UP) {
    if(defaultAppSwaperCurrentAppXPosition != defaultAppSwaperAppPositionsXmax) {
      defaultAppSwaperCurrentAppXPosition += 1;
    } else {
      return;
    }
  }
  else if (mSelect == RIGHT) {
    if(defaultAppSwaperCurrentAppYPosition != 0) {
      defaultAppSwaperCurrentAppYPosition = defaultAppSwaperCurrentAppYPosition - 1;
    } else {
      return;
    }
  } else if (mSelect == LEFT) {
    if(defaultAppSwaperCurrentAppYPosition != defaultAppSwaperAppPositionsYmax) {
      defaultAppSwaperCurrentAppYPosition += 1;
    } else {
      return;
    }
  } else {
    return;
  }
  changeCurrentApp(defaultAppSwaperAppPositions[defaultAppSwaperCurrentAppXPosition][defaultAppSwaperCurrentAppYPosition]);
}

//change tft brightness depending on wheather or not usb is pluged in
void bright_check (void) {
  static uint8_t old_brightness;
  if (power->isVBUSPlug()) {
    screen_brightness = 255;
    charge_cable_connected = true;
  }
  else {
    screen_brightness = on_battery_screen_brightness;
    charge_cable_connected = false;
  }
  if (old_brightness != screen_brightness) {
    ttgo->setBrightness(screen_brightness);       // 0-255
    old_brightness = screen_brightness;
    // ttgo->setBrightness(255);       // 0-255
  }
}

//update last_activity and screen brightness, call this to prevent watch from going to light sleep
void m_idle(void) {
  last_activity = millis();
  bright_check();
}

//make a vibration
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


//return the gesture or a default button press
int poll_swipe_or_menu_press(int num_choices) {
  uint32_t lasttouch, interval;
  int16_t nx, ny, x, y, x0, y0, amax, points;
  x = 0;
  y = 0;
  enum SWIPE_DIR resdir;
  resdir = NODIR;
  if (num_choices < 14) {
    num_choices = 12;
  }
  else {
    num_choices = 16;
  }
  int xdir = 0;
  int ydir = 0;
  int xdif = 0;
  int ydif = 0;
  x0 = -1;
  y0 = -1;
  char dir;
  int16_t max_lrud[4] = {1000, -1, 1000, -1};
  int16_t pts_lrud[4] = { -1, -1, -1, -1};
  points = 0;
  lasttouch = millis();
  do {
    interval = millis() - lasttouch;
    // Serial.printf("(1) interval = %lu\n", interval);
    if (ttgo->getTouch(nx, ny)) {
      // Serial.print(F("T"));
      if ((nx > 0) && (ny > 0) && (nx < 240) && (ny < 240)) {
        // Serial.println(F("G"));
        x = nx; y = ny;
        // remember the first touch:
        if (x0 < 0) {
          x0 = x;
        }
        if (y0 < 0) {
          y0 = y;
        }
        int i = points;
        if (pts_lrud[0] < 0 || x < max_lrud[0]) {
          max_lrud[0] = x;
          pts_lrud[0] = i++;
        }
        if (pts_lrud[1] < 0 || x > max_lrud[1]) {
          max_lrud[1] = x;
          pts_lrud[1] = i++;
        }
        if (pts_lrud[2] < 0 || y < max_lrud[2]) {
          max_lrud[2] = y;
          pts_lrud[2] = i++;
        }
        if (pts_lrud[3] < 0 || y > max_lrud[3]) {
          max_lrud[3] = y;
          pts_lrud[3] = i++;
        }
        points++;
      }
      lasttouch = millis();
    }
  } while (interval < 100);
  if (x0 >= 0 && y0 >= 0) {
    //udpate last activity cause touch
    m_idle();
    // Serial.printf("(2) interval = %lu\n", interval);
    xdir = x - x0;  // x extent of swipe.  near zero if a circle gesture
    ydir = y - y0;  // y extent of swipe.  near zero if a circle gesture
    xdif = max_lrud[1] - max_lrud[0]; // always a positive difference
    ydif = max_lrud[3] - max_lrud[2]; // always a positive difference
    amax = (xdif > ydif) ? xdif : ydif ;
    if (points > 100 && xdif > 60 && ydif > 60) { // is gesture a circle ?
      int16_t min_value[4] = {1000, 1000, 1000, 1000};
      char lut[4] = { 'L', 'R', 'U', 'D' };
      char dir_order[9];
      dir_order[8] = '\0';
      int8_t min_index = 0;
      for (int8_t j = 0 ; j < 4 ; j++) {
        for (int8_t i = 0 ; i < 4 ; i++) {
          if (pts_lrud[i] < min_value[j] && (j == 0 || pts_lrud[i] > min_value[j - 1])) {
            min_value[j] = pts_lrud[i];
            min_index = i;
          }
        }
        dir_order[j] = lut[min_index];
        dir_order[j + 4] = lut[min_index];
      }
      if (strstr(dir_order, "URDL")) {
        return (CWCIRCLE);
      }
      if (strstr(dir_order, "ULDR")) {
        return (CCWCIRCLE);
      }
    }
    else if (amax > 60) { // moved across 1/4 of the screen, so probably a swipe
      dir = (ydif > xdif) ? 'y' : 'x' ;
      resdir =
        (dir == 'x') ? ((xdir > 0) ? RIGHT : LEFT ) :
        (ydir > 0) ? DOWN : UP;
      return resdir;
    }
    else {  // must be button press
      if (num_choices == 12) {
        if (y < 85) {
          if (x < 80) return 0;
          else if (x > 160) return 2;
          else return 1;
        }
        else if (y < 135) {
          if (x < 80) return 3;
          else if (x > 160) return 5;
          else return 4;
        }
        else if (y < 185) {
          if (x < 80) return 6;
          else if (x > 160) return 8;
          else return 7;
        }
        else if (x < 80) return 9;
        else if (x > 160) return 11;
        else return 10;
      }
      else {  // must be 16 choices
        if (y < 85) {
          if (x < 60) return 0;
          else if (x < 120) return 1;
          else if (x < 180) return 2;
          else return 3;
        }
        else if (y < 135) {
          if (x < 60) return 4;
          else if (x < 120) return 5;
          else if (x < 180) return 6;
          else return 7;
        }
        else if (y < 185) {
          if (x < 60) return 8;
          else if (x < 120) return 9;
          else if (x < 180) return 10;
          else return 11;
        }
        if (x < 60) return 12;
        else if (x < 120) return 13;
        else if (x < 180) return 14;
        else return 15;
      }
    }
  }
  return -1;
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
  new_app(AppState::INIT);
  current_app = new_app;
  Serial.println();
  Serial.print("switching app to: " + appName);
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
