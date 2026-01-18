/*  11.043 - Minimal Heart Rate Monitor BLE Client with LCD for ESP32-S3
 *
 *  Course: IoT Development with ESP32-S3
 *  Target Hardware: ESP32-S3-DevKitC-1 v1.1
 *  Module: ESP32-S3-WROOM-1-N16R16V
 *
 *  This simplified sketch connects to a BLE heart rate monitor (e.g., Polar H7),
 *  subscribes to heart rate notifications, and displays the heart rate on an I2C LCD.
 *  It uses the NimBLE library, required for ESP32-S3, and an LCD with I2C backpack.
 *
 *  HARDWARE INFORMATION:
 *  =====================
 *  - ESP32-S3-DevKitC-1 v1.1 board
 *  - 16 MB Flash, 16 MB PSRAM
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
 *  - I2C 16x2 LCD (address 0x27)
 *  - BLE Heart Rate Monitor (e.g., Polar H7)
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
#include <LiquidCrystal_I2C.h>
#include <Wire.h>

// === Config ===
constexpr uint8_t SDA_PIN = 10;
constexpr uint8_t SCL_PIN = 8;
constexpr uint8_t LCD_ADDR = 0x27;
constexpr uint8_t LCD_COLS = 16;
constexpr uint8_t LCD_ROWS = 2;

const NimBLEUUID SERVICE_UUID("180D");
const NimBLEUUID CHAR_UUID("2A37");

LiquidCrystal_I2C lcd(LCD_ADDR, LCD_COLS, LCD_ROWS);
NimBLEAdvertisedDevice* foundDevice = nullptr;
NimBLEClient* client = nullptr;
bool connected = false;

// === BLE Callbacks ===
class ScanCB : public NimBLEScanCallbacks {
    void onResult(NimBLEAdvertisedDevice* device) {
        if (device->isAdvertisingService(SERVICE_UUID)) {
            NimBLEDevice::getScan()->stop();
            foundDevice = new NimBLEAdvertisedDevice(*device);  // Deep copy
        }
    }
};

class ClientCB : public NimBLEClientCallbacks {
    void onConnect(NimBLEClient* c) { connected = true; }
    void onDisconnect(NimBLEClient* c, int reason) {
        connected = false;
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Disconnected");
    }
};

// === Functions ===
bool connectToMonitor() {
    client = NimBLEDevice::createClient();
    client->setClientCallbacks(new ClientCB(), false);
    if (!client->connect(foundDevice)) return false;

    NimBLERemoteService* svc = client->getService(SERVICE_UUID);
    if (!svc) return false;

    NimBLERemoteCharacteristic* ch = svc->getCharacteristic(CHAR_UUID);
    if (!ch || !ch->canNotify()) return false;

    ch->subscribe(true, [](NimBLERemoteCharacteristic* c, uint8_t* data, size_t len, bool notify) {
        if (len < 2) return;
        uint8_t hr = data[1];
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("HR: ");
        lcd.print(hr);
    });

    return true;
}

void startScan() {
    NimBLEScan* scan = NimBLEDevice::getScan();
    scan->setScanCallbacks(new ScanCB());
    scan->setActiveScan(true);
    scan->start(5, false);
}

// === Setup/Loop ===
void setup() {
    Serial.begin(115200);
    Wire.begin(SDA_PIN, SCL_PIN);
    lcd.init();
    lcd.backlight();
    lcd.setCursor(0, 0);
    lcd.print("Scanning...");

    NimBLEDevice::init("");
    startScan();
}

void loop() {
    if (!connected && foundDevice) {
        if (!connectToMonitor()) {
            foundDevice = nullptr;
            startScan();
        }
    }
    if (!connected && !foundDevice) startScan();
    delay(500);
}
