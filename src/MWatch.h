#ifndef MWATCH_H
#define MWATCH_H

#include <vector>
#include <string>
#include <ArduinoJson.h>
#include <BLEUUID.h>

#ifdef __MAIN__
#define EXTERN
#define INIT(x) = x
#else
#define EXTERN extern
#define INIT(x)
#endif

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
  Q_EVENT_AXP_INT
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

#define WATCH_FLAG_SLEEP_MODE _BV(1)
#define WATCH_FLAG_SLEEP_EXIT _BV(2)
#define WATCH_FLAG_BMA_IRQ _BV(3)
#define WATCH_FLAG_AXP_IRQ _BV(4)

//clock skins:
#define BASIC_CLOCK_SKIN 1

//basic structs / enums
enum LEState {SLEEP = 0, WAKE_UP, START_BACKGROUND, STOP_BACKGROUND};
enum AppState { HANDLE = 0, INIT, DELETE, WIFI_SCAN_START, WIFI_SCAN_ENDED, WIFI_CONNECTING, WIFI_CONNECTED, WIFI_DISCONNECTING, WIFI_DISCONNECTED, WIFI_NO_AVAILABLE, SETUP }; //differents states called in an app
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

#define SLEEP_MODES {"screen_off_sleep_basic", "light_sleep_basic", "deep_sleep_basic", "screen_off_sleep_basic"} //first sleep mode shall be placed last too (easier to change modes then)

//applications functions (default way to make an application is to make a function with an AppState as argument (the app function is called with  enum AppState depending on the current state that the app needs to be (read an "application".cpp to understand) ))
void appClock(AppState); //defaults app displays the time and betterie %
void appClock2(AppState);
void appBattery(AppState); //displays informations about the power manager
void appTest(AppState); // not used in the program, I use it to test some stuff
void appCalc(AppState); // calculatrice
void appStopWatch(AppState); // stop watch app
void appWifiRemote(AppState s); // remote with use wifi to operate (calls api)
void appSettings(AppState s); // app to modify the settings (uses LVGL)

void changeCurrentApp(String appName); //changes the current app
void changeCurrentApp(void(*new_app)(AppState)); //changes the current app
int get_app_x_position_by_name(String app_name);
int get_app_y_position_by_name(String app_name);
App get_app_by_name(String name);

void handle_wifi_for_app(AppState s, bool need_wifi); //handles wifi for an app(needs to be called at the start of the app func), set need_wifi depending on when the app need wifi to preserve batterie, calls current_app(WIFI_CONNECTED) when everything is ready to use internet, calls current_app(WIFI_NO_AVAILABLE) when there is no registered ap nearby
void handle_lvgl_for_app(AppState s, lv_obj_t *page, bool disable_default_gesture, bool enable_default_app_swipe); //use when you want to use lvgl in an app (call at the start of the app func), give a ptr to the root lv_obj_t (hidden at app DELETE, showed at app INIT),set disable_default_gesture to true to allow lvgl to use touch, set enable_default_app_swipe to true to enable app swipe depending on the gesture as normal
void handle_lvgl_for_app(AppState s, void(*__show)(), void(*__hide)(), bool disable_default_gesture, bool enable_default_app_swipe); // __hide is called at app DELETE, __show is called at app INIT
void disable_touch_features_for_app(AppState s); //call like handle_wifi_for_app to disable poll_swipe_or_menu_press when the app is the current_app

void defaultAppSwaperGestureHandler(int mSelect); //handles gesture, mSelect is set to poll_swipe_or_menu_press(menu_press_mode), enables app swipe (called before current_app(HANDLE))

void app_stack_gesture_handler(int mSelect); //handler for the app stack (when mSelect = RIGHT, it calls unstack_app), called in the defaultAppSwaperGestureHandler when app_stack.empty() == false
void stack_app(void (*app_to_stack)(AppState)); //change the current app, but the previus app is restored when it swipes right (if touch enabled(see later))
void unstack_app(); //restore the previous app on the stack (an app is added to the stack when stack_app() is called)

