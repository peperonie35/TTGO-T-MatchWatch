#include "config.h"
#include "MWatch.h"

// clock icon colours
#define CLOCK_CASE TFT_DARKGREY
#define CLOCK_FACE TFT_NAVY
#define CLOCK_POINTER TFT_RED
static uint8_t clock_pointer = 0;
// clock pointer positions
static uint8_t cp[9][4] {
    {120, 65, 112, 56},  // [0] 10h30
    {120, 65, 120, 54},  // [1] 12h00
    {120, 65, 128, 56},  // [2] 01h30
    {120, 65, 131, 65},  // [3] 03h00
    {120, 65, 128, 73},  // [4] 04h30
    {120, 65, 120, 76},  // [5] 06h00
    {120, 65, 112, 73},  // [6] 07h30
    {120, 65, 109, 65},  // [7] 09h00
    {120, 65, 112, 56}   // [8] 10h30
};

static bool running = false;
static uint32_t startingTime;
static uint32_t elapsedTime;

const char *stop_watch_meun_labels[] = { 
    "", "", "",
    "", "", "",
    "start", "", "reset",
    "", "", ""
};

void refreshClockIcon() {
    tft->drawLine(cp [clock_pointer]  [0], cp [clock_pointer]  [1], cp [clock_pointer]  [2], cp [clock_pointer]  [3], CLOCK_FACE);    // erase old
    tft->drawLine(cp [clock_pointer+1][0], cp [clock_pointer+1][1], cp [clock_pointer+1][2], cp [clock_pointer+1][3], CLOCK_POINTER); // draw new
}

void drawClockIcon() {
    tft->fillCircle(120, 41, 3, CLOCK_CASE);  // 3 dots
	tft->fillCircle(108, 46, 3, CLOCK_CASE);
	tft->fillCircle(132, 46, 3, CLOCK_CASE);

	tft->fillCircle(120, 65, 22, CLOCK_CASE);
	tft->fillCircle(120, 65, 18, CLOCK_FACE);
	tft->fillRect(118, 42, 4, 3, CLOCK_CASE);
}

void displayElapsedTime(uint16_t color_f, uint16_t color_b) {
    tft->setTextColor(color_f, color_b);
    sprintf(buff, "%d:%02d:%02d.%03d", elapsedTime/3600000, (elapsedTime/60000)%60, (elapsedTime/1000)%60, elapsedTime%1000);
    tft->setTextSize(1);
    tft->drawString(buff, 50, 100, 4);
}

void appStopWatch(AppState s) {
    m_idle(); // disable auto sleep
    if(s == HANDLE) {
        if(last_gesture == 6) {
            if(!running) {
                running = true;
                startingTime = millis() - elapsedTime;
                stop_watch_meun_labels[6] = "pause";
                flash_keyboard_item(12, stop_watch_meun_labels, 4, true, 2, 0);
            } else {
                running = false;
                stop_watch_meun_labels[6] = "start";
                flash_keyboard_item(12, stop_watch_meun_labels, 4, true, 2, 0);
            }
        } else if (last_gesture == 8) {
            elapsedTime = 0;
            startingTime = millis();
            clock_pointer = 0;
            drawClockIcon();
            refreshClockIcon();
        }
        if(running) {
            elapsedTime = millis() - startingTime;
            clock_pointer ++;
		    if (clock_pointer>=8) clock_pointer=0;
		    refreshClockIcon();
        }
        displayElapsedTime(TFT_ORANGE, TFT_BLACK);
    } else if (s== INIT) {
        tft->fillScreen(TFT_BLACK);

        // draw clock face
	    tft->fillCircle(120, 41, 3, CLOCK_CASE);  // 3 dots
	    tft->fillCircle(108, 46, 3, CLOCK_CASE);
	    tft->fillCircle(132, 46, 3, CLOCK_CASE);

	    tft->fillCircle(120, 65, 22, CLOCK_CASE);
	    tft->fillCircle(120, 65, 18, CLOCK_FACE);
	    tft->fillRect(118, 42, 4, 3, CLOCK_CASE);

        refreshClockIcon();

        menu_press_mode = 12;
        if(running) {
            stop_watch_meun_labels[6] = "pause";
        } else {
            stop_watch_meun_labels[6] = "start";
        }
        flash_keyboard_item(12, stop_watch_meun_labels, 4, true, 2, 0);
        flash_keyboard_item(12, stop_watch_meun_labels, 4, true, 2, 2);
    } else if (s == DELETE) {
        tft->fillScreen(TFT_BLACK);
    }
}