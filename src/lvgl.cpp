#include "config.h"
#include "MWatch.h"

static void (*lvgl_app_change_to_execute_on_main_thread)() = nullptr;
static bool m_app_enabled = false;
static lv_style_t *current_style = nullptr;

static bool lv_gesture_on = false;

lv_style_t* get_lvgl_style() {
  return current_style;
}

static void set_default_style() {
  current_style = new lv_style_t;
  lv_style_init(current_style);
  lv_style_set_bg_color(current_style, LV_STATE_DEFAULT, LV_COLOR_BLACK);
  lv_style_set_text_color(current_style, LV_STATE_DEFAULT, LV_COLOR_WHITE);
  lv_style_set_border_color(current_style, LV_STATE_DEFAULT, LV_COLOR_BLACK);
  
  lv_style_set_bg_color(current_style, LV_STATE_PRESSED, LV_COLOR_BLACK);
  lv_style_set_text_color(current_style, LV_STATE_PRESSED, LV_COLOR_BLACK);
  lv_style_set_border_color(current_style, LV_STATE_PRESSED, LV_COLOR_BLACK);

  lv_style_set_bg_color(current_style, LV_STATE_FOCUSED, LV_COLOR_BLACK);
  lv_style_set_text_color(current_style, LV_STATE_FOCUSED, LV_COLOR_BLUE);
  lv_style_set_border_color(current_style, LV_STATE_FOCUSED, LV_COLOR_BLACK);

  lv_obj_add_style(lv_scr_act(), LV_OBJ_PART_MAIN, current_style);
}

static void setup_lvgl() {
  set_default_style();
}

static void event_cb_swipe(lv_obj_t *obj, lv_event_t e) {
  if(!m_app_enabled || !lv_gesture_on) {
    return;
  }
  if(e == LV_EVENT_GESTURE) {
    lv_gesture_dir_t dir = lv_indev_get_gesture_dir(lv_indev_get_act());
    if(dir == 0) {
      lvgl_app_change_to_execute_on_main_thread = *[](){defaultAppSwaperGestureHandler(SWIPE_DIR::UP);};
    } else if (dir == 1) {
      lvgl_app_change_to_execute_on_main_thread = *[](){defaultAppSwaperGestureHandler(SWIPE_DIR::DOWN);};
    } else if (dir == 2) {
      lvgl_app_change_to_execute_on_main_thread = *[](){defaultAppSwaperGestureHandler(SWIPE_DIR::LEFT);};
    } else if (dir == 3) {
      lvgl_app_change_to_execute_on_main_thread = *[](){defaultAppSwaperGestureHandler(SWIPE_DIR::RIGHT);};
    }
  }
}

void handle_lvgl_for_app(AppState s, lv_obj_t *page, bool disable_default_gesture, bool enable_default_app_swipe) {
  if(disable_default_gesture) {
    disable_touch_features_for_app(s);
  }
  if(s == HANDLE && m_app_enabled) {
    lv_task_handler();
  } else if (s==INIT) {
    if(!enable_default_app_swipe) {
      lv_gesture_on = false;
    } else {
      lv_gesture_on = true;
    }
    lv_obj_set_hidden(page, false);
    m_app_enabled = true;
  } else if (s == DELETE) {
    m_app_enabled = false;
    lv_obj_set_hidden(page, true);
    tft->fillScreen(TFT_BLACK);
  } else if (s == SETUP && enable_default_app_swipe) {
    lv_obj_set_event_cb(lv_scr_act(), event_cb_swipe);
  }
  if (s == SETUP && current_style == nullptr) {
    setup_lvgl();
  }
  if(lvgl_app_change_to_execute_on_main_thread != nullptr && s==HANDLE) {
    lvgl_app_change_to_execute_on_main_thread();
    lvgl_app_change_to_execute_on_main_thread = nullptr;
  }
}

void handle_lvgl_for_app(AppState s, void(*__show)(), void(*__hide)(), bool disable_default_gesture, bool enable_default_app_swipe) {
  if(disable_default_gesture) {
    disable_touch_features_for_app(s);
  }
  if(s == HANDLE && m_app_enabled) {
    lv_task_handler();
  } else if (s==INIT) {
    m_app_enabled = true;
    __show();
  } else if (s == DELETE) {
    m_app_enabled = false;
    __hide();
    tft->fillScreen(TFT_BLACK);
  } else if (s == SETUP && enable_default_app_swipe) {
    lv_obj_set_event_cb(lv_scr_act(), event_cb_swipe);
  }
  if (s == SETUP && current_style == nullptr) {
    setup_lvgl();
  }
  if(lvgl_app_change_to_execute_on_main_thread != nullptr && s==HANDLE) {
    lvgl_app_change_to_execute_on_main_thread();
    lvgl_app_change_to_execute_on_main_thread = nullptr;
  }
}