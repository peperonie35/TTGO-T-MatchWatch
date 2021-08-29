#include "config.h"
#include "MWatch.h"

static String password = "1234";
static String working_pasword = "";
static bool wifi_connected = false;

static char *wifi_label_txt = LV_SYMBOL_REFRESH;

static String api_url = "http://91.167.236.217:32000/api";

static lv_obj_t *main_page;

static bool shall_show_prompt = false;
static String prompt_txt;

static bool shall_show_pasword_prompt = false;
static const char * btnm_map[] = {"1", "2", "3", "4", "5", "\n",
                                  "6", "7", "8", "9", "0", "\n",
                                  LV_SYMBOL_CLOSE, LV_SYMBOL_OK, LV_SYMBOL_BACKSPACE, ""};

static void(*pending_password_action)();
static String pending_end_point;

static void setup_main_page(bool hidden = true);

static String read_password() {
    if(json_settings["alarmControllerPassword"].is<String>()) {
        return json_settings["alarmControllerPassword"].as<String>();
    }
}

static void change_password(String current, String new_one) {
    if(json_settings["alarmControllerPassword"].is<String>()) {
        if(json_settings["alarmControllerPassword"].as<String>() == current) {
            json_settings["alarmControllerPassword"] = new_one;
        }
    }
}

static void show_password_prompt() {
    shall_show_pasword_prompt = true;
    setup_main_page(false);
}

static void hide_password_prompt() {
    shall_show_pasword_prompt = false;
    setup_main_page(false);
}


static void hide_prompt() {
    shall_show_prompt = false;
    setup_main_page(false);
}

static void show_prompt(String txt) {
    shall_show_prompt = true;
    prompt_txt = txt;
    setup_main_page(false);
}

static void request_api(String end_point) {
    if(!wifi_connected) {
        show_prompt(" pas de connexion !\nappuyez pour fermer");
    } else {
        pending_end_point = end_point;
        pending_password_action = [](){
            make_http_get_request(api_url + pending_end_point);
        };
        show_password_prompt();
    }
}

