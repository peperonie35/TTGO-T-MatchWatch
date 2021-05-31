#include "config.h"
#include "MWatch.h"

uint32_t clock_update_time;

int battery_level = 0;

byte xpos; // Starting position for the display
byte ypos;
byte omm;
byte xcolon;

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
      // Uncomment ONE of the next 2 lines, using the ghost image demonstrates text overlay as time is drawn over it
      ttgo->tft->setTextColor(0x39C4, TFT_BLACK);  // Leave a 7 segment ghost image, comment out next line!
      //ttgo->tft->setTextColor(TFT_BLACK, TFT_BLACK); // Set font colour to black to wipe image
      // Font 7 is to show a pseudo 7 segment display.
      // Font 7 only contains characters [space] 0 1 2 3 4 5 6 7 8 9 0 : .
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
      ttgo->tft->drawChar(':', xcolon, ypos, 7);
    }
  }
}


void appClock(AppState s) {
  drawDigitalClock(s);
  if (s == AppState::DELETE) {
    tft->fillScreen(TFT_BLACK);
  } else if (s == AppState::INIT) {
    ttgo->tft->setTextColor(TFT_DARKGREY, TFT_BLACK);
    tft->drawString(String(battery_level) + "%", 190, 0, 4);
    ttgo->tft->setTextColor(TFT_YELLOW, TFT_BLACK);
  } else if (s == HANDLE) {
    if(battery_level != ttgo->power->getBattPercentage()) {
      battery_level = ttgo->power->getBattPercentage();
      ttgo->tft->setTextColor(TFT_DARKGREY, TFT_BLACK);
      tft->drawString(String(battery_level) + "%", 190, 0, 4);
    }
  }
}
