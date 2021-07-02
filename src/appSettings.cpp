#include "config.h"
#include "MWatch.h"

#include <BLEDevice.h>

static bool m_need_wifi = false;
static bool m_loading = false;

static lv_obj_t *main_page;
void appSettings(AppState);

static lv_obj_t *menu_screen_page;
static void menu_screen_app(AppState);

static lv_obj_t *menu_time_page;
static void menu_time_app(AppState);

static lv_obj_t *menu_ble_test_page;
static void menu_ble_test_app(AppState);

static lv_obj_t *menu_ble_page;
static void menu_ble_app(AppState);

//warning if you want more than 6 menus, you must turn handle_lvgl_for_app(s, page, true, true); to handle_lvgl_for_app(s, page, true, false); (to desactivate the app swaper) or reduce button height
#define NB_SETTINGS_BUTTONS_STR 6
static const char *settings_buttons_str[NB_SETTINGS_BUTTONS_STR] = {
  "screen",
  "time",
  "BLE",
  "WiFi",
  "commit settings changes",
  "restart watch"
};


static void btn_event_handler(lv_obj_t *obj, lv_event_t event) {
  if(event == LV_EVENT_CLICKED) {
    for(int i = 0; i < NB_SETTINGS_BUTTONS_STR; i ++) {
      if(String(settings_buttons_str[i]) == String(lv_list_get_btn_text(obj))) {
        Serial.println();
        Serial.print("[SETTINGS] going into " + String(settings_buttons_str[i]) + " menu");
        if(String(settings_buttons_str[i]) == "screen") {
          stack_app(menu_screen_app);
        } else if(String(settings_buttons_str[i]) == "time") {
          stack_app(menu_time_app);
        } else if(String(settings_buttons_str[i]) == "commit settings changes") {
          write_settings();
        } else if(String(settings_buttons_str[i]) == "restart watch") {
          ESP.restart();
        } else if(String(settings_buttons_str[i]) == "BLE") {
          stack_app(menu_ble_app);
        }
      }
    }
  }
}

static void setupPage(bool hidden = true) {
  if(main_page != nullptr) {
    lv_obj_del(main_page);
  }
  main_page = lv_list_create(lv_scr_act(), NULL);
  lv_obj_add_style(main_page, LV_OBJ_PART_MAIN, get_lvgl_style());
  lv_obj_set_size(main_page, 240, 240);
  lv_obj_align(main_page, NULL, LV_ALIGN_CENTER, 0, 0);
  for(int i = 0; i < NB_SETTINGS_BUTTONS_STR; i ++) {
    lv_obj_t *some_list_button;
    some_list_button = lv_list_add_btn(main_page, NULL, settings_buttons_str[i]);
    lv_obj_set_event_cb(some_list_button, btn_event_handler);
    lv_obj_add_style(some_list_button, LV_BTN_PART_MAIN, get_lvgl_style());
  }

  lv_obj_set_hidden(main_page, true);
}

