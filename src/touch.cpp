#include "config.h"
#include "MWatch.h"

#include <vector>
#include <functional>

static bool all_enable = true;

void disable_touch_features_for_app(AppState s) {
  if(s == INIT) {
    disable_touch_features();
  } else if (s == DELETE) {
    enable_touch_features();
  }
}

void disable_touch_features() {
  all_enable = false;
}

void enable_touch_features() {
  all_enable = true;
}

//called by void setup
void init_touch() {
}

//allow to change the current app, but swiping right will go back to the previous app
void stack_app(void (*app_to_stack)(AppState)) {
  app_stack.push_back(*current_app);
  changeCurrentApp(app_to_stack);
}

void unstack_app() {
  changeCurrentApp(app_stack[0]);
  app_stack.erase(app_stack.begin());
}

void app_stack_gesture_handler(int mSelect) {
  if(app_stack.empty()) {
    return;
  }
  if(mSelect == RIGHT) {
    changeCurrentApp(app_stack[0]);
    app_stack.erase(app_stack.begin());
  }
}

//handle default app change with gestures
void defaultAppSwaperGestureHandler(int mSelect) {
  if(!app_stack.empty() && mSelect == RIGHT) {
    app_stack_gesture_handler(RIGHT);
    return;
  }
  else if(mSelect == DOWN || mSelect == RIGHT || mSelect == LEFT || mSelect == UP) {
    while(!app_stack.empty()) {
      app_stack.erase(app_stack.begin());
    }
  }
  if (mSelect == DOWN) {
    if(defaultAppSwaperCurrentAppXPosition != 0) {
      defaultAppSwaperCurrentAppXPosition = defaultAppSwaperCurrentAppXPosition - 1;
    } else {
      return;
    }
  } else if (mSelect == UP) {
    if(defaultAppSwaperCurrentAppXPosition != defaultAppSwaperAppPositionsXmax) {
      defaultAppSwaperCurrentAppXPosition += 1;
    } else {
      return;
    }
  }
  else if (mSelect == RIGHT) {
    if(defaultAppSwaperCurrentAppYPosition != 0) {
      defaultAppSwaperCurrentAppYPosition = defaultAppSwaperCurrentAppYPosition - 1;
    } else {
      return;
    }
  } else if (mSelect == LEFT) {
    if(defaultAppSwaperCurrentAppYPosition != defaultAppSwaperAppPositionsYmax) {
      defaultAppSwaperCurrentAppYPosition += 1;
    } else {
      return;
    }
  } else {
    return;
  }
  changeCurrentApp(defaultAppSwaperAppPositions[defaultAppSwaperCurrentAppXPosition][defaultAppSwaperCurrentAppYPosition]);
}

