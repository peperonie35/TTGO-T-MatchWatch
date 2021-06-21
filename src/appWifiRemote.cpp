#include "config.h"
#include "MWatch.h"


enum DisplaySate {LOADING = 1, MENU, REQUESTING, NO_WIFI};

static DisplaySate disp_state;
static uint32_t last_update = 0;
static int nb_load_pt = 0;
static bool m_need_wifi = true;

static const String api_url = "put_url_here";
static const String api_key = "there_is_no_key_yet";

const char*wifiRemoteLabels[] = {
    "light on", "", "light off",
    "alarm on", "", "alarm off",
    "", "", "",
    "", "", ""
};

const char*wifiRemoteMatchingUrls[] = {
    "/light/alarm/turnon", "", "/light/alarm/turnoff",
    "/alarm/turnon", "", "/alarm/turnoff",
    "", "", "",
    "", "", ""
};

void change_disp_sate(DisplaySate s) {
    if(s == LOADING) {
        tft->fillScreen(TFT_BLACK);
        tft->drawCentreString("Chargement", half_width, half_height, 4);
        last_update = millis();
    }
    if(s == MENU) {
        tft->fillScreen(TFT_BLACK);
        menu_press_mode = 12;
        draw_keyboard(12, wifiRemoteLabels, 2, true, "WiFi remote");
    }
    if(s == REQUESTING) {
        tft->fillScreen(TFT_BLACK);
        tft->drawCentreString("Veuillez patienter", half_width, half_height, 4);
    }
    if(s == NO_WIFI) {
        tft->fillScreen(TFT_BLACK);
        tft->drawCentreString("Pas de WiFi", half_width, half_height, 4);
    }
    disp_state = s;
}

void appWifiRemote(AppState s) {
    handle_wifi_for_app(s, m_need_wifi);
    if(s == HANDLE) {
        m_idle(); 
        if(last_update + 1000 < millis() && disp_state == LOADING) {
            last_update = millis();
            nb_load_pt += 1;
            if(nb_load_pt > 3) {
                nb_load_pt = 1;
            }
            String out_str = "Chargement";
            for(int i = 0; i < nb_load_pt; i ++) {
                out_str += ".";
            }
            tft->fillScreen(TFT_BLACK);
            tft->drawCentreString(out_str, half_width, half_height, 4);
        }
        if(disp_state == MENU) {
            if(last_gesture < 11 && last_gesture > -1) {
                String url_to_add = wifiRemoteMatchingUrls[last_gesture];
                if(!(url_to_add == "" || url_to_add == "NONE")) {
                    String final_url = api_url + url_to_add + "?key=" + api_key;
                    change_disp_sate(REQUESTING);
                    String r = make_http_get_request(final_url);
                    tft->fillScreen(TFT_BLACK);
                    tft->drawCentreString(r, half_width, half_height, 4);
                    last_update = millis();
                }
            }
        }
        if(disp_state == REQUESTING && last_update + 1000 < millis()) {
            change_disp_sate(MENU);
        }
    } else if( s == INIT) {
        tft->fillScreen(TFT_BLACK);
        change_disp_sate(LOADING);
    } else if (s == DELETE) {
        tft->fillScreen(TFT_BLACK);
    } else if (s == WIFI_CONNECTED) {
        change_disp_sate(MENU);
    } else if (s == WIFI_DISCONNECTED) {
        change_disp_sate(LOADING);
    } else if (s == WIFI_NO_AVAILABLE) {
        change_disp_sate(NO_WIFI);
    }
}