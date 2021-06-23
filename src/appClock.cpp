#include "config.h"
#include "MWatch.h"

static uint32_t clock_update_time;

static int battery_level = 0;

static byte xpos; // Starting position for the display
static byte ypos;
static byte omm;
static byte xcolon;

//draw a digital clock folloiwng the current rtc date at m_tz time zone
void drawDigitalClock(AppState s) {
  if (s == AppState::INIT) {
    tft->fillScreen(TFT_BLACK);
    omm = 99;
    xcolon = 0;
  } else if (s == AppState::HANDLE) {
    xpos = 40; // Starting position for the
    ypos = 90;
    Date tt = getDate(m_tz);
    if (omm != tt.mm) {
      ttgo->tft->setTextColor(0x39C4, TFT_BLACK);
      ttgo->tft->drawString("88:88", xpos, ypos, 7); // Overwrite the text to clear it
      ttgo->tft->setTextColor(0xFBE0, TFT_BLACK); // Orange

      omm = tt.mm;

      if (tt.hh < 10) xpos += ttgo->tft->drawChar('0', xpos, ypos, 7);
      xpos += ttgo->tft->drawNumber(tt.hh, xpos, ypos, 7);
      xcolon = xpos;
      xpos += ttgo->tft->drawChar(':', xpos, ypos, 7);
      if (tt.mm < 10) xpos += ttgo->tft->drawChar('0', xpos, ypos, 7);
      ttgo->tft->drawNumber(tt.mm, xpos, ypos, 7);
    }

    if (tt.ss % 2) { // Flash the colon
      ttgo->tft->setTextColor(0x39C4, TFT_BLACK);
      xpos += ttgo->tft->drawChar(':', xcolon, ypos, 7);
      ttgo->tft->setTextColor(0xFBE0, TFT_BLACK);
    } else {
      ttgo->tft->setTextColor(0xFBE0, TFT_BLACK);
      ttgo->tft->drawChar(':', xcolon, ypos, 7);
    }
  }
}

static void m_ble_cb(String command, String data) {
  Serial.printf("\nrecived ble:\n command:\n  %s\n data:\n  %s", command.c_str(), data.c_str());
}

void appClock(AppState s) {
  drawDigitalClock(s);
  if (s == AppState::DELETE) {
    tft->fillScreen(TFT_BLACK);
  } else if (s == AppState::INIT) {
    tft->fillScreen(TFT_BLACK);
    ttgo->tft->setTextColor(TFT_DARKGREY, TFT_BLACK);
    tft->drawString(String(battery_level) + "%", 190, 0, 4);
    ttgo->tft->setTextColor(TFT_YELLOW, TFT_BLACK);
  } else if (s == HANDLE) {
    if(battery_level != ttgo->power->getBattPercentage()) {
      battery_level = ttgo->power->getBattPercentage();
      ttgo->tft->setTextColor(TFT_DARKGREY, TFT_BLACK);
      tft->drawString(String(battery_level) + "%", 190, 0, 4);
    }
  } else if(s == SETUP) {
    add_ble_cb(m_ble_cb, "appClock cb");
  }
}
