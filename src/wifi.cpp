#include "config.h"
#include "MWatch.h"
#include <time.h>
#include <WiFi.h>
#include "esp_wifi.h"
#include <WiFiUdp.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <NTPClient.h>

static bool m_wifi_on = false;

//makes a basic get http request and returns the result
String make_http_get_request(String url) {
  HTTPClient http;
  http.begin(url);
  int httpCode = http.GET();
  Serial.println();
  Serial.printf("[HTTP] GET %d", httpCode);
  String payload = "";
  if (httpCode > 0) {
    payload = http.getString();
  }
  else {
    Serial.println("[HTTP] Error on HTTP request");
  }
  http.end();
  return payload;
}

//request date from web and set the rtc date and to it, not done yet
void set_rtc_time_from_web() {
  WiFiUDP ntpUDP;
  NTPClient timeClient(ntpUDP);
  timeClient.begin();
  timeClient.forceUpdate();
  time_t rawtime = timeClient.getEpochTime();
  struct tm *ti;
  ti = localtime(&rawtime);
  Serial.println();
  Serial.print(timeClient.getFormattedTime());
  ttgo->rtc->setDateTime(ti->tm_year + 1900, ti->tm_mon + 1, ti->tm_mday, ti->tm_hour, ti->tm_min, ti->tm_sec);
  ttgo->rtc->syncToSystem();
}

//handle every wifi control stuff for an app,run in the app to enable wifi for the app, put nee_wifi to false when wifi is not needed to reduce power consumption
void handle_wifi_for_app(AppState s, bool need_wifi) {
  if (s == AppState::INIT) {
    if (need_wifi) {
      m_wifi_on = true;
      begin_network_scan();
    }
  } else if (s == AppState::WIFI_SCAN_ENDED) {
    if (need_wifi) {
      m_wifi_on = true;
      connect_to_wifi_ap(get_ap_if_wifi_scan_match());
    }
  } else if (s == AppState::DELETE) {
    turn_off_wifi();
    m_wifi_on = false;
  } else if(s == AppState::HANDLE && need_wifi && !m_wifi_on) {
    m_wifi_on = true;
    begin_network_scan();
  }
  if (!need_wifi && m_wifi_on == true) {
    m_wifi_on = false;
    turn_off_wifi();
  }
}

//handle wifi event to notify current app about changes
void WifiEventHandler(WiFiEvent_t event) {
  switch (event) {
    case SYSTEM_EVENT_STA_GOT_IP:
      Serial.println();
      Serial.print(F("WiFi connected! IP address: "));
      Serial.println(WiFi.localIP());
      current_app(AppState::WIFI_CONNECTED);
      break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
      Serial.println();
      Serial.println(F("WiFi lost connection"));
      current_app(AppState::WIFI_DISCONNECTED);
      break;
    case SYSTEM_EVENT_SCAN_DONE:
      Serial.println();
      Serial.println(F("Wifi scan done"));
      current_app(AppState::WIFI_SCAN_ENDED);
      break;
    default:
      break;
  }
}

//check the wifi scan and return the index of an Access point if it is known
int get_ap_if_wifi_scan_match() {
  int n = WiFi.scanComplete();
  for (int i = 0; i < n; i ++) {
    for (int k = 0; k < MAX_NB_AP; k ++) {
      if (AccessPoints[k].ssid == WiFi.SSID(i)) {
        return k;
      }
    }
  }
  return -1;
}

//setup wifi handle and set wifi to low to reduce power consumption
void wifi_setup() {
  WiFi.mode(WIFI_OFF);
  WiFi.onEvent(WifiEventHandler);
}

//start the network scan (non-blocking)
void begin_network_scan() {
  Serial.println();
  Serial.print("Scanning networks");
  current_app(AppState::WIFI_SCAN_START);
  WiFi.mode(WIFI_STA);
  WiFi.scanNetworks(true, true);
}

//connect to wifi using ssid and passowrd
void connect_to_wifi_ap(String ssid, String password) {
  Serial.println();
  Serial.print("Connecting to ap: " + ssid + " " + password);
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  WiFi.begin(ssid.c_str(), password.c_str());
}

//connect to wifi using Acces point index in the known list
void connect_to_wifi_ap(int k) {
  if(k == -1) {
    current_app(AppState::WIFI_NO_AVAILABLE);
  }
  Serial.println();
  Serial.print("Connecting to ap: " + AccessPoints[k].ssid + " " + AccessPoints[k].password);
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  WiFi.begin(AccessPoints[k].ssid.c_str(), AccessPoints[k].password.c_str());
}

//disconnect wifi
void disconnect_wifi_ap() {
  current_app(AppState::WIFI_DISCONNECTING);
  WiFi.disconnect();
}

//turns off wifi
void turn_off_wifi() {
  Serial.println();
  Serial.print("turning off wifi");
  esp_wifi_scan_stop();
  current_app(AppState::WIFI_DISCONNECTING);
  WiFi.disconnect();
  WiFi.mode(WIFI_OFF);
  current_app(AppState::WIFI_DISCONNECTED);
}
