
<h1>Create an app:</h1>

<h2>Baics of my shitty system:</h2>

an app (in my system) is a void function with takes an AppState (s in the exemple) as parameter: void my_app(AppState s);

each app is called in void setup() with s==SETUP
an app is called when it needs to be shown with s==INIT
an app is called when it needs to be hidden with s==DELETE
when an app is the app currently showed it is called at each loop in void loop() with s==HANDLE

to set an app to be the current app you can either call the changeCurrentApp("your app name(see later)") function or register your app in the app swiping system (it changes app when you swipe (2 dimention table of app that changes the current app following the direction of the swipe)) 

<h2>Requirements</h3>

1-Create an file_to_store_your_app.cpp and include "MWatch.h" and "config.h" into it

2-Create a function in this file: void appName(AppState s) {}

3-declare your app with the others in "MWatch.h" by adding void appName(AppState); at the start, and near the bottom of the file, by adding this line in the Applications table : {"your app name", &appName} and adding 1 to the #define NB_APP (ex if you have #define NB_APP 7 modify to #define NB_APP 8), now your app will be called in void setup() with s==SETUP

4-modify one of the app name in the defaultAppSwiperPositions table to be your app name send the program to the watch, and swipe to arrive to your app (should be a black screen if your app is plain)

<h2>How to display stuff</h2>

see appClock.cpp for an exemple

Once you have followed the requirement you now have an app registered and working, but it is black because you haven't coded what to display yet.

To display stuff you can use either the TFT_eSPI libray to draw directly on the 240*240 screen of the watch or you can use the LVLG library:

<h3>Using the TFT_eSPI libray</h3>

you can draw a keyboard with the draw_keyboard function or simply a key with flash_keyboard_item function (see MWatch.h for more informations)
if u draw a 4*3 keyboard set menu_press_mode to 12 or set to 16 if 4*4 keyboard
u can acess the last touch on the keyboard with last_gesture variable (from 0 to 11 or 15)
to disable the app swiping and menu press detection use disable_touch_features_for_app(s) function (put it in the first lines of the program)

<h3>Using LVGL</h3>

see appSettings.cpp for an example

use the handle_lvgl_for_app function to use LVGL, there is two variants of this function:

void handle_lvgl_for_app(AppState s, lv_obj_t *page, bool disable_default_gesture, bool enable_default_app_swipe);

there you have to give the appState and a ptr to the main lv_obj_t of your app, you have to set disable_default_gesture to true if you want to use any touch feature of LVGL, also, set enable_default_app_swipe to true to keep the default app swiper enable 

void handle_lvgl_for_app(AppState s, void(*__show)(), void(*__hide)(), bool disable_default_gesture, bool enable_default_app_swipe)

there, same as before, but instead of a ptr to an lv_obj_t you have to setup 2 functions that will show and hide your app


