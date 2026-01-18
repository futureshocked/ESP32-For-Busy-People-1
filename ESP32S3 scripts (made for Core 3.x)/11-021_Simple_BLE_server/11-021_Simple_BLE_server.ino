/*  11.021 - Simple BLE Server with Two Read/Write Characteristics for ESP32-S3
 * 
 * Course: IoT Development with ESP32-S3
 * Target Hardware: ESP32-S3-DevKitC-1 v1.1
 * Module: ESP32-S3-WROOM-1-N16R16V
 * 
 * This example creates a simple BLE peripheral (server) with two GATT characteristics,
 * each supporting read and write operations. Students can interact with this peripheral
 * using smartphone BLE apps such as "nRF Connect" or "BLE Scanner".
 * 
 * This updated version uses the NimBLE library instead of the original BLE stack, 
 * which is more resource-efficient and recommended for the ESP32-S3 platform.
 * 
 * HARDWARE INFORMATION:
 * =====================
 * - Board: ESP32-S3-DevKitC-1 v1.1
 * - Module: ESP32-S3-WROOM-1-N16R16V
 * - USB-C interface for programming and serial monitor
 * - No external hardware required
 * 
 * CIRCUIT DIAGRAM:
 * ================
 * No additional circuitry required.
 * 
 * COMPONENTS:
 * ===========
 * - ESP32-S3-DevKitC-1 v1.1 board
 * - USB-C cable
 * - BLE Scanner app (mobile)
 * 
 * GPIO ASSIGNMENTS:
 * =================
 * - Not applicable (no GPIO used in this BLE-only sketch)
 * 
 * TECHNICAL NOTES:
 * ================
 * - NimBLE is more efficient than the original Bluedroid BLE stack.
 * - ESP32-S3 supports BLE only (no Classic Bluetooth).
 * - Uses C++ string handling to extract and print characteristic values.
 * - UUIDs generated via https://www.uuidgenerator.net
 * 
 * IDE REQUIREMENTS:
 * =================
 * - Arduino IDE 2.0+
 * - ESP32 Arduino Core v3.0+
 * - Board: "ESP32S3 Dev Module"
 * - Library: NimBLE-Arduino by h2zero (https://github.com/h2zero/NimBLE-Arduino)
 * 
 * Created: April 9, 2019 by Peter Dalmaris
 * Updated: July 30, 2025 for ESP32-S3 compatibility using NimBLE
 */

// ==========================
// Include required libraries
// ==========================
#include <NimBLEDevice.h>

// ==========================
// UUID Definitions (custom)
// ==========================
#define SERVICE_UUID         "ce3fa4b9-9d6d-4787-963f-2afab5cc061e"
#define CHARACTERISTIC_UUID1 "32e97b51-92d1-4911-94f9-833213e35892"
#define CHARACTERISTIC_UUID2 "9035bf14-6416-4b6e-9a74-fcd74682e4e1"

// ==========================
// Callback Classes
// ==========================
class Char1Callbacks : public NimBLECharacteristicCallbacks {
    void onWrite(NimBLECharacteristic* pCharacteristic) {
        std::string value = pCharacteristic->getValue();
        if (!value.empty()) {
            Serial.println("*********");
            Serial.print("New value Char 1: ");
            Serial.println(value.c_str());
            Serial.println("*********");
        }
    }
};

class Char2Callbacks : public NimBLECharacteristicCallbacks {
    void onWrite(NimBLECharacteristic* pCharacteristic) {
        std::string value = pCharacteristic->getValue();
        if (!value.empty()) {
            Serial.println("*********");
            Serial.print("New value Char 2: ");
            Serial.println(value.c_str());
            Serial.println("*********");
        }
    }
};

// ==========================
// Setup BLE Server
// ==========================
void setup() {
    Serial.begin(115200);
    Serial.println("Starting BLE Server using NimBLE...");

    NimBLEDevice::init("ESP32S3_BLE_Server");

    NimBLEServer* pServer = NimBLEDevice::createServer();
    NimBLEService* pService = pServer->createService(SERVICE_UUID);

    NimBLECharacteristic* pChar1 = pService->createCharacteristic(
        CHARACTERISTIC_UUID1,
        NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE
    );
    pChar1->setValue("Hello World char 1");
    pChar1->setCallbacks(new Char1Callbacks());

    NimBLECharacteristic* pChar2 = pService->createCharacteristic(
        CHARACTERISTIC_UUID2,
        NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE
    );
    pChar2->setValue("Hello World char 2");
    pChar2->setCallbacks(new Char2Callbacks());

    pService->start();
    NimBLEAdvertising* pAdvertising = NimBLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->start();

    Serial.println("BLE Advertising started...");
}

// ==========================
// Loop (not used here)
// ==========================
void loop() {
    delay(1000);  // Nothing to do here
}
