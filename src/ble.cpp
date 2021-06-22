#include "config.h"
#include "MWatch.h"

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <esp_bt_main.h>

//sry, this is a mess but I had to adapt https://github.com/Bellafaire/ESP32-Smart-Watch ble system

void xFindDevice(void * pvParameters );

static BLEUUID serviceUUID("d3bde760-c538-11ea-8b6e-0800200c9a66");
static BLEUUID    charUUID("d3bde760-c538-11ea-8b6e-0800200c9a67");

static BLERemoteCharacteristic* pRemoteCharacteristic;
static BLEAdvertisedDevice* myDevice;
static BLEClient*  pClient;
static boolean connected = false;
static String command_to_send = "";
TaskHandle_t xConnect = NULL;
static bool restart = false;

static String receivedData = "";

class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {
      Serial.print("\nBLE Advertised Device found: " + String(advertisedDevice.toString().c_str()));
      if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(serviceUUID)) {
        BLEDevice::getScan()->stop();
        myDevice = new BLEAdvertisedDevice(advertisedDevice);
        Serial.print("\nDevice Found");
        restart = false;
      }
    }
};

class MyClientCallback : public BLEClientCallbacks {
    void onConnect(BLEClient* pclient) {
      connected = true;
    }

    void onDisconnect(BLEClient* pclient) {
      connected = false;
      pClient->disconnect();
      Serial.print("\nDevice has Disconnected %%%%%%%%");
    }
};


/* Opens connection to the server and allows us to access the characteristic that data
    is transmitted to, this should be called after xFindDevice completes, however it
    cannot be run in a seperate thread due to I2C
*/
void formConnection(void * pvParameters) {
  //if for some this function is called before we find the device then we need to
  //do the scan again and make sure that we have a device

  BLEDevice::init(watch_name);

  if (!myDevice) {
    xFindDevice((void*) 1);
  }

  //create a client to communicate to the server through
  pClient = BLEDevice::createClient();

  //set callbacks for client, if it disconnects we need to know,
  //also we don't consider the device to be connected unless the client is connected
  pClient->setClientCallbacks(new MyClientCallback());

  //check that device is found again
  //attempting to connect to a null device will cause the device to crash
  if (myDevice) {
    pClient->connect(myDevice);
  } else {
    vTaskDelete(NULL);
  }

  //obtain a reference to the desired service
  //and check for null reference, this indicates failure
  BLERemoteService* pRemoteService = pClient->getService(serviceUUID);
  if (!pRemoteService) {
    pClient->disconnect();
    Serial.print("\nCould not obtain remote service");
    vTaskDelete(NULL);
  }

  //second verse same as the first, but for the characteristic
  pRemoteCharacteristic = pRemoteService->getCharacteristic(charUUID);
  if (!pRemoteCharacteristic) {
    pClient->disconnect();
    Serial.print("\nCould not obtain remote characteristic");
    vTaskDelete(NULL);
  }

  vTaskDelete(NULL);
}

void xFindDevice(void * pvParameters ) {


    BLEDevice::init(watch_name);

    Serial.print("\nFind Device Task Launched %%%");

    BLEScan* pBLEScan = BLEDevice::getScan();
    pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
    pBLEScan->setInterval(40);           //I've found these interval and window values to work the best with android, but others may be better.
    pBLEScan->setWindow(39);
    pBLEScan->setActiveScan(true);
    pBLEScan->start(8);
    delete pBLEScan;

    if (myDevice) {
      Serial.print("\n%%% Device Found %%%");
      xTaskCreatePinnedToCore( formConnection, "FIND_DEVICE", 4096, (void *) 1 , tskIDLE_PRIORITY, &xConnect, 0 );
     } else {
     Serial.print("\n%%%% Device Not Found %%%");
    }

    vTaskDelete(NULL);
}

String get_ble_received_data() {
  //Serial.print(restart);
    if(restart || !pRemoteCharacteristic || !pRemoteCharacteristic->canRead()) {
        return "null";
    }
    receivedData = String(pRemoteCharacteristic->readValue().c_str());
    if(receivedData == "" || receivedData == "********") {
        receivedData = "null";
    }
    return receivedData;
}

void handle_ble() {
    if(command_to_send != "") {
        if(sendBLE(command_to_send)) {
            command_to_send = "";
        }
    }
    String data = get_ble_received_data();
    if(data != "null") {
        Serial.printf("\nrecovered value: %s", data.c_str());
    }
}

void add_ble_command_to_send(String command) {
    command_to_send = command;
}

bool sendBLE(String command) {
  //write our command to the remote characteristic
  if (!restart && pRemoteCharacteristic && pRemoteCharacteristic->canWrite()) {
    pRemoteCharacteristic->writeValue(command.c_str(), command.length());
    Serial.print("\nWrote \"" + command + "\" to remote device");
  } else {
    return false;
  }
  return true;
}

void xReconnectDevice(void * pvParameters ) {
  pClient->disconnect();
  if(pClient->connect(myDevice))
    restart = false;
  vTaskDelete(NULL);
}

void initBLE() {
  BLEDevice::init(watch_name);
  if (!connected) {
    xTaskCreatePinnedToCore( xFindDevice, "FIND_DEVICE", 4096, (void *) 1 , tskIDLE_PRIORITY, &xConnect, 0 );
  }  else if(restart) {
    xTaskCreatePinnedToCore( xReconnectDevice, "RECONNECT_DEVICE", 4096, (void *) 1 , tskIDLE_PRIORITY, &xConnect, 0 );
  }
}

void deleteBLE() {
  restart = true;
}

void handle_ble_for_app(AppState s, void(*ble_data_handler)(String data)) {
  if(s == HANDLE) {
    if(command_to_send != "" && sendBLE(command_to_send)) {
      command_to_send = "";
    }
    String d = get_ble_received_data();
    if(d != "null") {
      ble_data_handler(d);
    }
  } else if(s == DELETE) {
    restart = true;
  } else if(s == INIT) {
    initBLE();
  }
}