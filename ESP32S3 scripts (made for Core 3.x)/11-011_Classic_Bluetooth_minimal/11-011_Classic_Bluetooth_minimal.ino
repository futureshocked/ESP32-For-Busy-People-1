/*  11.011 - BLE UART Communication (Minimal Version)
 * 
 * Course: IoT Development with ESP32-S3
 * Target Hardware: ESP32-S3-DevKitC-1 v1.1
 * Module: ESP32-S3-WROOM-1-N16R16V
 * 
 * This script demonstrates BLE UART using the NimBLE library.
 * 
 * ESP32-S3 does not support Classic Bluetooth. This example uses BLE
 * to simulate serial communication with a mobile or desktop BLE app.
 * 
 * Use an app like nRF Connect (iOS/Android) or Serial Bluetooth Terminal (Android)
 * to connect to the device and send/receive messages.
 * 
 * Created: April 8, 2019 by Peter Dalmaris
 * Updated: July 30, 2025 for ESP32-S3 compatibility
 */

#include <NimBLEDevice.h>

NimBLECharacteristic* tx;
NimBLECharacteristic* rx;

class RxCB : public NimBLECharacteristicCallbacks {
    void onWrite(NimBLECharacteristic* c) {
        std::string msg = c->getValue();
        tx->setValue(msg);
        tx->notify();
    }
};

void setup() {
    Serial.begin(115200);
    NimBLEDevice::init("BLE-UART");

    auto server = NimBLEDevice::createServer();
    auto service = server->createService("6E400001-B5A3-F393-E0A9-E50E24DCCA9E");

    tx = service->createCharacteristic("6E400003-B5A3-F393-E0A9-E50E24DCCA9E", NIMBLE_PROPERTY::NOTIFY);
    rx = service->createCharacteristic("6E400002-B5A3-F393-E0A9-E50E24DCCA9E", NIMBLE_PROPERTY::WRITE);
    rx->setCallbacks(new RxCB());

    service->start();
    server->getAdvertising()->start();
}

void loop() {}