static void setup_menu_screen(bool hidden = true) {
  if(menu_screen_page != nullptr) {
    lv_obj_del(menu_screen_page);
  }
  menu_screen_page = lv_obj_create(lv_scr_act(), NULL);
  lv_obj_add_style(menu_screen_page, LV_OBJ_PART_MAIN, get_lvgl_style());
  //lv_obj_align(menu_screen_page, NULL, LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_size(menu_screen_page, 240, 240);

  lv_obj_t *list = lv_list_create(menu_screen_page, NULL);
  lv_obj_add_style(list, LV_OBJ_PART_MAIN, get_lvgl_style());
  lv_obj_set_size(list, 240, 240);
  lv_obj_align(list, NULL, LV_ALIGN_CENTER, 0, 0);

  lv_obj_t *britness_list_item;
  britness_list_item = lv_list_add_btn(list, NULL, (String("Brightness: ") + String(screen_brightness)).c_str());
  lv_obj_add_style(britness_list_item, LV_BTN_PART_MAIN, get_lvgl_style());

  lv_obj_t *some_list_item;
  some_list_item = lv_list_add_btn(list, NULL, "+");
  lv_obj_add_style(some_list_item, LV_BTN_PART_MAIN, get_lvgl_style());
  lv_obj_set_event_cb(some_list_item, [](lv_obj_t *obj, lv_event_t event){
    if(event == LV_EVENT_CLICKED) {
      if(screen_brightness + 20 >= 255) {
        screen_brightness = 255;
      } else {
        screen_brightness += 20;
      }
      setup_menu_screen(false);
    }
  });

  some_list_item = lv_list_add_btn(list, NULL, "-");
  lv_obj_add_style(some_list_item, LV_BTN_PART_MAIN, get_lvgl_style());
  lv_obj_set_event_cb(some_list_item, [](lv_obj_t *obj, lv_event_t event){
    if(event == LV_EVENT_CLICKED) {
      if(screen_brightness - 20 <= 1) {
        screen_brightness = 1;
      } else {
        screen_brightness -= 20;
      }      
      setup_menu_screen(false);
    }
  });

  some_list_item = lv_list_add_btn(list, NULL, (String("screen timeout: ") + String(screensaver_timeout) + String(" sec")).c_str());
  lv_obj_add_style(some_list_item, LV_BTN_PART_MAIN, get_lvgl_style());
  
  some_list_item = lv_list_add_btn(list, NULL, "+");
  lv_obj_add_style(some_list_item, LV_BTN_PART_MAIN, get_lvgl_style());
  lv_obj_set_event_cb(some_list_item, [](lv_obj_t *obj, lv_event_t event) {
    if(event == LV_EVENT_CLICKED) {
      screensaver_timeout += 1;
      setup_menu_screen(false);
    }
  });

  some_list_item = lv_list_add_btn(list, NULL, "-");
  lv_obj_add_style(some_list_item, LV_BTN_PART_MAIN, get_lvgl_style());
  lv_obj_set_event_cb(some_list_item, [](lv_obj_t *obj, lv_event_t event) {
    if(event == LV_EVENT_CLICKED) {
      if(screensaver_timeout - 1 < 2) {
        screensaver_timeout = 2;
      } else {
        screensaver_timeout -= 1;
      }
      setup_menu_screen(false);
    }
  });

  lv_obj_set_hidden(menu_screen_page, hidden);

}

static void setup_menu_time(bool hidden = true) {
  if(menu_time_page != nullptr) {
    lv_obj_del(menu_time_page);
  }
  
  menu_time_page = lv_list_create(lv_scr_act(), NULL);
  lv_obj_add_style(menu_time_page, LV_OBJ_PART_MAIN, get_lvgl_style());
  lv_obj_set_size(menu_time_page, 240, 240);
  lv_obj_align(menu_time_page, NULL, LV_ALIGN_CENTER, 0, 0);

  Date d = getDate(m_tz);
  char b[48];
  sniprintf(b, 48, "time: %d/%d/%d, %d:%d:%d", d.dday, d.mmonth, d.yyear, d.hh, d.mm, d.ss);

  lv_obj_t *some_button = lv_list_add_btn(menu_time_page, NULL, b);
  lv_obj_add_style(some_button, LV_BTN_PART_MAIN, get_lvgl_style());

  some_button = lv_list_add_btn(menu_time_page, NULL, "set time from web");
  lv_obj_add_style(some_button, LV_BTN_PART_MAIN, get_lvgl_style());
  lv_obj_set_event_cb(some_button, [](lv_obj_t *obj, lv_event_t event){
    if(event == LV_EVENT_CLICKED) {
      m_loading = true;
      m_need_wifi = true;
      setup_menu_time(false);
    }
  });

  if(m_loading) {
    lv_obj_t *spinner = lv_spinner_create(menu_time_page, NULL);
    lv_obj_add_style(spinner, LV_SPINNER_PART_INDIC , get_lvgl_style());
    lv_obj_add_style(spinner, LV_SPINNER_PART_BG , get_lvgl_style());
    lv_obj_set_size(spinner, 100, 100);
    lv_obj_align(spinner, NULL, LV_ALIGN_CENTER, 0, 0);
  }
  
  lv_obj_set_hidden(menu_time_page, hidden);
}

