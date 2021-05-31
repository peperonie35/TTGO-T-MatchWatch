#ifndef MWATCH_H
#define MWATCH_H

/*EEPROM for config storage is not implemented yet;
#include <EEPROM.h>
#include "EEPROM_rw.h"
*/

#ifdef __MAIN__
#define EXTERN
#define INIT(x) = x
#else
#define EXTERN extern
#define INIT(x)
#endif

//personal data:
#define M_LANGUAGE "fr"
#define OPENWEATHER_KEY "55b40669d0fd19e39e9112eea873ceb3"
#define LOCATIONIQ_KEY "71a25ff51f36cd7f4ed7b6ee8556dd5d"
#define HOME_LATITUDE "48.1605388"
#define HOME_LONGITUDE "-1.7643878"
#define HOME_CITY "Pace, FR"

//idk what most of that does, but it's imporant ?
#define G_EVENT_VBUS_PLUGIN         _BV(0)
#define G_EVENT_VBUS_REMOVE         _BV(1)
#define G_EVENT_CHARGE_DONE         _BV(2)

#define G_EVENT_WIFI_SCAN_START     _BV(3)
#define G_EVENT_WIFI_SCAN_DONE      _BV(4)
#define G_EVENT_WIFI_CONNECTED      _BV(5)
#define G_EVENT_WIFI_BEGIN          _BV(6)
#define G_EVENT_WIFI_OFF            _BV(7)

enum {
  Q_EVENT_WIFI_SCAN_DONE,
  Q_EVENT_WIFI_CONNECT,
  Q_EVENT_BMA_INT,
  Q_EVENT_AXP_INT,
} ;

//times zones
#define TZ_AEST    0x0fe559a3  // Eastern Australia (Melbourne) +10
#define TZ_ACST    0x2428e8a3 // Central Australia (Adelaide) +9:30
#define TZ_AWST    0x8db8269d // Western Australia (Perth) +8
#define TZ_CHINA   0x577f28ac // People's Republic of China +8
#define TZ_ICT     0x20f2d127 // Indochina Time (Vietnam) +7
#define TZ_NPT     0x9a96ce6f // Nepal +5:45
#define TZ_IST     0x72c06cd9 // India (Calcutta, New Delhi, etc.) +5:30
#define TZ_AST     0x9ceffbed // Arabia Standard Time (Iraq, Iran) +3
#define TZ_EET     0x4318fa27 // Eastern European Time (Athens) +2
#define TZ_CET     0xa2c58fd7 // Central European Time(Rome, Madrid) +1
#define TZ_WET     0x5c6a84ae // Western European Time (London)
#define TZ_GMT     0xd8e31abc // Greenwich Mean Time
#define TZ_AZO     0xf93ed918 // Azores Standard Time -1
#define TZ_OSC     0x33013174 // Oscar Standard Time (and South Georgia) -2
#define TZ_WGT     0x9805b5a9 // Western Greenland Time (Nuuk) -3
#define TZ_ATL     0xbc5b7183 // Atlantic Standard Time (Bermuda, Labrador)
#define TZ_EST     0x1e2a7654 // Eastern Standard Time -5
#define TZ_CST     0x4b92b5d4 // Central Standard Time -6
#define TZ_MST     0x97d10b2a // Mountain Standard Time -7
#define TZ_ARIZONA 0x34b5af01 // Arizona -7
#define TZ_PST     0xd99ee2dc // Pacific Standard Time -8
#define TZ_AKST    0x5a79260e // Alaska Standard Time -9
#define TZ_HST     0xe6e70af9 // Hawaii-Aleutian Standard Time -10

// #define DEFAULT_SCREEN_TIMEOUT  30*1000 // now user controllable

#define WATCH_FLAG_SLEEP_MODE   _BV(1)
#define WATCH_FLAG_SLEEP_EXIT   _BV(2)
#define WATCH_FLAG_BMA_IRQ      _BV(3)
#define WATCH_FLAG_AXP_IRQ      _BV(4)

//clock skins:
#define BASIC_CLOCK_SKIN 1

//basic structs / enums
enum AppState { HANDLE = 0, INIT, DELETE, WIFI_SCAN_START, WIFI_SCAN_ENDED, WIFI_CONNECTING, WIFI_CONNECTED, WIFI_DISCONNECTING, WIFI_DISCONNECTED, WIFI_NO_AVAILABLE };
struct AP {
  String ssid;
  String password;
};
struct App {
  String appName;
  void (*app_ptr)(AppState);
};
struct Date {
  uint8_t hh, mm, ss, mmonth, dday, gdow; // H, M, S variables
  uint16_t yyear; // Year is 16 bit int
};

