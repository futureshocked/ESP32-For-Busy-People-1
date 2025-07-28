/*  08.031 - Reading and Writing from SPIFFS for ESP32-S3 (Minimal)
 * 
 * Course: IoT Development with ESP32-S3
 * Target Hardware: ESP32-S3-DevKitC-1 v1.1
 * Module: ESP32-S3-WROOM-1-N16R16V
 * 
 * Demonstrates basic SPIFFS operations: mount, write, read, append.
 * 
 * COMPONENTS:
 *  - ESP32-S3-DevKitC-1
 * 
 * IDE REQUIREMENTS:
 *  - Arduino IDE 2.0+
 *  - ESP32 Arduino Core v3.0+
 *  - Board: "ESP32S3 Dev Module"
 * 
 * Created: April 3, 2019 by Peter Dalmaris
 * Updated: July 28, 2025 for ESP32-S3 compatibility
 */

#include <SPIFFS.h>

void setup() {
    Serial.begin(115200);
    delay(1000);

    if (!SPIFFS.begin(true)) {
        Serial.println("SPIFFS mount failed.");
        return;
    }

    File file = SPIFFS.open("/data.txt", FILE_WRITE);
    if (file) {
        file.print("Hello SPIFFS!\n");
        file.close();
    }

    file = SPIFFS.open("/data.txt", FILE_APPEND);
    if (file) {
        file.print("Appended line.\n");
        file.close();
    }

    file = SPIFFS.open("/data.txt", FILE_READ);
    if (file) {
        while (file.available()) {
            Serial.write(file.read());
        }
        file.close();
    }
}

void loop() {
  // Nothing here
}
