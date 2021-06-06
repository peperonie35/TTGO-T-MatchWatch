#include "config.h"
#include "MWatch.h"

enum Operator{NONE = -1, ADD , SUB, DIV, MULT};

static const char *calc_keyboard_labels[] = {
    "7", "8", "9", "/",
    "4", "5", "6", "*",
    "1", "2", "3", "+",
    ".", "0", "-", "="
};

static double current_num;
static double prev_num;
static bool in_decimal ;
static bool current_num_need_reset;
static int nb_decimal;
static Operator current_operator;

void disp_num(double num) {
    tft->fillRect(0, 5, 240, 20, TFT_BLACK);
    tft->drawString(String(num), 5, 5, 4);
}

void reset_current_num() {
    current_num = 0;
    nb_decimal = 0;
    in_decimal = 0;
    current_num_need_reset = false;
}

void get_result_into_prev_num() {
    if(current_operator == ADD) {
        prev_num = prev_num + current_num;
    } else if (current_operator == SUB) {
        prev_num = prev_num - current_num;
    } else if (current_operator == DIV) {
        prev_num = prev_num/current_num;
    } else if (current_operator == MULT) {
        prev_num = prev_num*current_num;
    } else if (current_operator == NONE) {
        prev_num = current_num;
    }
}

void update_current_num_with_num_pressed(int num) {
    if(current_num_need_reset) {
        reset_current_num();
        current_operator = NONE;
    }
    if(!in_decimal) {
        current_num = current_num*10+num;
    } else {
        nb_decimal += 1;
        current_num += num/double(pow(10, nb_decimal));
    }
    disp_num(current_num);
}

void update_for_operator_press(Operator op) {
    if(current_operator == NONE) {
        current_operator = op;
        prev_num = current_num;
        reset_current_num();
        disp_num(current_num);
    } else {
        if(!current_num_need_reset) {
            get_result_into_prev_num();
        }
        reset_current_num();
        current_operator = op;
        disp_num(current_num);
    }
}

void appCalc(AppState s) {
    m_idle(); // disable auto sleep for this app
    if(s == HANDLE) {
        if(last_gesture < 17 && last_gesture > -1) {
            String l = calc_keyboard_labels[last_gesture];
            if(l == "1") {
                update_current_num_with_num_pressed(1);
            } else if (l == "2") {
                update_current_num_with_num_pressed(2);
            } else if (l == "3") {
                update_current_num_with_num_pressed(3);
            } else if (l == "4") {
                update_current_num_with_num_pressed(4);
            } else if (l == "5") {
                update_current_num_with_num_pressed(5);
            } else if (l == "6") {
                update_current_num_with_num_pressed(6);
            } else if (l == "7") {
                update_current_num_with_num_pressed(7);
            } else if (l == "8") {
                update_current_num_with_num_pressed(8);
            } else if (l == "9") {
                update_current_num_with_num_pressed(9);
            } else if (l == "0") {
                update_current_num_with_num_pressed(0);
            } else if (l == ".") {
                in_decimal = true;
            } else if (l == "-") {
                update_for_operator_press(SUB);
            } else if (l == "=") {
                get_result_into_prev_num();
                current_num_need_reset = true;
                disp_num(prev_num);
            } else if (l == "+") {
                update_for_operator_press(ADD);
            } else if (l == "*") {
                update_for_operator_press(MULT);
            } else if (l == "/") {
                update_for_operator_press(DIV);
            } 
        }
    }
    else if(s == INIT) {
        menu_press_mode = 16;
        current_num_need_reset = false;
        current_num = 0;
        prev_num = 0;
        nb_decimal = 0;
        in_decimal = false;
        current_operator = NONE;       
        tft->fillScreen(TFT_BLACK);
        draw_keyboard(16, calc_keyboard_labels, 4, true, "");
    }
    else if(s == DELETE) {
        tft->fillScreen(TFT_BLACK);
    }
}