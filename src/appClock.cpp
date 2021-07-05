#include "config.h"
#include "MWatch.h"

#include "lvgl_declare.h"

static lv_obj_t *main_page;
static lv_obj_t *sub_app_1_page;

static const char *media_control_button_matrix[] = {LV_SYMBOL_PREV, LV_SYMBOL_PLAY, LV_SYMBOL_NEXT};
static lv_obj_t *media_control_buttons;
static bool media_buttons_hidden = true;

#define VAR_X 16
#define CHAR_SPACE 36
static lv_obj_t *hours_dec;
static lv_obj_t *hours_unit;
static lv_obj_t *separator;
static lv_obj_t *minutes_dec;
static lv_obj_t *minutes_unit;

static lv_obj_t *batterie_label;

static void change_num(lv_obj_t *img, int num) {
  if(num == 0) {
    lv_img_set_src(img, &NUM_0_IMG);
  } else if (num == 1) {
    lv_img_set_src(img, &NUM_1_IMG);
  } else if (num == 2) {
    lv_img_set_src(img, &NUM_2_IMG);
  } else if (num == 3) {
    lv_img_set_src(img, &NUM_3_IMG);
  } else if (num == 4) {
    lv_img_set_src(img, &NUM_4_IMG);
  } else if (num == 5) {
    lv_img_set_src(img, &NUM_5_IMG);
  } else if (num == 6) {
    lv_img_set_src(img, &NUM_6_IMG);
  } else if (num == 7) {
    lv_img_set_src(img, &NUM_7_IMG);
  } else if (num == 8) {
    lv_img_set_src(img, &NUM_8_IMG);
  } else if (num == 9) {
    lv_img_set_src(img, &NUM_9_IMG);
  }
}

static int old_mm = 69;
static void update_time(int hh, int mm, int ss) {
  if(old_mm != mm) {
    old_mm = mm;
    change_num(hours_dec, hh/10);
    change_num(hours_unit, hh%10);
    change_num(minutes_dec, mm/10);
    change_num(minutes_unit, mm%10);
  }
  if(ss % 2) {
    lv_obj_set_style_local_image_recolor(separator, LV_IMG_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_DARKGRAY);
  } else {
    lv_obj_set_style_local_image_recolor(separator, LV_IMG_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_AQUA);
  }
}

static void update_batterie_indicator() {
  static int batterie_level;
  if(batterie_level != ttgo->power->getBattPercentage()) {
    batterie_level = ttgo->power->getBattPercentage();
    String bs = String(batterie_level) + "%";
    lv_label_set_text(batterie_label, bs.c_str());
  }
}

static void setup_main_page(bool hidden = true) {
  if(main_page != nullptr) {
    lv_obj_del(main_page);
  }

  main_page = lv_obj_create(lv_scr_act(), NULL);
  lv_obj_add_style(main_page, LV_OBJ_PART_MAIN, get_lvgl_style());
  lv_obj_set_size(main_page, 240, 240);
  lv_obj_align(main_page, lv_scr_act(), LV_ALIGN_CENTER, 0, 0);

  hours_dec = lv_img_create(main_page, NULL);
  lv_obj_align(hours_dec, main_page, LV_ALIGN_CENTER, -(2*CHAR_SPACE) + VAR_X, 0);
  lv_img_set_src(hours_dec, &NUM_0_IMG);
  lv_obj_set_style_local_image_recolor(hours_dec, LV_IMG_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_AQUA);
  lv_obj_set_style_local_image_recolor_opa(hours_dec, LV_IMG_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_MAX);

  hours_unit = lv_img_create(main_page, NULL);
  lv_obj_align(hours_unit, main_page, LV_ALIGN_CENTER, -CHAR_SPACE + VAR_X, 0);
  lv_img_set_src(hours_unit, &NUM_0_IMG);
  lv_obj_set_style_local_image_recolor(hours_unit, LV_IMG_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_AQUA);
  lv_obj_set_style_local_image_recolor_opa(hours_unit, LV_IMG_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_MAX);

  separator = lv_img_create(main_page, NULL);
  lv_obj_align(separator, main_page, LV_ALIGN_CENTER, 0 + VAR_X, 0);
  lv_img_set_src(separator, &POINTS_IMG);
  lv_obj_set_style_local_image_recolor(separator, LV_IMG_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_AQUA);
  lv_obj_set_style_local_image_recolor_opa(separator, LV_IMG_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_MAX);
  lv_obj_add_style(separator, LV_IMG_PART_MAIN, get_lvgl_style());

  minutes_dec = lv_img_create(main_page, NULL);
  lv_obj_align(minutes_dec, main_page, LV_ALIGN_CENTER, CHAR_SPACE + VAR_X, 0);
  lv_img_set_src(minutes_dec, &NUM_0_IMG);
  lv_obj_set_style_local_image_recolor(minutes_dec, LV_IMG_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_AQUA);
  lv_obj_set_style_local_image_recolor_opa(minutes_dec, LV_IMG_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_MAX);

  minutes_unit = lv_img_create(main_page, NULL);
  lv_obj_align(minutes_unit, main_page, LV_ALIGN_CENTER, 2*CHAR_SPACE + VAR_X, 0);
  lv_img_set_src(minutes_unit, &NUM_0_IMG);
  lv_obj_set_style_local_image_recolor(minutes_unit, LV_IMG_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_AQUA);
  lv_obj_set_style_local_image_recolor_opa(minutes_unit, LV_IMG_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_MAX);

  batterie_label = lv_label_create(main_page, NULL);
  lv_label_set_text(batterie_label, "100%");
  lv_obj_align(batterie_label, main_page, LV_ALIGN_IN_TOP_RIGHT, -15, 0);
  lv_obj_set_style_local_text_font(batterie_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &lv_font_montserrat_22);
  lv_obj_set_style_local_text_color(batterie_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_DARKGRAY);
  lv_obj_set_style_local_text_color(batterie_label, LV_LABEL_PART_MAIN, LV_STATE_FOCUSED, LV_COLOR_DARKGRAY);

  media_control_buttons = lv_btnmatrix_create(main_page, NULL);
  lv_obj_set_size(media_control_buttons, 160, 40);
  lv_obj_align(media_control_buttons, main_page, LV_ALIGN_IN_BOTTOM_MID, 0, 0);
  lv_obj_add_style(media_control_buttons, LV_BTNMATRIX_PART_BG, get_lvgl_style());
  lv_obj_add_style(media_control_buttons, LV_BTNMATRIX_PART_BTN , get_lvgl_style());
  lv_btnmatrix_set_map(media_control_buttons, media_control_button_matrix);
  lv_obj_set_hidden(media_control_buttons, media_buttons_hidden);
  lv_obj_set_event_cb(media_control_buttons, [](lv_obj_t * obj, lv_event_t event){
    if(event == LV_EVENT_VALUE_CHANGED) {
      const char * txt = lv_btnmatrix_get_active_btn_text(obj);
      if(txt == LV_SYMBOL_PLAY) {
        add_ble_command("/play");
      } else if(txt == LV_SYMBOL_PAUSE) {
        add_ble_command("/pause");
      } else if(txt == LV_SYMBOL_NEXT) {
        add_ble_command("/nextSong");
      } else if(txt == LV_SYMBOL_PREV) {
        add_ble_command("/prevSong");
      }
    }
  });

  lv_obj_set_hidden(main_page, hidden);
}


