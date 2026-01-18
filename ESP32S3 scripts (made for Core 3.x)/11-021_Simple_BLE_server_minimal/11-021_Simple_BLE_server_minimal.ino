/*  11.021 - Simple BLE Server (Minimal) for ESP32-S3
 * 
 * Course: IoT Development with ESP32-S3
 * Target Hardware: ESP32-S3-DevKitC-1 v1.1
 * Module: ESP32-S3-WROOM-1-N16R16V
 * 
 * Minimal implementation of a BLE server with two characteristics
 * using the NimBLE library. This version is streamlined for
 * beginner students to get started quickly.
 * 
 * Created: April 9, 2019 by Peter Dalmaris
 * Updated: July 30, 2025 for ESP32-S3 using NimBLE
 */

#include <NimBLEDevice.h>

#define SERVICE_UUID         "ce3fa4b9-9d6d-4787-963f-2afab5cc061e"
#define CHARACTERISTIC_UUID1 "32e97b51-92d1-4911-94f9-833213e35892"
#define CHARACTERISTIC_UUID2 "9035bf14-6416-4b6e-9a74-fcd74682e4e1"

void setup() {
    Serial.begin(115200);
    NimBLEDevice::init("ESP32S3_MinimalBLE");
    NimBLEServer* server = NimBLEDevice::createServer();
    NimBLEService* service = server->createService(SERVICE_UUID);

    service->createCharacteristic(CHARACTERISTIC_UUID1, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE)->setValue("Hello Char 1");
    service->createCharacteristic(CHARACTERISTIC_UUID2, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE)->setValue("Hello Char 2");

    service->start();
    NimBLEDevice::getAdvertising()->addServiceUUID(SERVICE_UUID);
    NimBLEDevice::getAdvertising()->start();
}

void loop() {
    delay(1000); // Main loop not used
}
