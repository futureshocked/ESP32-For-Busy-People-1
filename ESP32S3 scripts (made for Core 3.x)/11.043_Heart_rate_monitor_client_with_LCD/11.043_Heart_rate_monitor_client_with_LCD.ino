/*  11.043 - Heart Rate Monitor BLE Client with LCD Display for ESP32-S3
 *
 *  Course: IoT Development with ESP32-S3
 *  Target Hardware: ESP32-S3-DevKitC-1 v1.1
 *  Module: ESP32-S3-WROOM-1-N16R16V
 *
 *  This sketch connects the ESP32-S3 to a BLE heart rate monitor (e.g., Polar H7),
 *  subscribes to heart rate notifications using the NimBLE stack (required for ESP32-S3),
 *  and displays the heart rate on an I2C 16x2 LCD screen. It demonstrates BLE client setup,
 *  service and characteristic discovery, and asynchronous notification handling using lambdas.
 *
 *  HARDWARE INFORMATION:
 *  =====================
 *  - ESP32-S3-DevKitC-1 v1.1 board
 *  - 16 MB Flash, 16 MB PSRAM
 *  - Native USB, ADC1: GPIO1–10, ADC2: GPIO11–20
 *  - GPIOs 35–37 are reserved for flash/PSRAM and should not be used
 *
 *  CIRCUIT DIAGRAM:
 *  ================
 *  ESP32-S3        I2C LCD
 *  -----------     ----------
 *  GPIO10   <----> SDA
 *  GPIO8    <----> SCL
 *  5V       <----> Vcc
 *  GND      <----> GND
 *
 *  COMPONENTS:
 *  ===========
 *  - ESP32-S3-DevKitC-1 v1.1
 *  - I2C 16x2 LCD with backpack (0x27)
 *  - Polar H7 or compatible BLE heart rate monitor
 *  - Jumper wires
 *
 *  GPIO ASSIGNMENTS:
 *  =================
 *  SDA: GPIO10
 *  SCL: GPIO8
 *
 *  TECHNICAL NOTES:
 *  ================
 *  - Uses lambda function for characteristic notification callback
 *  - Uses NimBLEScanCallbacks for device discovery
 *  - LCD is updated only on new data
 *  - Handles disconnections gracefully
 *
 *  IDE REQUIREMENTS:
 *  =================
 *  - Arduino IDE 2.0+
 *  - ESP32 Arduino Core v3.0+
 *  - Board: "ESP32S3 Dev Module"
 *
 *  Created: April 9, 2019 by Peter Dalmaris
 *  Updated: July 30, 2025 for ESP32-S3 compatibility
 */

#include <NimBLEDevice.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// ====================== Configuration Constants ======================
constexpr uint8_t I2C_ADDRESS = 0x27;
constexpr uint8_t LCD_COLS = 16;
constexpr uint8_t LCD_ROWS = 2;
constexpr uint8_t I2C_SDA = 10;
constexpr uint8_t I2C_SCL = 8;

const NimBLEUUID SERVICE_UUID("180D");   // Heart Rate Service
const NimBLEUUID CHAR_UUID("2A37");      // Heart Rate Measurement

// ========================== Global Variables =========================
LiquidCrystal_I2C lcd(I2C_ADDRESS, LCD_COLS, LCD_ROWS);
NimBLEAdvertisedDevice* foundDevice = nullptr;
NimBLEClient* bleClient = nullptr;
bool connected = false;

// ========================== BLE Callbacks ============================
class ClientCallbacks : public NimBLEClientCallbacks {
    void onConnect(NimBLEClient* client) override {
        Serial.println("✅ Connected to BLE server");
        connected = true;
    }

    void onDisconnect(NimBLEClient* client, int reason) override {
        Serial.printf("❌ Disconnected, reason = %d\n", reason);
        connected = false;
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Disconnected");
    }
};

class AdvertisedScanCB : public NimBLEScanCallbacks {
    void onResult(NimBLEAdvertisedDevice* advertisedDevice) {
        if (advertisedDevice->isAdvertisingService(SERVICE_UUID)) {
            Serial.println("📡 Found target device:");
            Serial.println(advertisedDevice->toString().c_str());
            NimBLEDevice::getScan()->stop();
            foundDevice = new NimBLEAdvertisedDevice(*advertisedDevice);  // clone
        }
    }
};

// ========================== BLE Connection ===========================
bool connectToHeartMonitor() {
    bleClient = NimBLEDevice::createClient();
    bleClient->setClientCallbacks(new ClientCallbacks(), false);

    if (!bleClient->connect(foundDevice)) {
        Serial.println("❌ Failed to connect to BLE server");
        NimBLEDevice::deleteClient(bleClient);
        return false;
    }

    NimBLERemoteService* service = bleClient->getService(SERVICE_UUID);
    if (!service) {
        Serial.println("❌ Heart rate service not found");
        bleClient->disconnect();
        return false;
    }

    NimBLERemoteCharacteristic* characteristic = service->getCharacteristic(CHAR_UUID);
    if (!characteristic || !characteristic->canNotify()) {
        Serial.println("❌ Heart rate characteristic not found or can't notify");
        bleClient->disconnect();
        return false;
    }

    // Lambda function handles heart rate notification
    characteristic->subscribe(true, [](NimBLERemoteCharacteristic* ch, uint8_t* data, size_t len, bool isNotify) {
        if (len < 2) return;
        uint8_t hr = data[1];
        Serial.printf("❤️ Heart Rate: %d bpm\n", hr);

        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Heart Rate:");
        lcd.setCursor(12, 0);
        lcd.print(hr);

        lcd.setCursor(0, 1);
        lcd.print(hr > 60 ? "He's alive!  " : "He's a robot?");
    });

    Serial.println("✅ Subscribed to heart rate notifications");
    return true;
}

// ============================ Scanner ================================
void scanForMonitor() {
    NimBLEScan* scan = NimBLEDevice::getScan();
    scan->setScanCallbacks(new AdvertisedScanCB(), false);
    scan->setActiveScan(true);
    scan->start(5, false);
}

// =============================== Setup ===============================
void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("🔎 Starting BLE heart monitor client");

    Wire.begin(I2C_SDA, I2C_SCL);
    lcd.init();
    lcd.backlight();
    lcd.setCursor(0, 0);
    lcd.print("Scanning BLE...");

    NimBLEDevice::init("");
    NimBLEDevice::setPower(ESP_PWR_LVL_P9);

    scanForMonitor();
}

// =============================== Loop ================================
void loop() {
    if (!connected && foundDevice) {
        if (connectToHeartMonitor()) {
            Serial.println("✅ Connection successful");
        } else {
            Serial.println("🔄 Retrying scan...");
            foundDevice = nullptr;
            scanForMonitor();
        }
    }

    if (!connected && !foundDevice) {
        scanForMonitor();
    }

    delay(500);
}
