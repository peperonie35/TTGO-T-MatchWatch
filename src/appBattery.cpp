#include "config.h"
#include "MWatch.h"

uint32_t batt_update_time;
int16_t x, y, per;
float vbus_v, vbus_c, batt_v;

void appBattery(AppState s) {
  if (s == AppState::HANDLE) {
    if (batt_update_time + 1000 < millis()) {
      batt_update_time = millis();
      // get the values
      vbus_v = power->getVbusVoltage();
      vbus_c = power->getVbusCurrent();
      batt_v = power->getBattVoltage();
      per    = power->getBattPercentage();
      tft->setCursor(0, 40);
      if (power->isVBUSPlug()) {
        tft->print("USB Power");
      }
      else {
        tft->print("Unplugged");
      }
      tft->setCursor(0, 70);
      if (power->isChargeing()) {
        tft->print("Charge:");
        tft->print(power->getBattChargeCurrent());
      } else {
        // Show current consumption
        tft->print("Discharge: ");
        tft->print(power->getBattDischargeCurrent());
      }
      tft->print(" mA       ");

      // Print the values
      tft->setCursor(0, 100);
      tft->print("bus voltage: "); tft->print(vbus_v); tft->print(" mV    ");
      tft->setCursor(0, 130);
      tft->print("bus current: "); tft->print(vbus_c); tft->print(" mA    ");
      tft->setCursor(0, 160);
      tft->print("V battery: "); tft->print(batt_v); tft->print(" mV    ");
      tft->setCursor(0, 190);
      tft->print("Percent: "); tft->print(per); tft->print(" %  ");
    }
  }
  if (s == AppState::INIT) {
    tft->fillScreen(TFT_BLACK);
    tft->setTextColor(TFT_YELLOW, TFT_BLACK);
    tft->drawCentreString("BATTERY STATS",  half_width, 10, 2);
    tft->setTextColor(TFT_GREEN, TFT_BLACK);
    tft->setTextFont(2);
    power->adc1Enable(AXP202_VBUS_VOL_ADC1 | AXP202_VBUS_CUR_ADC1 | AXP202_BATT_CUR_ADC1 | AXP202_BATT_VOL_ADC1, true);
  }
  if (s == AppState::DELETE) {
    tft->fillScreen(TFT_BLACK);
  }
}
