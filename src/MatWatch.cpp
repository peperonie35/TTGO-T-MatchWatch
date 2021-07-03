#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "freertos/queue.h"
#include <soc/rtc.h>
#include "esp_sleep.h"
// ---------------
#include "config.h"
#include <SPIFFS.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <AceTime.h>
#include <Arduino.h>
#include "SPIFFS.h"
#include "FS.h"
#include <EEPROM.h>

#define __MAIN__

#define EPPROM_SIZE JSON_SETTINGS_SIZE

#include "MWatch.h"

QueueHandle_t g_event_queue_handle = NULL;
EventGroupHandle_t g_event_group = NULL;
EventGroupHandle_t isr_group = NULL;

void setup() {
  // put your setup code here, to run once:

  Serial.begin(115200);

  Serial.print("\nbooting MWatch . . . ");

  //EEPROM.begin(EPPROM_SIZE);
  if (!SPIFFS.begin(true)) {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }
  
  read_settings();

  pinMode(4, OUTPUT);
  quickBuzz();

  //Create a program that allows the required message objects and group flags
  g_event_queue_handle = xQueueCreate(20, sizeof(uint8_t));
  g_event_group = xEventGroupCreate();
  isr_group = xEventGroupCreate();

  ttgo = TTGOClass::getWatch();
  ttgo->begin();
  tft = ttgo->tft;
  power = ttgo->power;
  tft->setTextFont(1);
  tft->fillScreen(TFT_BLACK);
  tft->setTextColor(TFT_YELLOW, TFT_BLACK);
  half_width = tft->width() / 2;
  half_height = tft->height() / 2;


  // Turn on the IRQ used
  power->adc1Enable(AXP202_BATT_VOL_ADC1 | AXP202_BATT_CUR_ADC1 | AXP202_VBUS_VOL_ADC1 | AXP202_VBUS_CUR_ADC1, AXP202_ON);
  power->enableIRQ(AXP202_VBUS_REMOVED_IRQ | AXP202_VBUS_CONNECT_IRQ | AXP202_CHARGING_FINISHED_IRQ, AXP202_ON);
  power->clearIRQ();

  // Turn off unused power
  power->setPowerOutPut(AXP202_EXTEN, AXP202_OFF);
  power->setPowerOutPut(AXP202_DCDC2, AXP202_OFF);
  power->setPowerOutPut(AXP202_LDO3, AXP202_OFF);  // audio device
  power->setPowerOutPut(AXP202_LDO4, AXP202_OFF);

  //Initialize lvgl
  ttgo->lvgl_begin();

  // Connection interrupted to the specified pin
  pinMode(AXP202_INT, INPUT);
  attachInterrupt(AXP202_INT, [] {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    EventBits_t  bits = xEventGroupGetBitsFromISR(isr_group);
    if (bits & WATCH_FLAG_SLEEP_MODE)
    {
      //! For quick wake up, use the group flag
      xEventGroupSetBitsFromISR(isr_group, WATCH_FLAG_SLEEP_EXIT | WATCH_FLAG_AXP_IRQ, &xHigherPriorityTaskWoken);
    } else
    {
      uint8_t data = Q_EVENT_AXP_INT;
      xQueueSendFromISR(g_event_queue_handle, &data, &xHigherPriorityTaskWoken);
    }
    if (xHigherPriorityTaskWoken)
    {
      portYIELD_FROM_ISR ();
    }
  }, FALLING);

  ttgo->rtc->syncToSystem();

  ttgo->openBL(); // Turn on the backlight

  wifi_setup(); //setup wifi for use in app

  init_touch();
  
  m_idle(); //reset last_activity to prevent screensaver_timeout at startup

  //calling the setup for all apps
  for(App app : Applications) {
    app.app_ptr(SETUP);
  }

  current_app(AppState::INIT); // init the default app

  start_ble_task();

  Serial.println();
  Serial.print("Setup finished");

  watch_on = true;
}

void loop() {
  // put your main code here, to run repeatedly:
  uint8_t data;


  //! Fast response wake-up interrupt
  EventBits_t  bits = xEventGroupGetBits(isr_group);
  if (bits & WATCH_FLAG_SLEEP_EXIT) {
    low_energy(WAKE_UP);
    if (bits & WATCH_FLAG_AXP_IRQ) {
      power->readIRQ();
      power->clearIRQ();
      //TODO: Only accept axp power pek key short press
      xEventGroupClearBits(isr_group, WATCH_FLAG_AXP_IRQ);
    }
    xEventGroupClearBits(isr_group, WATCH_FLAG_SLEEP_EXIT);
    xEventGroupClearBits(isr_group, WATCH_FLAG_SLEEP_MODE);
  }
  if ((bits & WATCH_FLAG_SLEEP_MODE)) {
    //! No event processing after entering the information screen
    return;
  }

  //! Normal polling

  if (xQueueReceive(g_event_queue_handle, &data, 5 / portTICK_RATE_MS) == pdPASS) {
    switch (data) {
      case Q_EVENT_AXP_INT:
        power->readIRQ();
        if (power->isPEKShortPressIRQ()) {
          power->clearIRQ();
          last_activity = 0;
          Serial.println();
          Serial.print("button sleep request ");
          low_energy(SLEEP);
          return;
        }
        power->clearIRQ();
        break;
      default:
        break;
    }
  }

  //handle on screen display if not in sleep
  //if(lv_disp_get_inactive_time(NULL) < screensaver_timeout * 1000) //does not reset at touch
  if( last_activity + screensaver_timeout * 1000 > millis()){
    last_gesture = poll_swipe_or_menu_press(menu_press_mode);
    current_gesture_handler(last_gesture);
    current_app(AppState::HANDLE);
   } else {
    Serial.println();
    Serial.print("screensaver_timeout bye !");
    low_energy(SLEEP);
  }

}





//handle going in and out of low_energy state
void low_energy (LEState) {
  if (watch_on) {
    xEventGroupSetBits(isr_group, WATCH_FLAG_SLEEP_MODE);
    if(json_settings["sleep_mode"].is<String>()) {
      String slm = json_settings["sleep_mode"].as<String>();
      if(slm == "light_sleep_basic") {
        light_sleep_basic_in();
      } else if(slm == "deep_sleep_basic") {
        deep_sleep_basic_in();
      } else if(slm == "screen_off_sleep_basic") {
        screen_off_sleep_basic_in();
      }
    } else {
      json_settings["sleep_mode"] = "light_sleep_basic";
      write_settings();
      light_sleep_basic_in();
    }
    watch_on = false;
  } else {
    if(json_settings["sleep_mode"].is<String>()) {
      String slm = json_settings["sleep_mode"].as<String>();
      if(slm == "light_sleep_basic") {
        light_sleep_basic_out();
      } else if(slm == "screen_off_sleep_basic") {
        screen_off_sleep_basic_out();
      }
    } else {
      json_settings["sleep_mode"] = "light_sleep_basic";
      write_settings();
      light_sleep_basic_out();
    }
    watch_on = true;
  }
}