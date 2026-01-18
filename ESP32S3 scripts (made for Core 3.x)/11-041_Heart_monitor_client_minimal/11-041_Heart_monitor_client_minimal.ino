/*  11.041 - BLE Heart Rate Monitor Client (Minimal) for ESP32-S3
 * 
 * Course: IoT Development with ESP32-S3
 * Target Hardware: ESP32-S3-DevKitC-1 v1.1
 * Module: ESP32-S3-WROOM-1-N16R16V
 * 
 * DESCRIPTION:
 * ============
 * This minimal example shows how to connect the ESP32-S3 to a BLE heart rate monitor
 * and receive heart rate data via BLE notifications.
 * 
 * The sketch uses the NimBLE library, which is required for ESP32-S3. It is compatible 
 * with heart rate monitors such as the Polar H7 that support the Heart Rate Service (UUID 180D).
 * 
 * HARDWARE INFORMATION:
 * =====================
 * - Board: ESP32-S3-DevKitC-1 v1.1
 * - Module: ESP32-S3-WROOM-1-N16R16V
 * - No GPIOs or external components needed
 * 
 * CIRCUIT DIAGRAM:
 * ================
 * [ESP32-S3]~~~(((BLE)))~~~>[Heart Rate Monitor Device]
 * 
 * COMPONENTS:
 * ===========
 * - ESP32-S3-DevKitC-1 v1.1
 * - BLE heart rate monitor (e.g., Polar H7)
 * 
 * GPIO ASSIGNMENTS:
 * =================
 * None used
 * 
 * TECHNICAL NOTES:
 * ================
 * - Uses NimBLE-Arduino library (lightweight and compatible with ESP32-S3)
 * - Actively scans for BLE devices advertising the Heart Rate Service
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

NimBLEUUID serviceUUID("180D");
NimBLEUUID charUUID("2A37");

bool doConnect = false;
bool connected = false;
NimBLEAdvertisedDevice* targetDevice = nullptr;
NimBLERemoteCharacteristic* hrChar = nullptr;

void onNotify(NimBLERemoteCharacteristic* pChar, uint8_t* data, size_t len, bool) {
  if (len > 1) {
    Serial.print("Heart rate: ");
    Serial.print(data[1]);
    Serial.println(" BPM");
  }
}

class AdvertisedDeviceCallbacks : public NimBLEScanCallbacks {
  void onResult(NimBLEAdvertisedDevice* dev) {
    if (dev->isAdvertisingService(serviceUUID)) {
      NimBLEDevice::getScan()->stop();
      targetDevice = dev;
      doConnect = true;
    }
  }
};

bool connectToDevice() {
  auto client = NimBLEDevice::createClient();
  client->connect(targetDevice);
  auto service = client->getService(serviceUUID);
  if (!service) return false;
  hrChar = service->getCharacteristic(charUUID);
  if (!hrChar || !hrChar->canNotify()) return false;
  return hrChar->subscribe(true, onNotify);
}

void setup() {
  Serial.begin(115200);
  NimBLEDevice::init("");
  NimBLEDevice::getScan()->setScanCallbacks(new AdvertisedDeviceCallbacks());
  NimBLEDevice::getScan()->setActiveScan(true);
  NimBLEDevice::getScan()->start(5, false);
}

void loop() {
  if (doConnect) {
    if (connectToDevice()) {
      Serial.println("Subscribed to heart rate notifications.");
      connected = true;
    } else {
      Serial.println("Connection failed.");
    }
    doConnect = false;
  }
  delay(100);
}
