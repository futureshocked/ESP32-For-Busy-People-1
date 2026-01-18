/*  11.031 - Simple BLE Server with LCD for ESP32-S3
 * 
 * Course: IoT Development with ESP32-S3
 * Target Hardware: ESP32-S3-DevKitC-1 v1.1
 * Module: ESP32-S3-WROOM-1-N16R16V
 * 
 * This sketch demonstrates how to implement a Bluetooth Low Energy (BLE) server on the ESP32-S3
 * using the NimBLE library. It includes two read/write characteristics. A 16x2 I2C LCD displays
 * the current values of both characteristics. The user can interact with the server via BLE apps
 * such as BLE Scanner to read and write values, which are shown on the LCD.
 * 
 * HARDWARE INFORMATION:
 * =====================
 * - ESP32-S3-DevKitC-1 v1.1
 * - 16MB Flash, 16MB PSRAM
 * - GPIO1-GPIO21 safe for I/O
 * - I2C LCD 1602, address 0x27
 * 
 * CIRCUIT DIAGRAM:
 * ================
 * 
 *    ESP32-S3           I2C LCD 16x2
 *   ----------        ----------------
 *    5V               -> VCC
 *    GND              -> GND
 *    GPIO10 (SDA)     -> SDA
 *    GPIO8  (SCL)     -> SCL
 * 
 * COMPONENTS:
 * ===========
 * - ESP32-S3-DevKitC-1 v1.1
 * - I2C LCD 1602 display
 * - USB-C cable for programming and power
 * 
 * GPIO ASSIGNMENTS:
 * =================
 * - SDA: GPIO10 (ADC1_CH9)
 * - SCL: GPIO8 (ADC1_CH7)
 * 
 * TECHNICAL NOTES:
 * ================
 * - NimBLE is used instead of the deprecated BLE library for performance and memory efficiency.
 * - Use a BLE scanner to write data to each characteristic.
 * - LCD updated only when data is received.
 * 
 * IDE REQUIREMENTS:
 * =================
 * - Arduino IDE 2.0+
 * - ESP32 Arduino Core v3.0+
 * - Board: "ESP32S3 Dev Module"
 * - Library: NimBLE-Arduino, LiquidCrystal_I2C
 * 
 * Created: April 9, 2019 by Peter Dalmaris
 * Updated: July 30, 2025 for ESP32-S3 compatibility
 */

#include <NimBLEDevice.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Constants
constexpr char DEVICE_NAME[] = "ESP32S3 BLE LCD";
constexpr char SERVICE_UUID[] = "ce3fa4b9-9d6d-4787-963f-2afab5cc061e";
constexpr char CHARACTERISTIC_UUID1[] = "32e97b51-92d1-4911-94f9-833213e35892";
constexpr char CHARACTERISTIC_UUID2[] = "9035bf14-6416-4b6e-9a74-fcd74682e4e1";

constexpr uint8_t LCD_I2C_ADDRESS = 0x27;
constexpr uint8_t LCD_COLUMNS = 16;
constexpr uint8_t LCD_ROWS = 2;
constexpr gpio_num_t SDA_PIN = GPIO_NUM_10;
constexpr gpio_num_t SCL_PIN = GPIO_NUM_8;

// Globals
LiquidCrystal_I2C lcd(LCD_I2C_ADDRESS, LCD_COLUMNS, LCD_ROWS);
NimBLECharacteristic* characteristic1;
NimBLECharacteristic* characteristic2;

// Callback for characteristic 1
class Char1Callbacks : public NimBLECharacteristicCallbacks {
  void onWrite(NimBLECharacteristic* characteristic) {
    std::string value = characteristic->getValue();
    Serial.print("Char1 received: "); Serial.println(value.c_str());

    lcd.setCursor(0, 0);
    lcd.print("C1:             ");
    lcd.setCursor(3, 0);
    lcd.print(value.c_str());
  }
};

// Callback for characteristic 2
class Char2Callbacks : public NimBLECharacteristicCallbacks {
  void onWrite(NimBLECharacteristic* characteristic) {
    std::string value = characteristic->getValue();
    Serial.print("Char2 received: "); Serial.println(value.c_str());

    lcd.setCursor(0, 1);
    lcd.print("C2:             ");
    lcd.setCursor(3, 1);
    lcd.print(value.c_str());
  }
};

void setup() {
  Serial.begin(115200);
  Wire.begin(SDA_PIN, SCL_PIN);
  lcd.init();
  lcd.backlight();

  NimBLEDevice::init(DEVICE_NAME);
  NimBLEServer* server = NimBLEDevice::createServer();
  NimBLEService* service = server->createService(SERVICE_UUID);

  characteristic1 = service->createCharacteristic(
    CHARACTERISTIC_UUID1,
    NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE
  );
  characteristic1->setCallbacks(new Char1Callbacks());
  characteristic1->setValue("Hi C1");

  characteristic2 = service->createCharacteristic(
    CHARACTERISTIC_UUID2,
    NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE
  );
  characteristic2->setCallbacks(new Char2Callbacks());
  characteristic2->setValue("Hi C2");

  service->start();
  NimBLEAdvertising* advertising = server->getAdvertising();
  advertising->start();

  // Initial LCD display
  lcd.setCursor(0, 0); lcd.print("C1: Hi C1");
  lcd.setCursor(0, 1); lcd.print("C2: Hi C2");

  Serial.println("BLE Server ready");
}

void loop() {
  delay(1000); // Nothing to do here
}
