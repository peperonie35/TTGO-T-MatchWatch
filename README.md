# MatchWatch
This project is inspired by https://github.com/wfdudley/T-watch-2020 It's a software for the TTGO T-watch 2020 v1

<h2>How to install ?</h2>

<h3>VS CODE:</h3>

1-install visual studio code

2-install the platform.io extention

3-download the project source code

4-open it with platform.io in visual studio code.

<h3>ARDUINO IDE:</h3>

1-download the project source code

2-rename src/MatWatch.cpp to src/MatWatch.ino

3-open with the arduino IDE.

4-make sure to install the following libraries: TTGO TWatch Library, ArduinoJson, AceTime, NTPClient

<h2>How to use ?</h2>

<h3>Make an app:</h3>

For easy implementation I advise you to use the following steps to create an app:

1-Create an [file_to_store_your_app].cpp and include "MWatch.h" and "config.h" into it

2-Create a function in this file: void appName(AppState s) {}

3-declare your app with the others in "MWatch.h" by adding void appName(AppState); near the bottom of the file, adding this line in the Applications table : {"your app name", &appName} and adding 1 to the #define NB_APP (ex if you have #define NB_APP 7 modify to #define NB_APP 8), now your app will be called in void setup() with s==SETUP

4-now when you will call changeCurrentApp("your app name"); (it will change the current_app to it), the function (or app) that you made will be called when the app needs to show up with s==INIT, and when the app needs to be hidden with s==DELETE (please note that by default the current_app is reset to the clock app when the watch goes to sleep)

5-if you want your app to show up when u swipe you have to change on of the name in the defaultAppSwaperAppPositions table to the name of your app (there it's "your app name")