//other functions:
void quickBuzz(void); //makes a vibration
void m_idle(void); //resets last activity to prevent watch from going to sleep (also makes a bright_check())
void bright_check(void); //check the brightness and mofifies the screen brightness if it has changed (called by default in m_idle)
void low_energy(LEState); //called when going in and out of light_sleep (handles light_sleep to save batterie)
void disable_rtc_alarm(void); //diables the rtc alarm (nothing uses this for now)
Date getDate(uint32_t tz); //returs the date depending on the timezone (tz) use m_tz setting if u want (default timezone for the app)
void draw_keyboard(uint8_t, const char **, uint8_t, bool, char *); //draws a keyboard (4*3 or 4*4 respectivly num_keys = 12 or num_keys = 16) needs the table of labels (const char **)
void flash_keyboard_item (uint8_t num_keys, const char **b_labels, uint8_t font, bool leave_room_for_label, int row, int col); //draws a keyboard item (4*3 or 4*4 respectivly num_keys = 12 or num_keys = 16) needs the table of labels (leave_room_for_label let a small space at the top to write something)
void flash_keyboard_item_txt (uint8_t num_keys, String text, uint8_t font, bool leave_room_for_label, int row, int col); //draws a keyboard item (4*3 or 4*4 respectivly num_keys = 12 or num_keys = 16) needs a text string 

lv_style_t* get_lvgl_style(); //returns the default lvgl style for the app (default is just black theme (not sure if it works for everything))

int poll_swipe_or_menu_press(int num_choices); //used to detect gesture without LVGL, 
void init_touch(); //called in void setup()
void disable_touch_features(); // disables poll_swipe_or_menu_press
void enable_touch_features(); //enables poll_swipe_or_menu_press

void wifi_setup(); //called in void setup to setup wifi settings
void begin_network_scan(); //makes a wifi scan of nearby access point(s) (non blocking), calls current_app(WIFI_SCAN_ENDED) when the scan is done, calls current_app(WIFI_NO_AVAILABLE) when there is no registered access point nearby
int get_ap_if_wifi_scan_match(); //reads the result of the wifi scan and returns the access point index if it exists
void connect_to_wifi_ap(String ssid, String password); //connects to an ap depending on the ssid and password
void connect_to_wifi_ap(int k); //connects to an ap depending on his index
void disconnect_wifi_ap(); //diconnects from the ap
void turn_off_wifi(); //turns off wifi
String make_http_get_request(String url); //makes an http request
void set_rtc_time_from_web(); //update system time and rtc time from web

void read_settings();
void write_settings();
void write_file_string(String path, String &data);
String read_file_string(String path);


bool is_ble_connected();
void start_ble_task();
void stop_ble_task();
void enable_ble();
void disable_ble();
void add_ble_command(String command);
void add_ble_cb(void(*cb)(String command, String data), String cb_name);
void remove_ble_cb(String cb_name_to_rmv);

void light_sleep_basic_in();
void light_sleep_basic_out();
void deep_sleep_basic_in();
void screen_off_sleep_basic_in();
void screen_off_sleep_basic_out();

//global variables:

//config values
#ifdef __MAIN__
EXTERN std::string watch_name = "MatWatch";
EXTERN String defaultAppName = "Clock App";
EXTERN uint8_t screen_brightness = 255;
EXTERN uint32_t m_tz = TZ_CET;
EXTERN void (*current_app)(AppState) = &appClock;
EXTERN void (*current_gesture_handler)(int) = &defaultAppSwaperGestureHandler;
EXTERN int menu_press_mode = 16;
EXTERN uint8_t on_battery_screen_brightness = 100;
EXTERN uint8_t screensaver_timeout = 5;
#define MAX_NB_AP 2 //don't forget to change this in the ifndef __MAIN__ part
EXTERN AP AccessPoints[MAX_NB_AP] = {
  { "ssid-1", "password-1"},    // Home hotspot
  { "ssid-2", "password-2" }, // my phone hotspot
};

