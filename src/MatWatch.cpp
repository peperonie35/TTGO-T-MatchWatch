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

#define __MAIN__

#include "MWatch.h"

QueueHandle_t g_event_queue_handle = NULL;
EventGroupHandle_t g_event_group = NULL;
EventGroupHandle_t isr_group = NULL;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.print("booting MWatch . . . ");
  if (!SPIFFS.begin(true)) {
    Serial.println(F("An Error has occurred while mounting SPIFFS"));
  }
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

  pinMode(RTC_INT, INPUT_PULLUP);
  attachInterrupt(RTC_INT, [] {
    rtcIrq = 1;
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

  has_handled_gesture = true;
  //xTaskCreate(handleTouchTaskFunction, "touchTask", 10000, NULL, 1, NULL); //create a spearate task to allow touch in background (not working yet)
  
  current_app(AppState::INIT); // init the default app
  
  m_idle(); //reset last_activity to prevent screensaver_timeout at startup

  Serial.println();
  Serial.print("Setup finished");

}

void loop() {
  // put your main code here, to run repeatedly:
  bool rlst;
  uint8_t data;

  //! Fast response wake-up interrupt
  EventBits_t  bits = xEventGroupGetBits(isr_group);
  if (bits & WATCH_FLAG_SLEEP_EXIT) {
    m_idle();
    setCpuFrequencyMhz(160);

    low_energy();

    if (bits & WATCH_FLAG_BMA_IRQ) {
      do {
        rlst =  ttgo->bma->readInterrupt();
      } while (!rlst);
      xEventGroupClearBits(isr_group, WATCH_FLAG_BMA_IRQ);
    }
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
  if (rtcIrq) {
    Serial.println();
    Serial.print(F("Polled for rtc alarm"));
    rtcIrq = 0;
    disable_rtc_alarm();
  }

  if (xQueueReceive(g_event_queue_handle, &data, 5 / portTICK_RATE_MS) == pdPASS) {
    switch (data) {
      case Q_EVENT_AXP_INT:
        power->readIRQ();
        if (power->isPEKShortPressIRQ()) {
          power->clearIRQ();
          last_activity = 0;
          Serial.println();
          Serial.print("button sleep request ");
          low_energy();
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
    low_energy();
  }
}

//handle going in and out of low_energy state
void low_energy (void) {
  if (ttgo->bl->isOn()) {
    Serial.println();
    Serial.print("Entering light sleep mode.");
    xEventGroupSetBits(isr_group, WATCH_FLAG_SLEEP_MODE);
    current_app(AppState::DELETE);
    ttgo->closeBL();
    ttgo->stopLvglTick();
    ttgo->displaySleep();
    if (!WiFi.isConnected()) {
      WiFi.mode(WIFI_OFF);
      // rtc_clk_cpu_freq_set(RTC_CPU_FREQ_2M);
      setCpuFrequencyMhz(20);
      // Serial.println(F("before gpio_wakeup_enable() party."));

      esp_err_t erret;
      erret = gpio_wakeup_enable ((gpio_num_t)AXP202_INT, GPIO_INTR_LOW_LEVEL);
      if (erret != ESP_OK) {
        Serial.println();
        Serial.print(F("gpio_wakeup_enable failed for AXP202_INT"));
      }
      // Serial.println(F("gpio_wakeup_enable(RTC_INT, LOW_LEVEL)"));
      erret = gpio_wakeup_enable ((gpio_num_t)RTC_INT, GPIO_INTR_LOW_LEVEL);
      if (erret != ESP_OK) {
        Serial.println();
        Serial.print(F("gpio_wakeup_enable failed for RTC_INT"));
      }
      erret = esp_sleep_enable_gpio_wakeup();
      if (erret != ESP_OK) {
        Serial.println();
        Serial.print(F("esp_sleep_enable_gpio_wakeup() failed"));
      }
      esp_light_sleep_start();
    } else {
      Serial.println();
      Serial.print("Error entering light sleep: wifi conected");
    }
  } else {
    ttgo->startLvglTick();
    ttgo->displayWakeup();
    ttgo->rtc->syncToSystem();  // set OS clock to RTC clock
    if (rtcIrq) {
      Serial.println();
      Serial.print(F("wake from sleep, we see rtc alarm"));
      rtcIrq = 0;
    }
    lv_disp_trig_activity(NULL);
    ttgo->openBL();
    current_app(AppState::INIT);
  }
}