static void setup_menu_ble(bool hidden = true) {
  if(menu_ble_page != nullptr) {
    lv_obj_del(menu_ble_page);
  }

  menu_ble_page = lv_list_create(lv_scr_act(), NULL);
  lv_obj_add_style(menu_ble_page, LV_OBJ_PART_MAIN, get_lvgl_style());
  lv_obj_set_size(menu_ble_page, 240, 240);
  lv_obj_align(menu_ble_page, NULL, LV_ALIGN_CENTER, 0, 0);

  lv_obj_t *some_button;
  if(json_settings["BLE_enable"].is<bool>() && json_settings["BLE_enable"].as<bool>()) {
    some_button = lv_list_add_btn(menu_ble_page, NULL, "disable ble use");
    lv_obj_set_event_cb(some_button, [](lv_obj_t *obj, lv_event_t event){
      if(event == LV_EVENT_CLICKED) {
        disable_ble();
        setup_menu_ble(false);
      }
    });
  } else {
    some_button = lv_list_add_btn(menu_ble_page, NULL, "enable ble use");
    lv_obj_set_event_cb(some_button, [](lv_obj_t *obj, lv_event_t event){
      if(event == LV_EVENT_CLICKED) {
        enable_ble();
        setup_menu_ble(false);
      }
    });
  }
  lv_obj_add_style(some_button, LV_BTN_PART_MAIN, get_lvgl_style());


  some_button = lv_list_add_btn(menu_ble_page, NULL, "serviceUUID:");
  lv_obj_add_style(some_button, LV_BTN_PART_MAIN, get_lvgl_style());

  if(json_settings["serviceUUID"].is<std::string>())
  some_button = lv_list_add_btn(menu_ble_page, NULL, json_settings["serviceUUID"].as<std::string>().c_str());
  lv_obj_add_style(some_button, LV_BTN_PART_MAIN, get_lvgl_style());

  some_button = lv_list_add_btn(menu_ble_page, NULL, "charUUID:");
  lv_obj_add_style(some_button, LV_BTN_PART_MAIN, get_lvgl_style());

  if(json_settings["charUUID"].is<std::string>())
  some_button = lv_list_add_btn(menu_ble_page, NULL, json_settings["charUUID"].as<std::string>().c_str());
  lv_obj_add_style(some_button, LV_BTN_PART_MAIN, get_lvgl_style());

  some_button = lv_list_add_btn(menu_ble_page, NULL, "test ble");
  lv_obj_set_event_cb(some_button, [](lv_obj_t *obj, lv_event_t event){
    if(event == LV_EVENT_CLICKED) {
      stack_app(menu_ble_test_app);
    }
  });
  lv_obj_add_style(some_button, LV_BTN_PART_MAIN, get_lvgl_style());

  lv_obj_set_hidden(menu_ble_page, hidden);
}

