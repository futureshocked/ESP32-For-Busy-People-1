/*  11.030 - Simple BLE server with LCD

   This example sketch shows how to create a BLE service with two read/write characteristics.

   After you upload the sketch to your ESP32, use a BLE app, such as BLE Scanner, to interact with 
   the BLE service running on the ESP32.

   The LCD will display the current values of the two characteristics.

   The value for characteristic 1 is displayed in row 0.
   The value for characteristic 0 is displayed in row 1.

   This sketch was written by Peter Dalmaris using information from the
   ESP32 datasheet and examples.


   Components
   ----------
    - ESP32 Dev Kit v4


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


#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <LiquidCrystal_I2C.h>

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID         "ce3fa4b9-9d6d-4787-963f-2afab5cc061e"  // This is a custom service
#define CHARACTERISTIC_UUID1 "32e97b51-92d1-4911-94f9-833213e35892"  // This is a custom characteristic
#define CHARACTERISTIC_UUID2 "9035bf14-6416-4b6e-9a74-fcd74682e4e1"  // This is a custom characteristic

LiquidCrystal_I2C lcd(0x27, 16, 2); // If this address is not working for your I2C backpack,
// run the address scanner sketch to determine the actual
// address.

BLECharacteristic *pCharacteristic1;
BLECharacteristic *pCharacteristic2;


class MyCallbacks1: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string value = pCharacteristic->getValue();
      lcd.init();
      lcd.backlight(); 
      lcd.setCursor(0, 0);
      lcd.print("C1:             "); // Clear only row 0
      lcd.setCursor(3, 0);          // Reset the cursor to start writing
      if (value.length() > 0) {
        Serial.println("*********");
        Serial.print("New value Char 1: ");
        for (int i = 0; i < value.length(); i++)
        {
          Serial.print(value[i]);
          lcd.print(value[i]);
        }
        Serial.println();
        Serial.println("*********");
      }
    }
};

class MyCallbacks2: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string value = pCharacteristic->getValue();      
      lcd.init();
      lcd.backlight(); 
      lcd.setCursor(0, 1);
      lcd.print("C2:             "); // Clear only row 1
      lcd.setCursor(3, 1);          // Reset the cursor to start writing
      if (value.length() > 0) {
        Serial.println("*********");
        Serial.print("New value Char 2: ");
        for (int i = 0; i < value.length(); i++)
        {
          Serial.print(value[i]);
            lcd.print(value[i]);
        }

        Serial.println();
        Serial.println("*********");
      }
    }
};

void setup() {
  Serial.begin(115200);

  BLEDevice::init("Peter's ESP32");
  BLEServer *pServer = BLEDevice::createServer();

  BLEService *pService = pServer->createService(SERVICE_UUID);

  BLECharacteristic *pCharacteristic1 = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID1,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_WRITE
                                       );
  BLECharacteristic *pCharacteristic2 = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID2,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_WRITE
                                       );

  pCharacteristic1->setCallbacks(new MyCallbacks1());

  pCharacteristic1->setValue("Hi World c1");

  pCharacteristic2->setCallbacks(new MyCallbacks2());

  pCharacteristic2->setValue("Hi World c2");
  
  pService->start();

  BLEAdvertising *pAdvertising = pServer->getAdvertising();
  pAdvertising->start();

  lcd.init();
  lcd.backlight();  
  lcd.setCursor(0, 0);
  lcd.print("C1:");
  std::string value1 = pCharacteristic1->getValue();
  for (int i = 0; i < value1.length(); i++)
          lcd.print(value1[i]);
  lcd.setCursor(0, 1);
  lcd.print("C2:");
  std::string value2 = pCharacteristic2->getValue();
  for (int i = 0; i < value2.length(); i++)
          lcd.print(value2[i]);
 
}

void loop() {
  // put your main code here, to run repeatedly:
  delay(2000);
}
