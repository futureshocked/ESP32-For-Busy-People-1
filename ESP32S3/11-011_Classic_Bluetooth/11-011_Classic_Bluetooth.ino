/*  11.011 - BLE UART Communication using NimBLE on ESP32-S3
 * 
 * Course: IoT Development with ESP32-S3
 * Target Hardware: ESP32-S3-DevKitC-1 v1.1
 * Module: ESP32-S3-WROOM-1-N16R16V
 * 
 * This sketch demonstrates how to establish a wireless serial bridge over 
 * Bluetooth Low Energy (BLE) using the NimBLE library.
 * 
 * Because the ESP32-S3 does not support Classic Bluetooth (BR/EDR), we 
 * use BLE UART emulation to provide similar functionality to the 
 * legacy `BluetoothSerial` class.
 * 
 * The BLE UART service emulates a serial connection using the Nordic UART Service (NUS)
 * protocol, enabling communication with mobile devices or desktop tools like 
 * nRF Connect, Serial Bluetooth Terminal (Android), or LightBlue (iOS).
 * 
 * HARDWARE INFORMATION:
 * =====================
 * - SoC: ESP32-S3-WROOM-1-N16R16V
 * - Flash: 16MB
 * - PSRAM: 16MB
 * - USB: USB-C (native USB)
 * - Bluetooth: BLE 5.0 only (no Classic BT support)
 * 
 * CIRCUIT DIAGRAM:
 * ================
 * [No external components required]
 * 
 * COMPONENTS:
 * ===========
 * - ESP32-S3-DevKitC-1 v1.1
 * - USB-C cable for programming and power
 * 
 * GPIO ASSIGNMENTS:
 * =================
 * None used (communication over BLE only)
 * 
 * TECHNICAL NOTES:
 * ================
 * - BLE UART uses GATT services to transmit and receive serial data.
 * - Compatible with nRF Connect or Serial Bluetooth Terminal for testing.
 * - This script uses the NimBLE library for lower memory usage and improved performance.
 * 
 * IDE REQUIREMENTS:
 * =================
 * - Arduino IDE 2.0+
 * - ESP32 Arduino Core v3.0+
 * - Board: "ESP32S3 Dev Module"
 * - Library: NimBLE-Arduino (https://github.com/h2zero/NimBLE-Arduino)
 * 
 * Created: April 8, 2019 by Peter Dalmaris
 * Updated: July 30, 2025 for ESP32-S3 compatibility by ChatGPT
 */

#include <NimBLEDevice.h>

// UUIDs for Nordic UART Service (NUS)
#define UART_SERVICE_UUID        "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
#define UART_CHAR_TX_UUID        "6E400003-B5A3-F393-E0A9-E50E24DCCA9E" // Notify
#define UART_CHAR_RX_UUID        "6E400002-B5A3-F393-E0A9-E50E24DCCA9E" // Write

NimBLECharacteristic* uartTxCharacteristic;
NimBLECharacteristic* uartRxCharacteristic;

class UartRxCallbacks : public NimBLECharacteristicCallbacks {
    void onWrite(NimBLECharacteristic* characteristic) {
        std::string rxValue = characteristic->getValue();
        if (!rxValue.empty()) {
            Serial.print("Received over BLE: ");
            Serial.println(rxValue.c_str());

            // Echo back
            uartTxCharacteristic->setValue(rxValue);
            uartTxCharacteristic->notify();
        }
    }
};

void setup() {
    Serial.begin(115200);
    delay(2000);
    Serial.println("Starting BLE UART server...");

    NimBLEDevice::init("ESP32S3-BLE-UART");
    NimBLEServer* server = NimBLEDevice::createServer();

    NimBLEService* uartService = server->createService(UART_SERVICE_UUID);

    uartTxCharacteristic = uartService->createCharacteristic(
        UART_CHAR_TX_UUID,
        NIMBLE_PROPERTY::NOTIFY
    );

    uartRxCharacteristic = uartService->createCharacteristic(
        UART_CHAR_RX_UUID,
        NIMBLE_PROPERTY::WRITE
    );
    uartRxCharacteristic->setCallbacks(new UartRxCallbacks());

    uartService->start();
    server->getAdvertising()->start();

    Serial.println("BLE UART server started. Ready to connect.");
}

void loop() {
    // Nothing needed here; everything is event-driven via callbacks
}
