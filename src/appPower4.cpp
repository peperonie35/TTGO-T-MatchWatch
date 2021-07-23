#include "config.h"
#include "MWatch.h"

static lv_obj_t *main_page = nullptr;

static lv_color_t current_color = LV_COLOR_RED;
static lv_obj_t *playing_page = nullptr;

#define NB_X_POWER_4_CASE 7
#define NB_Y_POWER_4_CASE 6
static int power_4_grid_map[NB_Y_POWER_4_CASE][NB_X_POWER_4_CASE] = {
    {0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0}
};

#define DELTA 17

static void setup_playing_page(bool hidden = true) {
    if(playing_page != nullptr) {
        lv_obj_del(playing_page);
    }

    playing_page = lv_obj_create(lv_scr_act(), NULL);
    lv_obj_add_style(playing_page, LV_OBJ_PART_MAIN, get_lvgl_style());
    lv_obj_set_size(playing_page, 240, 240);
    lv_obj_align(playing_page, lv_scr_act(), LV_ALIGN_CENTER, 0, 0);


    for(int i = 0; i < NB_Y_POWER_4_CASE; i ++) {
        for(int k = 0; k < NB_X_POWER_4_CASE; k ++) {
            lv_obj_t *l = lv_label_create(playing_page, NULL);
            lv_obj_set_size(l, 32, 32);
            lv_label_set_text(l, "O");
            lv_obj_add_style(l, LV_LABEL_PART_MAIN, get_lvgl_style());
            if(power_4_grid_map[i][k] == 0) {
                lv_obj_set_style_local_text_color(l, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
            } else if(power_4_grid_map[i][k] == 1) {
                lv_obj_set_style_local_text_color(l, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_RED);
            } else if(power_4_grid_map[i][k] == 2) {
                lv_obj_set_style_local_text_color(l, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_YELLOW);
            }
            lv_obj_align(l, playing_page, LV_ALIGN_IN_TOP_LEFT, k*32 + DELTA, 204-i*32);
        }
    }

    for(int i = 0; i < NB_X_POWER_4_CASE; i ++) {
        lv_obj_t *b = lv_btn_create(playing_page, NULL);
        lv_obj_add_style(b, LV_BTN_PART_MAIN, get_lvgl_style());
        lv_obj_set_size(b, 32, 32);

        lv_obj_t *l = lv_label_create(b, NULL);
        lv_label_set_text(l, LV_SYMBOL_DOWN);
        lv_obj_set_style_local_text_color(l, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GREEN);

        lv_obj_align(b, playing_page, LV_ALIGN_IN_TOP_LEFT, i*32 + 5, 5);

        static int playing_player;
        if(playing_player == 0) {
            playing_player = 1;
        }

        //this is shit code but it works

        if(i == 0) {
            lv_obj_set_event_cb(b, [](lv_obj_t *obj, lv_event_t event){
                if(event == LV_EVENT_CLICKED) {
                    for(int k = 0; k < NB_Y_POWER_4_CASE; k ++) {
                        if(power_4_grid_map[k][0] == 0) {
                            power_4_grid_map[k][0] = playing_player;
                            if(playing_player == 1) {
                                playing_player = 2;
                            } else if(playing_player == 2) {
                                playing_player = 1;
                            }
                            break;
                        }
                    }
                   setup_playing_page(false);
                }
            });
        }

        if(i == 1) {
            lv_obj_set_event_cb(b, [](lv_obj_t *obj, lv_event_t event){
                if(event == LV_EVENT_CLICKED) {
                    for(int k = 0; k < NB_Y_POWER_4_CASE; k ++) {
                        if(power_4_grid_map[k][1] == 0) {
                            power_4_grid_map[k][1] = playing_player;
                            if(playing_player == 1) {
                                playing_player = 2;
                            } else if(playing_player == 2) {
                                playing_player = 1;
                            }
                            break;
                        }
                    }
                   setup_playing_page(false);
                }
            });
        }

        if(i == 2) {
            lv_obj_set_event_cb(b, [](lv_obj_t *obj, lv_event_t event){
                if(event == LV_EVENT_CLICKED) {
                    for(int k = 0; k < NB_Y_POWER_4_CASE; k ++) {
                        if(power_4_grid_map[k][2] == 0) {
                            power_4_grid_map[k][2] = playing_player;
                            if(playing_player == 1) {
                                playing_player = 2;
                            } else if(playing_player == 2) {
                                playing_player = 1;
                            }
                            break;
                        }
                    }
                   setup_playing_page(false);
                }
            });
        }

        if(i == 3) {
            lv_obj_set_event_cb(b, [](lv_obj_t *obj, lv_event_t event){
                if(event == LV_EVENT_CLICKED) {
                    for(int k = 0; k < NB_Y_POWER_4_CASE; k ++) {
                        if(power_4_grid_map[k][3] == 0) {
                            power_4_grid_map[k][3] = playing_player;
                            if(playing_player == 1) {
                                playing_player = 2;
                            } else if(playing_player == 2) {
                                playing_player = 1;
                            }
                            break;
                        }
                    }
                   setup_playing_page(false);
                }
            });
        }

        if(i == 4) {
            lv_obj_set_event_cb(b, [](lv_obj_t *obj, lv_event_t event){
                if(event == LV_EVENT_CLICKED) {
                    for(int k = 0; k < NB_Y_POWER_4_CASE; k ++) {
                        if(power_4_grid_map[k][4] == 0) {
                            power_4_grid_map[k][4] = playing_player;
                            if(playing_player == 1) {
                                playing_player = 2;
                            } else if(playing_player == 2) {
                                playing_player = 1;
                            }
                            break;
                        }
                    }
                   setup_playing_page(false);
                }
            });
        }

        if(i == 5) {
            lv_obj_set_event_cb(b, [](lv_obj_t *obj, lv_event_t event){
                if(event == LV_EVENT_CLICKED) {
                    for(int k = 0; k < NB_Y_POWER_4_CASE; k ++) {
                        if(power_4_grid_map[k][5] == 0) {
                            power_4_grid_map[k][5] = playing_player;
                            if(playing_player == 1) {
                                playing_player = 2;
                            } else if(playing_player == 2) {
                                playing_player = 1;
                            }
                            break;
                        }
                    }
                   setup_playing_page(false);
                }
            });
        }

        if(i == 6) {
            lv_obj_set_event_cb(b, [](lv_obj_t *obj, lv_event_t event){
                if(event == LV_EVENT_CLICKED) {
                    for(int k = 0; k < NB_Y_POWER_4_CASE; k ++) {
                        if(power_4_grid_map[k][6] == 0) {
                            power_4_grid_map[k][6] = playing_player;
                            if(playing_player == 1) {
                                playing_player = 2;
                            } else if(playing_player == 2) {
                                playing_player = 1;
                            }
                            break;
                        }
                    }
                   setup_playing_page(false);
                }
            });
        }

        //end of shitty code
    }

    lv_obj_set_hidden(playing_page, hidden);
}

static void playing_sub_app(AppState s) {
    handle_lvgl_for_app(s, playing_page, true, true);
    if(s == HANDLE) {
        m_idle();
    }
    else if(s == SETUP) {
        setup_playing_page();
    }
}

static void setup_main_page(bool hidden = true) {
    if(main_page != nullptr) {
        lv_obj_del(main_page);
    }

    main_page = lv_list_create(lv_scr_act(), NULL);
    lv_obj_add_style(main_page, LV_OBJ_PART_MAIN, get_lvgl_style());
    lv_obj_set_size(main_page, 240, 240);
    lv_obj_align(main_page, lv_scr_act(), LV_ALIGN_CENTER, 0, 0);

    lv_obj_t *some_button = lv_list_add_btn(main_page, NULL, "POWER 4 GAME !!!");
    lv_obj_add_style(some_button, LV_BTN_PART_MAIN, get_lvgl_style());

    some_button = lv_list_add_btn(main_page, NULL, "Start new game");
    lv_obj_add_style(some_button, LV_BTN_PART_MAIN, get_lvgl_style());
    lv_obj_set_event_cb(some_button, [](lv_obj_t *obj, lv_event_t event) {
        if(event == LV_EVENT_CLICKED) {
            for(int i = 0; i < NB_Y_POWER_4_CASE; i ++) {
                for(int k = 0; k < NB_X_POWER_4_CASE; k ++) {
                    power_4_grid_map[i][k] = 0;
                }
            }
            setup_playing_page();
            stack_app(playing_sub_app);
        }
    });

    some_button = lv_list_add_btn(main_page, NULL, "Resume game");
    lv_obj_add_style(some_button, LV_BTN_PART_MAIN, get_lvgl_style());
    lv_obj_set_event_cb(some_button, [](lv_obj_t *obj, lv_event_t event) {
        if(event == LV_EVENT_CLICKED) {
            stack_app(playing_sub_app);
        }
    });

    lv_obj_set_hidden(main_page, hidden);
}


void appPower4(AppState s) {
    handle_lvgl_for_app(s, main_page, true, true);
    if(s == HANDLE) {
        m_idle();
    }
    else if(s == SETUP) {
        setup_main_page();
        playing_sub_app(SETUP);
    }
}