//return the gesture or a default button press
int poll_swipe_or_menu_press(int num_choices) {
  if(!all_enable) {
    return -1;
  }
  uint32_t lasttouch, interval;
  int16_t nx, ny, x, y, x0, y0, amax, points;
  x = 0;
  y = 0;
  enum SWIPE_DIR resdir;
  resdir = NODIR;
  if (num_choices < 14) {
    num_choices = 12;
  }
  else {
    num_choices = 16;
  }
  int xdir = 0;
  int ydir = 0;
  int xdif = 0;
  int ydif = 0;
  x0 = -1;
  y0 = -1;
  char dir;
  int16_t max_lrud[4] = {1000, -1, 1000, -1};
  int16_t pts_lrud[4] = { -1, -1, -1, -1};
  points = 0;
  lasttouch = millis();
  do {
    interval = millis() - lasttouch;
    // Serial.printf("(1) interval = %lu\n", interval);
    if (ttgo->getTouch(nx, ny)) {
      // Serial.print(F("T"));
      if ((nx > 0) && (ny > 0) && (nx < 240) && (ny < 240)) {
        // Serial.println(F("G"));
        x = nx; y = ny;
        // remember the first touch:
        if (x0 < 0) {
          x0 = x;
        }
        if (y0 < 0) {
          y0 = y;
        }
        int i = points;
        if (pts_lrud[0] < 0 || x < max_lrud[0]) {
          max_lrud[0] = x;
          pts_lrud[0] = i++;
        }
        if (pts_lrud[1] < 0 || x > max_lrud[1]) {
          max_lrud[1] = x;
          pts_lrud[1] = i++;
        }
        if (pts_lrud[2] < 0 || y < max_lrud[2]) {
          max_lrud[2] = y;
          pts_lrud[2] = i++;
        }
        if (pts_lrud[3] < 0 || y > max_lrud[3]) {
          max_lrud[3] = y;
          pts_lrud[3] = i++;
        }
        points++;
      }
      lasttouch = millis();
    }
  } while (interval < 100);
  if (x0 >= 0 && y0 >= 0) {
    //udpate last activity cause touch
    m_idle();
    // Serial.printf("(2) interval = %lu\n", interval);
    xdir = x - x0;  // x extent of swipe.  near zero if a circle gesture
    ydir = y - y0;  // y extent of swipe.  near zero if a circle gesture
    xdif = max_lrud[1] - max_lrud[0]; // always a positive difference
    ydif = max_lrud[3] - max_lrud[2]; // always a positive difference
    amax = (xdif > ydif) ? xdif : ydif ;
    if (points > 100 && xdif > 60 && ydif > 60) { // is gesture a circle ?
      int16_t min_value[4] = {1000, 1000, 1000, 1000};
      char lut[4] = { 'L', 'R', 'U', 'D' };
      char dir_order[9];
      dir_order[8] = '\0';
      int8_t min_index = 0;
      for (int8_t j = 0 ; j < 4 ; j++) {
        for (int8_t i = 0 ; i < 4 ; i++) {
          if (pts_lrud[i] < min_value[j] && (j == 0 || pts_lrud[i] > min_value[j - 1])) {
            min_value[j] = pts_lrud[i];
            min_index = i;
          }
        }
        dir_order[j] = lut[min_index];
        dir_order[j + 4] = lut[min_index];
      }
      if (strstr(dir_order, "URDL")) {
        return (CWCIRCLE);
      }
      if (strstr(dir_order, "ULDR")) {
        return (CCWCIRCLE);
      }
    }
    else if (amax > 60) { // moved across 1/4 of the screen, so probably a swipe
      dir = (ydif > xdif) ? 'y' : 'x' ;
      resdir =
        (dir == 'x') ? ((xdir > 0) ? RIGHT : LEFT ) :
        (ydir > 0) ? DOWN : UP;
      return resdir;
    }
    else {  // must be button press
      if (num_choices == 12) {
        if (y < 85) {
          if (x < 80) return 0;
          else if (x > 160) return 2;
          else return 1;
        }
        else if (y < 135) {
          if (x < 80) return 3;
          else if (x > 160) return 5;
          else return 4;
        }
        else if (y < 185) {
          if (x < 80) return 6;
          else if (x > 160) return 8;
          else return 7;
        }
        else if (x < 80) return 9;
        else if (x > 160) return 11;
        else return 10;
      }
      else {  // must be 16 choices
        if (y < 85) {
          if (x < 60) return 0;
          else if (x < 120) return 1;
          else if (x < 180) return 2;
          else return 3;
        }
        else if (y < 135) {
          if (x < 60) return 4;
          else if (x < 120) return 5;
          else if (x < 180) return 6;
          else return 7;
        }
        else if (y < 185) {
          if (x < 60) return 8;
          else if (x < 120) return 9;
          else if (x < 180) return 10;
          else return 11;
        }
        if (x < 60) return 12;
        else if (x < 120) return 13;
        else if (x < 180) return 14;
        else return 15;
      }
    }
  }
  return -1;
}