static void setup_menu_ble_test(bool hidden = true) {
  if(menu_ble_test_page != nullptr) {
    lv_obj_del(menu_ble_test_page);
  }

  menu_ble_test_page = lv_list_create(lv_scr_act(), NULL);
  lv_obj_add_style(menu_ble_test_page, LV_OBJ_PART_MAIN, get_lvgl_style());
  lv_obj_set_size(menu_ble_test_page, 240, 240);
  lv_obj_align(menu_ble_test_page, NULL, LV_ALIGN_CENTER, 0, 0);

  lv_obj_t *some_button;

  some_button = lv_list_add_btn(menu_ble_test_page, NULL, "hello test (serial print)");
  lv_obj_set_event_cb(some_button, [](lv_obj_t *obj, lv_event_t event){
    if(event == LV_EVENT_CLICKED) {
      add_ble_command("/hello");
    }
  });
  lv_obj_add_style(some_button, LV_BTN_PART_MAIN, get_lvgl_style());

  some_button = lv_list_add_btn(menu_ble_test_page, NULL, "play");
  lv_obj_set_event_cb(some_button, [](lv_obj_t *obj, lv_event_t event){
    if(event == LV_EVENT_CLICKED) {
      add_ble_command("/play");
    }
  });
  lv_obj_add_style(some_button, LV_BTN_PART_MAIN, get_lvgl_style());

  some_button = lv_list_add_btn(menu_ble_test_page, NULL, "pause");
  lv_obj_set_event_cb(some_button, [](lv_obj_t *obj, lv_event_t event){
    if(event == LV_EVENT_CLICKED) {
      add_ble_command("/pause");
    }
  });
  lv_obj_add_style(some_button, LV_BTN_PART_MAIN, get_lvgl_style());

  some_button = lv_list_add_btn(menu_ble_test_page, NULL, "nextSong");
  lv_obj_set_event_cb(some_button, [](lv_obj_t *obj, lv_event_t event){
    if(event == LV_EVENT_CLICKED) {
      add_ble_command("/nextSong");
    }
  });
  lv_obj_add_style(some_button, LV_BTN_PART_MAIN, get_lvgl_style());

  some_button = lv_list_add_btn(menu_ble_test_page, NULL, "prevSong");
  lv_obj_set_event_cb(some_button, [](lv_obj_t *obj, lv_event_t event){
    if(event == LV_EVENT_CLICKED) {
      add_ble_command("/prevSong");
    }
  });
  lv_obj_add_style(some_button, LV_BTN_PART_MAIN, get_lvgl_style());

  some_button = lv_list_add_btn(menu_ble_test_page, NULL, "time (serial print)");
  lv_obj_set_event_cb(some_button, [](lv_obj_t *obj, lv_event_t event){
    if(event == LV_EVENT_CLICKED) {
      add_ble_command("/time");
    }
  });
  lv_obj_add_style(some_button, LV_BTN_PART_MAIN, get_lvgl_style());

  lv_obj_set_hidden(menu_ble_test_page, hidden);
}

static void menu_time_app(AppState s) {
  handle_wifi_for_app(s, m_need_wifi);
  handle_lvgl_for_app(s, menu_time_page, true, true);
  if(s == HANDLE) {
    m_idle();
    Date d = getDate(m_tz);
    static int last_sec;
    if(d.ss != last_sec) {
      last_sec = d.ss;
      setup_menu_time(false);
    }
  } else if(s == DELETE) {
    m_need_wifi = false;
    m_loading = false;
  } else if(s == WIFI_CONNECTED) {
    set_rtc_time_from_web();
    m_need_wifi = false;
    m_loading = false;
  }
}

static void menu_screen_app(AppState s) {
  handle_lvgl_for_app(s, menu_screen_page, true, true);
  if(s == HANDLE) {
    m_idle();
  }
}

static void ble_callback(String data, String command) {
  if(command == "/hello") {
    Serial.print("\nhello test successful, ble working !");
  }
}

static void menu_ble_test_app(AppState s) {
  handle_lvgl_for_app(s, menu_ble_test_page, true, true);
  if(s == HANDLE) {
    m_idle();
  } else if(s == INIT) {
    add_ble_cb(ble_callback, "menu_ble_test_app_cb");
  } else if(s == DELETE) {
    remove_ble_cb("menu_ble_test_app_cb");
  }
}


static void menu_ble_app(AppState s) {
  handle_lvgl_for_app(s, menu_ble_page, true, true);
  if(s == HANDLE) {
    m_idle();
  }
}
void appSettings(AppState s) {
    handle_lvgl_for_app(s, main_page, true, true);
    if(s == HANDLE) {
      m_idle();
    } else if(s == SETUP) {
      setupPage();
      setup_menu_screen();
      setup_menu_time();
      setup_menu_ble();
      setup_menu_ble_test();
    }
}