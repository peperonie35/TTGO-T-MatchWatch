#include "config.h"
#include "MWatch.h"

#include <vector>

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <esp_bt_main.h>

TaskHandle_t xBLE = NULL;
BLEServer *pServer;
BLEService * pService;
BLECharacteristic *pCharacteristic;
BLEAdvertising *pAdvertising;

static bool connected;
static std::vector<String> command_stack;
static std::vector<String> result_command_satck;
static std::vector<String> result_data_satck;
static std::vector<void(*)(String,String)> ble_cb_func_stack;
static std::vector<String> ble_cb_name_stack;
static String working_command = "";

static void handle_on_write(String data);

class cbServer : public BLEServerCallbacks
{
  void onConnect(BLEServer *server) {
    connected = true;
    pCharacteristic->setValue("NA");
    pCharacteristic->notify();
    Serial.print("\nConnected to device");
  }
  void onDisconnect(BLEServer *server) {
    connected = false;
    working_command = "";
    Serial.print("\nDisconnected from device");
  }
};

class cbChar : public BLECharacteristicCallbacks
{
  void onWrite(BLECharacteristic *m_pCharacteristic) {
    Serial.printf("\ndevice wrote: %s", m_pCharacteristic->getValue().c_str());
    handle_on_write(m_pCharacteristic->getValue().c_str());
  }
  void onRead(BLECharacteristic *m_pCharacteristic) {
    //nothing to do
  }
};

void add_ble_cb(void(*cb)(String command, String data), String cb_name) {
  ble_cb_func_stack.push_back(cb);
  ble_cb_name_stack.push_back(cb_name);
}

void remove_ble_cb(String cb_name_to_rmv) {
  for(int i = 0; i < ble_cb_name_stack.size(); i ++) {
    if(ble_cb_name_stack[i] == cb_name_to_rmv) {
      ble_cb_func_stack.erase(ble_cb_func_stack.begin() + i);
      ble_cb_name_stack.erase(ble_cb_name_stack.begin() + i);
    } 
  }
}

static void handle_on_write(String data) {
  static String resp_buf;
  if(working_command != "" && data != "NA") {
    if(data == ".packet_start") {
      pCharacteristic->setValue("READY");
      pCharacteristic->notify();
      resp_buf = "";
    } else if(data == ".packet_end") {
      result_command_satck.push_back(working_command);
      result_data_satck.push_back(resp_buf);
      for(void(*ble_cb_func)(String,String) : ble_cb_func_stack) {
        ble_cb_func(working_command, resp_buf);
      }
      working_command = "";
      pCharacteristic->setValue("NA");
      pCharacteristic->notify();
    } else if(data == "ARBORT") {
      working_command = "";
      resp_buf = "";
      pCharacteristic->setValue("NA");
    } else {
      resp_buf += data;
    }
  } else {
    pCharacteristic->setValue("NA");
    resp_buf = "";
  }
}

void xBLETask(void * pvParameters) {
  BLEDevice::init(watch_name);
  pServer = BLEDevice::createServer();
  pService = pServer->createService(serviceUUID);
  pServer->setCallbacks(new cbServer());
  pCharacteristic = pService->createCharacteristic(charUUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_INDICATE);
  pCharacteristic->setCallbacks(new cbChar());
  pCharacteristic->addDescriptor(new BLE2902());
  pCharacteristic->setValue("NA");
  pAdvertising = pServer->getAdvertising();
  pAdvertising->addServiceUUID(serviceUUID);
  pService->start();
  pAdvertising->setScanResponse(true);
  pAdvertising->start();
  while(1) {
    if(connected) {
      String value = pCharacteristic->getValue().c_str();
      if(!command_stack.empty() && value == "NA" && working_command == "") {
        working_command = command_stack.back();
        command_stack.pop_back();
        pCharacteristic->setValue(working_command.c_str());
        pCharacteristic->notify();
      }
    }
    vTaskDelay(50/portTICK_PERIOD_MS);
  }
  vTaskDelete(NULL);
}

void add_ble_command(String command) {
  command_stack.push_back(command);
}

void enable_ble() {
  json_settings["BLE_enable"] = true;
  write_settings();
  start_ble_task();
}

void disable_ble() {
  json_settings["BLE_enable"] = false;
  write_settings();
  stop_ble_task();
}

void stop_ble_task() {
  Serial.print("\nBLE task stoping");
  if(xBLE != NULL) {
    pService->stop();
    pAdvertising->stop();
    BLEDevice::deinit(false);
    vTaskDelete(xBLE);
    xBLE = NULL;
  }
}

void start_ble_task() {
  Serial.print("\nBLE task starting");
  if(!json_settings["BLE_enable"].is<bool>() || !json_settings["BLE_enable"].as<bool>()) {
    return;
  }
  if(xBLE == NULL) {
    xTaskCreate(xBLETask, "BLE", 8192, (void *) 1, tskIDLE_PRIORITY, &xBLE);
  }
}