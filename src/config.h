#define LILYGO_WATCH_2020_V1              // Use T-Watch2020
#define LILYGO_WATCH_LVGL 
#include <LilyGoWatch.h>
#ifndef LILYGO_WATCH_2020_V1
#define STANDARD_BACKPLANE
#else
#undef STANDARD_BACKPLANE
#define EXTERNAL_DAC_BACKPLANE
#endif
#include <LilyGoWatch.h>