enum SWIPE_DIR { NODIR = 31, UP, DOWN, LEFT, RIGHT, CWCIRCLE, CCWCIRCLE };

//applications functions:
void appClock(AppState);
void appBattery(AppState);
void appBasicRequest(AppState);
void appCalc(AppState);
void appStopWatch(AppState);
void appWifiRemote(AppState s);

void changeCurrentApp(String appName);
void handle_wifi_for_app(AppState s, bool need_wifi);
void defaultAppSwaperGestureHandler(int mSelect);

//other functions:
void quickBuzz(void);
void m_idle(void);
void bright_check(void);
void low_energy(void);
void disable_rtc_alarm(void);
int poll_swipe_or_menu_press(int num_choices);
Date getDate(uint32_t tz);
void draw_keyboard(uint8_t, const char **, uint8_t, bool, char *);
void flash_keyboard_item (uint8_t num_keys, const char **b_labels, uint8_t font, bool leave_room_for_label, int row, int col);
void flash_keyboard_item_txt (uint8_t num_keys, String text, uint8_t font, bool leave_room_for_label, int row, int col);

void wifi_setup();
void begin_network_scan();
int get_ap_if_wifi_scan_match();
void connect_to_wifi_ap(String ssid, String password);
void connect_to_wifi_ap(int k);
void disconnect_wifi_ap();
void turn_off_wifi();
String make_http_get_request(String url);

//global variables:

//config values
#ifdef __MAIN__
EXTERN uint32_t m_tz = TZ_CET;
EXTERN void (*current_app)(AppState) = &appClock;
EXTERN void (*current_gesture_handler)(int) = &defaultAppSwaperGestureHandler;
EXTERN int menu_press_mode = 16;
EXTERN uint8_t on_battery_screen_brightness = 100;
EXTERN uint8_t screensaver_timeout = 5;
#define MAX_NB_AP 2
EXTERN AP AccessPoints[MAX_NB_AP] = {
  { "ssid", "password"},    // Home hotspot
  { "ssid1", "password1" }, // my phone hotspot
};
EXTERN int defaultAppSwaperAppPositionsXmax = 2;
EXTERN int defaultAppSwaperAppPositionsYmax = 1;
EXTERN String defaultAppSwaperAppPositions[3][2] = {
  {"Clock App", "Calc App"},
  {"Battery App", "StopWatch App"},
  {"WifiRemote App", "Calc App"}
 };
EXTERN int defaultAppSwaperCurrentAppXPosition = 0;
EXTERN int defaultAppSwaperCurrentAppYPosition = 0;
#endif
#ifndef __MAIN__
EXTERN uint32_t m_tz;
EXTERN void (*current_app)(AppState);
EXTERN void (*current_gesture_handler)(int);
EXTERN int menu_press_mode;
EXTERN uint8_t on_battery_screen_brightness;
EXTERN uint8_t screensaver_timeout;
#define MAX_NB_AP 2
EXTERN AP AccessPoints[MAX_NB_AP];
EXTERN int defaultAppSwaperAppPositionsXmax;
EXTERN int defaultAppSwaperAppPositionsYmax;
EXTERN String defaultAppSwaperAppPositions[3][2];
EXTERN int defaultAppSwaperCurrentAppXPosition;
EXTERN int defaultAppSwaperCurrentAppYPosition;
#endif

#define NB_APP 6
EXTERN App Applications[NB_APP]
#ifdef __MAIN__
= {
  { "Clock App", &appClock },
  { "Battery App", &appBattery},
  { "BasicRequest App", &appBasicRequest},
  { "Calc App", &appCalc},
  { "StopWatch App", &appStopWatch},
  { "WifiRemote App", &appWifiRemote}
}
#endif
;
//end config values

//other values

EXTERN TTGOClass *ttgo;
EXTERN TFT_eSPI *tft;
EXTERN AXP20X_Class *power;

EXTERN char buff[512];

EXTERN unsigned int half_width;
EXTERN unsigned int half_height;

EXTERN int last_gesture;
EXTERN bool has_handled_gesture;

EXTERN uint8_t screen_brightness;
EXTERN boolean charge_cable_connected;
EXTERN uint32_t last_activity;

EXTERN boolean rtcIrq;

#endif // MWATCH_H
