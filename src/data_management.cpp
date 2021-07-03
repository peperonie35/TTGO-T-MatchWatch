#include "config.h"
#include "MWatch.h"

#include <ArduinoJson.h>
#include <string>
#include "SPIFFS.h"
#include "FS.h"
#include <EEPROM.h>
#include <BLEUUID.h>


String read_file_string(String path) {
    fs::File file = SPIFFS.open(path, FILE_READ);
    if(!file || file.isDirectory()) {
        Serial.printf("\nFailed to open %s file for reading", path.c_str());
        return "Error";
    }
    String data = file.readString();
    file.close();
    return data;
}

void write_file_string(String path, String &data) {
    fs::File file = SPIFFS.open(path, FILE_WRITE);
    if(!file || file.isDirectory()) {
        Serial.printf("\nFailed to open %s file for writing", path.c_str());
        return;
    }
    if(file.print(data)) {
        Serial.printf("\nFile %s written", path.c_str());
    } else {
        Serial.printf("\nError while writing %s file", path.c_str());
    }
    file.close();
}

void read_settings() {

    //String str_file = EEPROM.readString(0);
    String str_file = read_file_string("/settings.json");

    Serial.println();
    Serial.print(str_file);

    DeserializationError error = deserializeJson(json_settings, str_file);

    if (error) {
        Serial.print("\nfailed to deserialize settings\n");
        Serial.print(error.f_str());
        write_settings();
        return;
    }

    if(json_settings["watch_name"].is<std::string>())
    watch_name = json_settings["watch_name"].as<std::string>();

    if(json_settings["screen_brightness"].is<uint8_t>())
    screen_brightness = json_settings["screen_brightness"].as<uint8_t>();

    if(json_settings["m_tz"].is<uint32_t>())
    m_tz = json_settings["m_tz"].as<uint32_t>();

    if(json_settings["screensaver_timeout"].is<uint8_t>())
    screensaver_timeout = json_settings["screensaver_timeout"].as<uint8_t>();

    if(json_settings["serviceUUID"].is<std::string>())
    serviceUUID = BLEUUID::fromString(json_settings["serviceUUID"].as<std::string>());

    if(json_settings["charUUID"].is<std::string>())
    charUUID = BLEUUID::fromString(json_settings["charUUID"].as<std::string>());

    if(json_settings["AccessPoints"].is<JsonArray>()) {
        for(int i = 0; i < MAX_NB_AP; i ++) {
            if(json_settings["AccessPoints"][i].is<JsonArray>()) {
                if(json_settings["AccessPoints"][i][0].is<String>())
                AccessPoints[i].ssid = json_settings["AccessPoints"][i][0].as<String>();
                
                if(json_settings["AccessPoints"][i][1].is<String>())
                AccessPoints[i].password = json_settings["AccessPoints"][i][1].as<String>();
            }
        }
    }

    if(json_settings["defaultAppSwaperAppPositions"].is<JsonArray>()){
        for(int i = 0; i < MAX_NB_X_APPSWAPER; i ++) {
            if(json_settings["defaultAppSwaperAppPositions"][i].is<JsonArray>()) {
                for(int k = 0; k < MAX_NB_Y_APPSWAPER; k++) {
                    if(json_settings["defaultAppSwaperAppPositions"][i][k].is<String>())
                    defaultAppSwaperAppPositions[i][k] = json_settings["defaultAppSwaperAppPositions"][i][k].as<String>();
                }
            }
        }
    }

    if(json_settings["defaultAppName"].is<String>()) {
        defaultAppName = json_settings["defaultAppName"].as<String>();
        current_app = get_app_by_name(defaultAppName).app_ptr;
        defaultAppSwaperCurrentAppXPosition = get_app_x_position_by_name(defaultAppName);
        defaultAppSwaperCurrentAppYPosition = get_app_y_position_by_name(defaultAppName);
    }
}

void write_settings() {
    json_settings["watch_name"] = watch_name;
    
    json_settings["screen_brightness"] = screen_brightness;

    json_settings["m_tz"] = m_tz;

    json_settings["screensaver_timeout"] = screensaver_timeout;

    json_settings["serviceUUID"] = serviceUUID.toString();

    json_settings["charUUID"] = charUUID.toString();

    for(int i = 0; i < MAX_NB_AP; i ++) {
        json_settings["AccessPoints"][i][0] = AccessPoints[i].ssid;
        json_settings["AccessPoints"][i][1] = AccessPoints[i].password;
    }

    for(int i = 0; i < MAX_NB_X_APPSWAPER; i ++) {
        for(int k = 0; k < MAX_NB_Y_APPSWAPER; k++) {
            json_settings["defaultAppSwaperAppPositions"][i][k] = defaultAppSwaperAppPositions[i][k];
        }
    }

    json_settings["defaultAppName"] = defaultAppName;

    String out;
    serializeJson(json_settings, out);

    //EEPROM.writeString(0, out);
    //EEPROM.commit();
    write_file_string("/settings.json", out);

    Serial.print("\nSettings saved");
}