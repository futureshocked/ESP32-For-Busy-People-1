/*  11.013 - BLE UART with I2C LCD and BME280 Sensor for ESP32-S3
 * 
 * Course: IoT Development with ESP32-S3
 * Target Hardware: ESP32-S3-DevKitC-1 v1.1
 * Module: ESP32-S3-WROOM-1-N16R16V
 * 
 * This sketch demonstrates how to:
 *  - Receive characters from a BLE UART terminal and display them on an I2C LCD.
 *  - Periodically send temperature and humidity readings from a BME280 sensor over BLE.
 *  - Clear the LCD using a specific character ('~').
 * 
 * HARDWARE MODERNIZATION:
 * ========================
 * Classic Bluetooth is not supported on the ESP32-S3. Therefore, this updated version 
 * uses BLE UART communication via the NimBLE-Arduino library, which is efficient 
 * and works well with BLE terminal apps like "nRF Connect" or "Serial Bluetooth Terminal".
 * 
 * HARDWARE INFORMATION:
 * =====================
 * - ESP32-S3-DevKitC-1 v1.1 (GPIO38 WS2812 LED not used here)
 * - I2C LCD display (16x2)
 * - BME280 temperature & humidity sensor (I2C)
 * - External BLE central device (e.g., phone with BLE terminal app)
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
 * COMPONENTS:
 * ===========
 * - ESP32-S3-DevKitC-1 v1.1
 * - BME280 sensor (I2C, address 0x76 or 0x77)
 * - 16x2 LCD with I2C backpack (default address: 0x27)
 * - BLE UART app (e.g., Serial Bluetooth Terminal)
 * 
 * GPIO ASSIGNMENTS:
 * =================
 * SDA → GPIO21
 * SCL → GPIO22
 * 
 * TECHNICAL NOTES:
 * ================
 * - Uses NimBLE-Arduino for BLE UART communication.
 * - LCD buffer is cleared after 2 rows are filled or if '~' is received.
 * - BME280 readings are updated every ~10 seconds (200×50ms).
 * - Use dtostrf for float formatting.
 * 
 * IDE REQUIREMENTS:
 * =================
 * - Arduino IDE 2.0+
 * - ESP32 Arduino Core v3.0+
 * - Board: "ESP32S3 Dev Module"
 * - Libraries:
 *   - NimBLE-Arduino (https://github.com/h2zero/NimBLE-Arduino)
 *   - LiquidCrystal_I2C by John Rickman
 *   - Adafruit BME280 and Adafruit Unified Sensor
 * 
 * Created: April 8 2019 by Peter Dalmaris
 * Updated: July 30 2025 for ESP32-S3 compatibility (BLE UART replacement)
 */

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <NimBLEDevice.h>

// I2C LCD and BME280 setup
LiquidCrystal_I2C lcd(0x27, 16, 2); // Adjust if your LCD uses a different address
Adafruit_BME280 bme;                // I2C BME280 sensor

// Constants
constexpr uint8_t SDA_PIN = 21;
constexpr uint8_t SCL_PIN = 22;
constexpr float SEALEVELPRESSURE_HPA = 1013.25;
constexpr uint8_t MAX_ROWS = 1;
constexpr uint8_t MAX_COLS = 15;

// BLE service/characteristics UUIDs
#define SERVICE_UUID         "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_RX_UUID "6E400002-B5A3-F393-E0A9-E50E24DCCA9E" // Write
#define CHARACTERISTIC_TX_UUID "6E400003-B5A3-F393-E0A9-E50E24DCCA9E" // Notify

// BLE server and characteristics
NimBLECharacteristic *txCharacteristic;
bool deviceConnected = false;

int lazyCounter = 0;
uint8_t col = 0;
uint8_t row = 0;

// Helper: clear LCD
void resetLCD() {
  col = 0;
  row = 0;
  lcd.clear();
}

// BLE callbacks
class UARTCallbacks : public NimBLECharacteristicCallbacks {
  void onWrite(NimBLECharacteristic* characteristic) {  // Remove 'override'
    std::string rxValue = characteristic->getValue();
    for (char c : rxValue) {
      if (c == '~') {
        resetLCD(); // OK after Fix 2
      } else {
        lcd.setCursor(col, row);
        lcd.print(c);
        col++;
        if (col > MAX_COLS) {
          col = 0;
          row++;
        }
        if (row > MAX_ROWS) {
          resetLCD();
        }
      }
    }
  }
};

class ServerCallbacks : public NimBLEServerCallbacks {
  void onConnect(NimBLEServer* pServer) {  // Remove 'override'
    deviceConnected = true;
  }

  void onDisconnect(NimBLEServer* pServer) {  // Remove 'override'
    deviceConnected = false;
    NimBLEDevice::startAdvertising();
  }
};

// Setup function
void setup() {
  Wire.begin(SDA_PIN, SCL_PIN);
  Serial.begin(115200);

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Ready to receive");
  delay(2000);
  lcd.clear();

  if (!bme.begin(0x76)) {
    Serial.println("Could not find BME280 sensor!");
    while (1);
  }

  NimBLEDevice::init("ESP32S3-BLE-Display");
  NimBLEServer *pServer = NimBLEDevice::createServer();
  pServer->setCallbacks(new ServerCallbacks());

  NimBLEService *pService = pServer->createService(SERVICE_UUID);

  NimBLECharacteristic *rxCharacteristic = pService->createCharacteristic(
      CHARACTERISTIC_RX_UUID,
      NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::WRITE_NR);
  rxCharacteristic->setCallbacks(new UARTCallbacks());

  txCharacteristic = pService->createCharacteristic(
      CHARACTERISTIC_TX_UUID,
      NIMBLE_PROPERTY::NOTIFY);

  pService->start();
  NimBLEAdvertising *pAdvertising = NimBLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->start();

  Serial.println("BLE UART Service started");
}

// Main loop
void loop() {
  if (lazyCounter >= 200) {
    if (deviceConnected) {
      sendSensorReadings();
    }
    lazyCounter = 0;
  }

  lazyCounter++;
  delay(50);
}



// Helper: send sensor data via BLE UART
void sendSensorReadings() {
  float temp = bme.readTemperature();
  float humi = bme.readHumidity();

  char msg[64];
  snprintf(msg, sizeof(msg), "Temperature: %.2f C\nHumidity: %.2f %%\n", temp, humi);
  txCharacteristic->setValue((uint8_t*)msg, strlen(msg));
  txCharacteristic->notify();
}