#define MAX_NB_X_APPSWAPER 5 //don't forget to change this in the ifndef __MAIN__ part (see below), it took me 2h to figure it out, I'm depressed (Serial.print shows this value, but the program is using the other), don't make that mistake
#define MAX_NB_Y_APPSWAPER 5 //don't forget to change this in the ifndef __MAIN__ part (see below), it took me 2h to figure it out, I'm depressed (Serial.print shows this value, but the program is using the other), don't make that mistake
EXTERN String defaultAppSwaperAppPositions[MAX_NB_X_APPSWAPER][MAX_NB_Y_APPSWAPER] = {
  {"/GO_DEFAULT", "/GO_DEFAULT", "/GO_DEFAULT", "/GO_DEFAULT", "/GO_DEFAULT"},
  {"/GO_DEFAULT", "/GO_DEFAULT", "WifiRemote App", "/GO_DEFAULT", "/GO_DEFAULT"},
  {"/GO_DEFAULT", "Calc App", "Clock App", "Settings App", "/GO_DEFAULT"},
  {"/GO_DEFAULT", "/GO_DEFAULT", "StopWatch App", "/GO_DEFAULT", "/GO_DEFAULT"},
  {"/GO_DEFAULT", "/GO_DEFAULT", "/GO_DEFAULT", "/GO_DEFAULT", "/GO_DEFAULT"}
 };
EXTERN int defaultAppSwaperCurrentAppXPosition = 1;
EXTERN int defaultAppSwaperCurrentAppYPosition = 1;

EXTERN BLEUUID serviceUUID = BLEUUID::fromString("d3bde760-c538-11ea-8b6e-0800200c9a66");
EXTERN BLEUUID charUUID = BLEUUID::fromString("d3bde760-c538-11ea-8b6e-0800200c9a67");

#endif
#ifndef __MAIN__
EXTERN std::string watch_name;
EXTERN String defaultAppName;
EXTERN uint8_t screen_brightness;
EXTERN uint32_t m_tz;
EXTERN void (*current_app)(AppState);
EXTERN void (*current_gesture_handler)(int);
EXTERN int menu_press_mode;
EXTERN uint8_t on_battery_screen_brightness;
EXTERN uint8_t screensaver_timeout;
#define MAX_NB_AP 2
EXTERN AP AccessPoints[MAX_NB_AP];
#define MAX_NB_X_APPSWAPER 5
#define MAX_NB_Y_APPSWAPER 5
EXTERN String defaultAppSwaperAppPositions[MAX_NB_X_APPSWAPER][MAX_NB_Y_APPSWAPER];
EXTERN int defaultAppSwaperCurrentAppXPosition;
EXTERN int defaultAppSwaperCurrentAppYPosition;
EXTERN BLEUUID serviceUUID;
EXTERN BLEUUID charUUID;
#endif

#define NB_APP 8
EXTERN App Applications[NB_APP]
#ifdef __MAIN__
= {
  { "Clock App", &appClock},
  { "Battery App", &appBattery},
  { "Test App", &appTest},
  { "Calc App", &appCalc},
  { "StopWatch App", &appStopWatch},
  { "WifiRemote App", &appWifiRemote},
  { "Settings App", &appSettings},
  { "Clock2 App", &appClock2}
}
#endif
;
//end config values

//other values

EXTERN TTGOClass *ttgo;
EXTERN TFT_eSPI *tft;
EXTERN AXP20X_Class *power;
EXTERN bool watch_on;

#define JSON_SETTINGS_SIZE 2048
EXTERN StaticJsonDocument<JSON_SETTINGS_SIZE> json_settings;

EXTERN char buff[512];

EXTERN unsigned int half_width;
EXTERN unsigned int half_height;

EXTERN int last_gesture;

EXTERN std::vector<void(*)(AppState)> app_stack;

EXTERN boolean charge_cable_connected;
EXTERN uint32_t last_activity;

#endif // MWATCH_H
