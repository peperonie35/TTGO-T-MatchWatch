# MatWatch
This project is a software for the TTGO T-watch 2020 v1 (should also work for v2 and v3, but make sure to modify config.h)

It is a sketch for the arduino library but offers easy ways to create new apps and features, it's very customisable

I took bits from https://github.com/wfdudley/T-watch-2020 (I stole some functions and stuff)

It uses the following libraries: TTGO TWatch Library, ArduinoJson, AceTime, NTPClient

<h2>How to install ?</h2>

<h3>VS CODE:</h3>

1-install visual studio code

2-install the platform.io extention

3-download the project source code

4-open it with platform.io in visual studio code.

5-after changing your settings in data/settings.json build and send the filesystem image and then send the code to the esp32 (not mandatory thought, you can change settings in the watch or in the MWatch.h file)

<h3>ARDUINO IDE:</h3>

1-download the project source code

2-rename src/MatWatch.cpp to src/MatWatch.ino

3-open with the arduino IDE.

4-make sure to install the following libraries: TTGO TWatch Library, ArduinoJson, AceTime, NTPClient

5-after changing your settings in data/settings.json build and send the filesystem image and then send the code to the esp32 (not mandatory thought, you can change settings in the watch or in the MWatch.h file)

<h2>How to use ?</h2>

create an app: [here](doc/create_an_app.md)

I will add more doc later but for now just read the MWatch.h file, there is bits of infomation here, also read the apps files to understand how an app works

<h2>Features:</h2>

an intuitive system that manages apps (app swipe and more)

Bluetooth (I have a working companion app, with some bugs, didn't published it yet, ask me if you want it)

easy way to manage wifi for apps

easy way to use lvgl for apps

easy way to create new apps

easy way to store data or settings (with SPIFFS)

apps: stop watch app, wifi remote app, calc app, battery info app, settings app

<h2>I plan to add:</h2>

Alarm to wake me up (didn't start working on that)

<h2>Known issues that I probably will never fix</h2>

Somtimes the appWifiRemote crashes with error "Cache disabled but cached memory region accessed", it seems related to touch feature (in the trace)
