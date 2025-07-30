/*  11.013 - BLE UART (Minimal) with I2C LCD and BME280 for ESP32-S3
 * 
 * Course: IoT Development with ESP32-S3
 * Target Hardware: ESP32-S3-DevKitC-1 v1.1
 * Module: ESP32-S3-WROOM-1-N16R16V
 * 
 * This is a simplified version of the BLE UART example that:
 * - Displays BLE-received characters on a 16x2 LCD screen
 * - Sends temperature and humidity from a BME280 sensor every ~10 seconds
 * - Clears the LCD when '~' is received
 * 
 * This minimal version focuses on core functionality and reduces code complexity.
 * 
 * HARDWARE MODERNIZATION:
 * ========================
 * ESP32-S3 does not support Classic Bluetooth. We use NimBLE to emulate a BLE UART terminal.
 * 
 * CIRCUIT DIAGRAM:
 * ================
 * ESP32-S3         | BME280    | I2C LCD
 * ----------------|-----------|----------
 * 3V3             | VIN       | VCC
 * GND             | GND       | GND
 * GPIO21 (SDA)    | SDA       | SDA
 * GPIO22 (SCL)    | SCL       | SCL
 * 
 * IDE REQUIREMENTS:
 * =================
 * - Arduino IDE 2.0+
 * - ESP32 Arduino Core v3.0+
 * - Board: "ESP32S3 Dev Module"
 * - Libraries:
 *   - NimBLE-Arduino
 *   - LiquidCrystal_I2C
 *   - Adafruit_BME280 + Adafruit Unified Sensor
 */

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Adafruit_BME280.h>
#include <NimBLEDevice.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);
Adafruit_BME280 bme;
NimBLECharacteristic* txChar;

int row = 0, col = 0;
int lazyCounter = 0;
bool connected = false;

class RxCB : public NimBLECharacteristicCallbacks {
  void onWrite(NimBLECharacteristic* c) {  // ✅ Removed 'override'
    std::string value = c->getValue();
    for (char ch : value) {
      if (ch == '~') {
        row = col = 0;
        lcd.clear();
      } else {
        lcd.setCursor(col, row);
        lcd.print(ch);
        col++;
        if (col > 15) { col = 0; row++; }
        if (row > 1)  { row = 0; lcd.clear(); }
      }
    }
  }
};

class ConnCB : public NimBLEServerCallbacks {
  void onConnect(NimBLEServer*) {  // ✅ Removed 'override'
    connected = true;
  }

  void onDisconnect(NimBLEServer*) {  // ✅ Removed 'override'
    connected = false;
    NimBLEDevice::startAdvertising();
  }
};

void setup() {
  Wire.begin(21, 22);
  Serial.begin(115200);
  lcd.init(); lcd.backlight();
  if (!bme.begin(0x76)) while (1);

  NimBLEDevice::init("BLE-Display");
  auto server = NimBLEDevice::createServer();
  server->setCallbacks(new ConnCB());

  auto service = server->createService("6E400001-B5A3-F393-E0A9-E50E24DCCA9E");
  auto rx = service->createCharacteristic("6E400002-B5A3-F393-E0A9-E50E24DCCA9E",
      NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::WRITE_NR);
  rx->setCallbacks(new RxCB());

  txChar = service->createCharacteristic("6E400003-B5A3-F393-E0A9-E50E24DCCA9E",
      NIMBLE_PROPERTY::NOTIFY);

  service->start();
  NimBLEDevice::getAdvertising()->addServiceUUID(service->getUUID());
  NimBLEDevice::getAdvertising()->start();
}

void loop() {
  if (++lazyCounter >= 200) {
    if (connected) {
      char msg[64];
      snprintf(msg, sizeof(msg), "T: %.1fC H: %.1f%%", bme.readTemperature(), bme.readHumidity());
      txChar->setValue(msg);
      txChar->notify();
    }
    lazyCounter = 0;
  }
  delay(50);
}
