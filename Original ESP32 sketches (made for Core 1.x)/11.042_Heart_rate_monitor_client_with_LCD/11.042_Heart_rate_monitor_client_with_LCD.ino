/*  11.042 - Heart rate monitor as BLE client with LCD

   This example sketch shows how to connect to a heart rate monitor as a BLE client,
   and read the heart rate characteristic values via notifications.

   To use this sketch, you will need a Polar H7 heart rate monitor.

   This sketch builds on sketch 11.040 and adds an LCD that shows the current heart rate.

   You can make modifications to the UUIDs of the service and the characteristic in order
   to connect to a different service.

   This sketch was written by Peter Dalmaris using information from the
   ESP32 datasheet and examples.


   Components
   ----------
    - ESP32 Dev Kit v4
    - Polar H7 monitor
    - I2C LCD


    IDE
    ---
    Arduino IDE with ESP32 Arduino Code
    (https://github.com/espressif/arduino-esp32)


    Libraries
    ---------
    - BLEDevice
    - BLEUtils
    - BLEServer
    - LiquidCrystal_I2C
    

   Connections
   -----------

   Connect the hardware breakout like this:

    ESP32   |   I2C LCD
   ------------------------------------
    5V      |     Vcc
    GND     |     GND
    GPIO22  |     SCL
    GPIO21  |     SDA


    Other information
    -----------------

    1. ESP32 Datasheet: https://www.espressif.com/sites/default/files/documentation/esp32_datasheet_en.pdf
    2. BLE Scanner app on Google Play (you may need to search for this in your store): https://play.google.com/store/apps/details?id=com.macdom.ble.blescanner&hl=en_AU
    3. BLE Scanner app on iTunes (you may need to search for this in your store): https://itunes.apple.com/au/app/ble-scanner-4-0/id1221763603?mt=8
    4. GATT overview: https://www.bluetooth.com/specifications/gatt/generic-attributes-overview
    5. UUID generator: https://www.uuidgenerator.net
    6. GATT Characteristics: https://www.bluetooth.com/specifications/gatt/characteristics
    7. Learn more about std::string --> https://en.wikipedia.org/wiki/C%2B%2B_string_handling
    8. Liquid Crystal Display library: https://github.com/johnrickman/LiquidCrystal_I2C/blob/master/LiquidCrystal_I2C.h
    9. ASCII codes table: http://www.asciitable.com

    Created on April 9 2019 by Peter Dalmaris

*/

#include "BLEDevice.h"
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2); // If this address is not working for your I2C backpack,
// run the address scanner sketch to determine the actual
// address.

// The remote service we wish to connect to.
static BLEUUID serviceUUID("180D");
// The characteristic of the remote service we are interested in.
static BLEUUID    charUUID("2A37");

static boolean doConnect = false;
static boolean connected = false;
static boolean doScan = false;
static BLERemoteCharacteristic* pRemoteCharacteristic;
static BLEAdvertisedDevice* myDevice;

static void notifyCallback(
  BLERemoteCharacteristic* pBLERemoteCharacteristic,
                                    uint8_t* pData,
                                    size_t length,
                                    bool isNotify) {
    Serial.print("Notify callback for characteristic ");
    Serial.print(" HR: ");
    Serial.print(pData[1]);
    Serial.println();
    
    lcd.init();
    lcd.backlight();
    lcd.setCursor(0, 0);
    lcd.print("Peter's HR:");
    lcd.setCursor(12, 0);
    lcd.print("   ");
    lcd.setCursor(12, 0);
    lcd.print(pData[1]);

    lcd.setCursor(0, 1);
    if (pData[1] > 60)   
      lcd.print("He's alive!  ");
    else    
      lcd.print("He's a robot?");
    
    
}

class MyClientCallback : public BLEClientCallbacks {
  void onConnect(BLEClient* pclient) {
  }

  void onDisconnect(BLEClient* pclient) {
    connected = false;
    Serial.println("onDisconnect");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Disconnected");
  }
};

bool connectToServer() {
    Serial.print("Forming a connection to ");
    Serial.println(myDevice->getAddress().toString().c_str());
    
    BLEClient*  pClient  = BLEDevice::createClient();
    Serial.println(" - Created client");

    pClient->setClientCallbacks(new MyClientCallback());

    // Connect to the remove BLE Server.
    pClient->connect(myDevice);  // if you pass BLEAdvertisedDevice instead of address, it will be recognized type of peer device address (public or private)
    Serial.println(" - Connected to server");

    // Obtain a reference to the service we are after in the remote BLE server.
    BLERemoteService* pRemoteService = pClient->getService(serviceUUID);
    if (pRemoteService == nullptr) {
      Serial.print("Failed to find our service UUID: ");
      Serial.println(serviceUUID.toString().c_str());
      pClient->disconnect();
      return false;
    }
    Serial.println(" - Found our service");


    // Obtain a reference to the characteristic in the service of the remote BLE server.
    pRemoteCharacteristic = pRemoteService->getCharacteristic(charUUID);
    if (pRemoteCharacteristic == nullptr) {
      Serial.print("Failed to find our characteristic UUID: ");
      Serial.println(charUUID.toString().c_str());
      pClient->disconnect();
      return false;
    }
    Serial.println(" - Found our characteristic");

    // Read the value of the characteristic.
    if(pRemoteCharacteristic->canRead()) {
      std::string value = pRemoteCharacteristic->readValue();
      Serial.print("The characteristic value was: ");
      Serial.println(value.c_str());
    }

    if(pRemoteCharacteristic->canNotify())
      pRemoteCharacteristic->registerForNotify(notifyCallback);

    connected = true;
}
/**
 * Scan for BLE servers and find the first one that advertises the service we are looking for.
 */
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
 /**
   * Called for each advertising BLE server.
   */
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    Serial.print("BLE Advertised Device found: ");
    Serial.println(advertisedDevice.toString().c_str());

    // We have found a device, let us now see if it contains the service we are looking for.
    if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(serviceUUID)) {

      BLEDevice::getScan()->stop();
      myDevice = new BLEAdvertisedDevice(advertisedDevice);
      doConnect = true;
      doScan = true;

    } // Found our server
  } // onResult
}; // MyAdvertisedDeviceCallbacks


void setup() {
  Serial.begin(115200);
  Serial.println("Starting Arduino BLE Client application...");
  BLEDevice::init("");
  
  scan_for_heart_monitor();
} 


// This is the Arduino main loop function.
void loop() {

  // If the flag "doConnect" is true then we have scanned for and found the desired
  // BLE Server with which we wish to connect.  Now we connect to it.  Once we are 
  // connected we set the connected flag to be true.
  if (doConnect == true) {
    if (connectToServer()) {
      Serial.println("We are now connected to the BLE Server.");
    } else {
      Serial.println("We have failed to connect to the server; there is nothin more we will do.");
    }
    doConnect = false;
  }

  if (connected == false)
     scan_for_heart_monitor();

}

void scan_for_heart_monitor()
{
  // Retrieve a Scanner and set the callback we want to use to be informed when we
  // have detected a new device.  Specify that we want active scanning and start the
  // scan to run for 5 seconds.
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true);
  pBLEScan->start(5, false);
}