static void setup_sub_app_1_page(bool hidden = true) {
  if(sub_app_1_page != nullptr) {
    lv_obj_del(sub_app_1_page);
  }

  sub_app_1_page = lv_obj_create(lv_scr_act(), NULL);
  lv_obj_add_style(sub_app_1_page, LV_OBJ_PART_MAIN, get_lvgl_style());
  lv_obj_set_size(sub_app_1_page, 240, 240);
  lv_obj_align(sub_app_1_page, lv_scr_act(), LV_ALIGN_CENTER, 0, 0);

  lv_obj_t *test_label = lv_label_create(sub_app_1_page, NULL);
  lv_obj_align(test_label, sub_app_1_page, LV_ALIGN_IN_TOP_LEFT, 0, 0);
  lv_obj_add_style(test_label, LV_LABEL_PART_MAIN, get_lvgl_style());
  lv_obj_set_style_local_text_font(test_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &lv_font_montserrat_22);
  lv_label_set_text(test_label, "Ble informations\nwill appear here");

  lv_obj_set_hidden(sub_app_1_page, hidden);
}

static void(*func_to_run_on_main_thread)() = nullptr; //to avoid crashes
static uint32_t last_press = 0;

static void sub_app_1_event_cb(lv_obj_t *obj, lv_event_t event) {
  if(event == LV_EVENT_PRESSED) {
    m_idle();
    if(millis() - last_press < 500) {
      func_to_run_on_main_thread = [](){
        unstack_app();
      };
    }
    last_press = millis();
  }
}

static void sub_app_1(AppState s) {
  handle_lvgl_for_app(s, sub_app_1_page, true, true);
  if(s == HANDLE) {
    m_idle();
    if(func_to_run_on_main_thread != nullptr) {
      func_to_run_on_main_thread();
      func_to_run_on_main_thread = nullptr;
    }
  } else if(s == SETUP) {
    setup_sub_app_1_page();
    lv_obj_set_event_cb(sub_app_1_page, sub_app_1_event_cb); //before setup, impotant
  } else if (s == INIT) {
    last_press = 0;
  }
}

static void main_page_event_cb(lv_obj_t *obj, lv_event_t event) {
  if(event == LV_EVENT_PRESSED) {
    m_idle();
    if(millis() - last_press < 500) {
      func_to_run_on_main_thread = [](){
        stack_app(sub_app_1);
      };
    }
    last_press = millis();
  }
}

static void ble_data_cb(String command, String data) {
  if(command == "isPlaying") {
    if(data == "music_playing") {
      media_control_button_matrix[1] = LV_SYMBOL_PAUSE;
    } else if(data == "music_not_playing") {
      media_control_button_matrix[1] = LV_SYMBOL_PLAY;
    }
  }
}

void appClock(AppState s) {
  handle_lvgl_for_app(s, main_page, true, true);
  if(s == HANDLE) {
    if(media_buttons_hidden == is_ble_connected()) {
      media_buttons_hidden = !media_buttons_hidden;
      lv_obj_set_hidden(media_control_buttons, media_buttons_hidden);
    }
    Date tt = getDate(m_tz);
    update_time(tt.hh, tt.mm, tt.ss);
    update_batterie_indicator();
    if(func_to_run_on_main_thread != nullptr) {
      func_to_run_on_main_thread();
      func_to_run_on_main_thread = nullptr;
    }
  } else if(s == SETUP) {
    setup_main_page();
    lv_obj_set_event_cb(main_page, main_page_event_cb); //after setup_main_page, impotant
    add_ble_cb(ble_data_cb, "appClock ble cb");
    sub_app_1(SETUP);
  } else if(s == INIT) {
    last_press = 0;
    Date tt = getDate(m_tz);
    update_time(tt.hh, tt.mm, tt.ss);
  }
}
