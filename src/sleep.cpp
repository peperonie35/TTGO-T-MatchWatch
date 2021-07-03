#include "config.h"
#include "MWatch.h"

#include <WiFi.h>

void deep_sleep_basic_in() {
    ttgo->displaySleep();
    ttgo->powerOff();
    esp_sleep_enable_ext0_wakeup((gpio_num_t)AXP202_INT, LOW);
    esp_deep_sleep_start();
}

void screen_off_sleep_basic_out() {
    m_idle();
    ttgo->openBL();
    ttgo->displayWakeup();
}

void screen_off_sleep_basic_in() {
    Serial.print("\nEntering screen off sleep basic");
    ttgo->closeBL();
    ttgo->displaySleep();
    last_activity = 2147483657;
}

void light_sleep_basic_out() {
    Serial.print("\nExiting light sleep basic");
    m_idle();
    setCpuFrequencyMhz(160);
    ttgo->startLvglTick();
    ttgo->displayWakeup();
    ttgo->rtc->syncToSystem();
    lv_disp_trig_activity(NULL);
    ttgo->openBL();
    current_app(AppState::INIT);
    if(json_settings["BLE_enable"].is<bool>() && json_settings["BLE_enable"].as<bool>()) {
        start_ble_task();
    }
}

void light_sleep_basic_in() {

    Serial.print("\nEntering light sleep basic");
   
    current_app(DELETE);
    current_app = get_app_by_name(defaultAppName).app_ptr;
    defaultAppSwaperCurrentAppXPosition = get_app_x_position_by_name(defaultAppName);
    defaultAppSwaperCurrentAppYPosition = get_app_y_position_by_name(defaultAppName);

    ttgo->closeBL();
    ttgo->stopLvglTick();
    ttgo->displaySleep();
    
    if(json_settings["BLE_enable"].is<bool>() && json_settings["BLE_enable"].as<bool>()) {
        stop_ble_task();
    }

    last_activity = 2147483657;    

    WiFi.mode(WIFI_OFF);
    setCpuFrequencyMhz(20);

    esp_err_t erret;
    erret = gpio_wakeup_enable ((gpio_num_t)AXP202_INT, GPIO_INTR_LOW_LEVEL);
    if (erret != ESP_OK) {
        Serial.println();
        Serial.print(F("gpio_wakeup_enable failed for AXP202_INT"));
    }
    erret = gpio_wakeup_enable ((gpio_num_t)RTC_INT, GPIO_INTR_LOW_LEVEL);
	if(erret != ESP_OK) {
	    Serial.print(F("\ngpio_wakeup_enable failed for RTC_INT"));
	}
    erret = esp_sleep_enable_gpio_wakeup();
    if (erret != ESP_OK) {
        Serial.println();
        Serial.print(F("esp_sleep_enable_gpio_wakeup() failed"));
    }
    esp_light_sleep_start();
}