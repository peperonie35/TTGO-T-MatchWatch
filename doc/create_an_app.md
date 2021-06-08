
<h3>Make an app:</h3>

For easy implementation I advise you to use the following steps to create an app:

1-Create an [file_to_store_your_app].cpp and include "MWatch.h" and "config.h" into it

2-Create a function in this file: void appName(AppState s) {}

3-declare your app with the others in "MWatch.h" by adding void appName(AppState); at the start, and near the bottom of the file, by adding this line in the Applications table : {"your app name", &appName} and adding 1 to the #define NB_APP (ex if you have #define NB_APP 7 modify to #define NB_APP 8), now your app will be called in void setup() with s==SETUP

4-now when you will call changeCurrentApp("your app name"); it will change the current_app to your app, therefor, the function of the app that you made will be called when the app needs to show up with s==INIT, and when the app needs to be hidden with s==DELETE (please note that by default the current_app is reset to the clock app when the watch goes to sleep) and with s==HANDLE at each loop in void loop()

5-if you want your app to show up when u swipe you have to change on of the name in the defaultAppSwaperAppPositions table to the name of your app (there it's "your app name")