static void setup_main_page(bool hidden) {
    if(main_page != nullptr) {
        lv_obj_del(main_page);
    }

    main_page = lv_obj_create(lv_scr_act(), NULL);
    lv_obj_add_style(main_page, LV_OBJ_PART_MAIN, get_lvgl_style());
    lv_obj_set_size(main_page, 240, 240);
    lv_obj_align(main_page, lv_scr_act(), LV_ALIGN_CENTER, 0, 0);

    lv_obj_t *wifi_label = lv_label_create(main_page, NULL);
    lv_obj_add_style(wifi_label, LV_LABEL_PART_MAIN, get_lvgl_style());
    lv_obj_align(wifi_label, main_page, LV_ALIGN_IN_TOP_RIGHT, 0, 0);
    lv_label_set_text(wifi_label, wifi_label_txt);

    lv_obj_t *activate_alarm_button = lv_btn_create(main_page, NULL);
    lv_obj_t *activate_alarm_label = lv_label_create(activate_alarm_button, NULL);
    lv_label_set_text(activate_alarm_label, "activer l'alarme");
    lv_obj_set_style_local_bg_color(activate_alarm_button, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_RED);
    lv_obj_set_style_local_text_color(activate_alarm_button, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_obj_align(activate_alarm_button, main_page, LV_ALIGN_CENTER, 0, 40);
    lv_obj_set_event_cb(activate_alarm_button, [](lv_obj_t *obj, lv_event_t event){
        if(event == LV_EVENT_CLICKED) {
            request_api("/control/alarm/masterbedroom/turnon");
        }
    });

    lv_obj_t *disactivate_alarm_button = lv_btn_create(main_page, NULL);
    lv_obj_t *disactivate_alarm_label = lv_label_create(disactivate_alarm_button, NULL);
    lv_label_set_text(disactivate_alarm_label, "déactiver l'alarme");
    lv_obj_set_style_local_bg_color(disactivate_alarm_button, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GREEN);
    lv_obj_set_style_local_text_color(disactivate_alarm_button, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_obj_align(disactivate_alarm_button, main_page, LV_ALIGN_CENTER, 0, -40);
    lv_obj_set_event_cb(disactivate_alarm_button, [](lv_obj_t *obj, lv_event_t event){
        if(event == LV_EVENT_CLICKED) {
            request_api("/control/alarm/masterbedromm/turnoff");
        }
    });

    lv_obj_t *no_wifi_prompt = lv_btn_create(main_page, NULL);
    lv_obj_t *no_wifi_prompt_label = lv_label_create(no_wifi_prompt, NULL);
    lv_label_set_text(no_wifi_prompt_label, prompt_txt.c_str());
    lv_obj_set_style_local_bg_color(no_wifi_prompt, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_ORANGE);
    lv_obj_set_style_local_text_color(no_wifi_prompt, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_obj_set_size(no_wifi_prompt, 240, 240);
    lv_obj_align(no_wifi_prompt, main_page, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_event_cb(no_wifi_prompt, [](lv_obj_t *obj, lv_event_t event){
        if(event == LV_EVENT_CLICKED) {
          hide_prompt();
        }
    });
    if(!shall_show_prompt) {
        lv_obj_set_hidden(no_wifi_prompt, true);
    }


    lv_obj_t *keyboard = lv_btnmatrix_create(main_page, NULL);
    lv_btnmatrix_set_map(keyboard, btnm_map);
    lv_obj_set_style_local_text_color(keyboard, LV_BTNMATRIX_PART_BTN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_obj_set_style_local_bg_color(keyboard, LV_BTNMATRIX_PART_BTN, LV_STATE_DEFAULT, LV_COLOR_ORANGE);
    lv_obj_set_style_local_bg_color(keyboard, LV_BTNMATRIX_PART_BG, LV_STATE_DEFAULT, LV_COLOR_ORANGE);
    lv_obj_set_size(keyboard, 240, 240);
    lv_obj_align(keyboard, main_page, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_event_cb(keyboard, [](lv_obj_t *obj, lv_event_t event){
        if(LV_EVENT_VALUE_CHANGED == event) {
            const char *txt = lv_btnmatrix_get_active_btn_text(obj);
            if(txt == LV_SYMBOL_OK) {
                if(working_pasword == password) {
                    working_pasword =  "";
                    pending_password_action();
                    hide_password_prompt();
                } else {
                    hide_password_prompt();
                    delay(50);
                    show_prompt("mauvais mot de passe !\n  appuyer pour fermer");
                }
            } else if(txt == LV_SYMBOL_BACKSPACE) {
                String tmp_psw = "";
                for(int i = 0; i < working_pasword.length()-1; i ++) {
                    tmp_psw += working_pasword[i];
                }
                working_pasword = tmp_psw;
            } else if(txt == LV_SYMBOL_CLOSE) {
                working_pasword = "";
                hide_password_prompt();
            } else {
                working_pasword += txt;
            }
        }
    });
    if(!shall_show_pasword_prompt) {
        lv_obj_set_hidden(keyboard, true);
    }

    lv_obj_set_hidden(main_page, hidden);
}

void appAlarmController(AppState s) {
    handle_wifi_for_app(s, true);   
    handle_lvgl_for_app(s, main_page, true, false);
    m_idle();
    if(s == INIT) {
        password = read_password();
    } else if(s == DELETE) {
        write_settings();
    } else if(s == SETUP) {
        setup_main_page();
    } else if(s == WIFI_CONNECTED) {
        wifi_label_txt = LV_SYMBOL_WIFI;
        setup_main_page(false);
        wifi_connected = true;
    } else if(s == WIFI_DISCONNECTED) {
        wifi_label_txt = LV_SYMBOL_REFRESH;
        setup_main_page(false);
        wifi_connected = false;
    }
}