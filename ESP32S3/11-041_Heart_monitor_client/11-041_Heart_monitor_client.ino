/*  11.041 - BLE Heart Rate Monitor Client for ESP32-S3
 * 
 * Course: IoT Development with ESP32-S3
 * Target Hardware: ESP32-S3-DevKitC-1 v1.1
 * Module: ESP32-S3-WROOM-1-N16R16V
 * 
 * DESCRIPTION:
 * ============
 * This example demonstrates how to build a BLE client application using the ESP32-S3
 * to connect to a heart rate monitor device (such as the Polar H7) and receive heart
 * rate data via BLE notifications from the Heart Rate Measurement characteristic.
 * 
 * This version uses the NimBLE library, which is the recommended BLE stack for ESP32-S3
 * due to its significantly reduced memory usage and improved performance compared to
 * the original BLEDevice-based library. All code has been updated to ensure compatibility
 * with the ESP32 Arduino Core v3.0+ and ESP32-S3 architecture.
 * 
 * HARDWARE INFORMATION:
 * =====================
 * - Board: ESP32-S3-DevKitC-1 v1.1
 * - Module: ESP32-S3-WROOM-1-N16R16V
 * - Flash: 16MB
 * - PSRAM: 16MB
 * - Native USB (GPIO19 D-, GPIO20 D+)
 * - Serial monitor via USB-C (115200 baud)
 * - No external components required for BLE operation
 * 
 * CIRCUIT DIAGRAM:
 * ================
 * [ESP32-S3]~~~(((BLE)))~~~>[Polar H7 HRM or compatible BLE heart rate monitor]
 * 
 * COMPONENTS:
 * ===========
 * - ESP32-S3-DevKitC-1 v1.1
 * - BLE heart rate monitor (e.g., Polar H7)
 * 
 * GPIO ASSIGNMENTS:
 * =================
 * No GPIOs used in this sketch.
 * 
 * TECHNICAL NOTES:
 * ================
 * - Service UUID: 0x180D (Heart Rate Service)
 * - Characteristic UUID: 0x2A37 (Heart Rate Measurement)
 * - This sketch uses BLE active scanning and connects to the first device advertising the Heart Rate service.
 * 
 * IDE REQUIREMENTS:
 * =================
 * - Arduino IDE 2.0+
 * - ESP32 Arduino Core v3.0+
 * - Board: "ESP32S3 Dev Module"
 * - Library: NimBLE-Arduino (https://github.com/h2zero/NimBLE-Arduino)
 * 
 * Created: April 9, 2019 by Peter Dalmaris
 * Updated: July 30, 2025 for ESP32-S3 compatibility using NimBLE
 */

#include <NimBLEDevice.h>

// BLE UUIDs for Heart Rate Service and Heart Rate Measurement Characteristic
static NimBLEUUID HR_SERVICE_UUID("180D");
static NimBLEUUID HR_CHAR_UUID("2A37");

// Flags
static bool doConnect = false;
static bool connected = false;
static NimBLEAdvertisedDevice* targetDevice = nullptr;
static NimBLERemoteCharacteristic* remoteHRChar = nullptr;

/**
 * Notification callback for heart rate data
 */
void onHeartRateNotify(NimBLERemoteCharacteristic* pChar, uint8_t* pData, size_t length, bool isNotify) {
    Serial.print("Heart Rate Notify (");
    Serial.print(length);
    Serial.print(" bytes): ");
    if (length > 1) {
        uint8_t bpm = pData[1]; // Second byte is BPM in standard Heart Rate Measurement
        Serial.print(bpm);
        Serial.println(" BPM");
    } else {
        Serial.println("Invalid HR data");
    }
}

/**
 * Client connection callbacks
 */
class ClientCallbacks : public NimBLEClientCallbacks {
    void onConnect(NimBLEClient* pClient) {
        Serial.println("Connected to server.");
    }

    void onDisconnect(NimBLEClient* pClient) {
        connected = false;
        Serial.println("Disconnected from server.");
    }
};

/**
 * Connect to heart rate monitor and subscribe to HR characteristic
 */
bool connectToHeartRateMonitor() {
    NimBLEClient* client = NimBLEDevice::createClient();
    client->setClientCallbacks(new ClientCallbacks(), false);
    client->connect(targetDevice);

    NimBLERemoteService* hrService = client->getService(HR_SERVICE_UUID);
    if (!hrService) {
        Serial.println("Failed to find Heart Rate service.");
        client->disconnect();
        return false;
    }

    remoteHRChar = hrService->getCharacteristic(HR_CHAR_UUID);
    if (!remoteHRChar) {
        Serial.println("Failed to find HR Measurement characteristic.");
        client->disconnect();
        return false;
    }

    if (remoteHRChar->canNotify()) {
        if (!remoteHRChar->subscribe(true, onHeartRateNotify)) {
            Serial.println("Failed to subscribe to HR notifications.");
            return false;
        }
    } else {
        Serial.println("HR characteristic does not support notifications.");
        return false;
    }

    connected = true;
    return true;
}

/**
 * Called when a BLE advertisement is received
 */
class AdvertisedDeviceCallbacks : public NimBLEScanCallbacks {
    void onResult(NimBLEAdvertisedDevice* advertisedDevice) {
        Serial.print("Found device: ");
        Serial.println(advertisedDevice->toString().c_str());

        if (advertisedDevice->isAdvertisingService(HR_SERVICE_UUID)) {
            NimBLEDevice::getScan()->stop();
            targetDevice = advertisedDevice;
            doConnect = true;
        }
    }
};

void setup() {
    Serial.begin(115200);
    Serial.println("BLE Client - Heart Rate Monitor");

    NimBLEDevice::init("");
    NimBLEDevice::setPower(ESP_PWR_LVL_P9);

    NimBLEScan* scan = NimBLEDevice::getScan();
    scan->setScanCallbacks(new AdvertisedDeviceCallbacks());
    scan->setInterval(45);
    scan->setWindow(15);
    scan->setActiveScan(true);
    scan->start(5, false);
}

void loop() {
    if (doConnect) {
        if (connectToHeartRateMonitor()) {
            Serial.println("Successfully connected and subscribed to HR notifications.");
        } else {
            Serial.println("Connection failed.");
        }
        doConnect = false;
    }

    delay(100); // Light processing loop
}
