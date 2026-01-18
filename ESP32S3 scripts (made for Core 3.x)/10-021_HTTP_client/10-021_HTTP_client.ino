/*  10.021 - Simple HTTP GET Client for ESP32-S3
 * 
 * Course: IoT Development with ESP32-S3
 * Target Hardware: ESP32-S3-DevKitC-1 v1.1
 * Module: ESP32-S3-WROOM-1-N16R16V
 * 
 * This sketch demonstrates how to perform a simple HTTP GET request using the WiFiClient class.
 * It connects to a Wi-Fi network and retrieves a text file from a remote server.
 * The file is then printed to the serial monitor.
 * 
 * This modernized version supports the ESP32-S3 platform using the latest ESP32 Arduino Core v3.0+.
 * 
 * HARDWARE INFORMATION:
 * =====================
 * - Board: ESP32-S3-DevKitC-1 v1.1
 * - SoC: ESP32-S3-WROOM-1-N16R16V
 * - Flash: 16 MB, PSRAM: 16 MB
 * - No onboard sensors or peripherals required
 * 
 * CIRCUIT DIAGRAM:
 * ================
 * No external components required.
 * 
 * COMPONENTS:
 * ===========
 * - ESP32-S3-DevKitC-1 v1.1 board
 * - USB-C cable for power and programming
 * - Computer with Arduino IDE 2.0+
 * 
 * GPIO ASSIGNMENTS:
 * =================
 * Not applicable – no GPIO used in this sketch.
 * 
 * TECHNICAL NOTES:
 * ================
 * - Ensure the board is configured as "ESP32S3 Dev Module" in Arduino IDE.
 * - The ESP32-S3 supports native USB and lacks some legacy GPIO peripherals.
 * - This sketch is fully compatible with modern WiFi APIs and ESP32-S3 core.
 * 
 * IDE REQUIREMENTS:
 * =================
 * - Arduino IDE 2.0+
 * - ESP32 Arduino Core v3.0+
 * - Board: "ESP32S3 Dev Module"
 * 
 * Created: April 8, 2019 by Peter Dalmaris
 * Updated: July 30, 2025 for ESP32-S3 compatibility
 */

#include <WiFi.h>  // Wi-Fi library included in ESP32 core

//------------------------------------//
//         Configuration Constants     //
//------------------------------------//
const char* SSID     = "Your_SSID";      // Replace with your Wi-Fi SSID
const char* PASSWORD = "Your_PASSWORD";  // Replace with your Wi-Fi password

const char* HOST = "txplore-downloads.s3.amazonaws.com";
const char* URL_PATH = "/esp32/test_file.txt";

const uint16_t HTTP_PORT = 80;
const uint32_t REQUEST_INTERVAL_MS = 10000;

//------------------------------------//
//         Setup Function             //
//------------------------------------//
void setup() {
    Serial.begin(115200);
    delay(500);

    Serial.println("\n\n[WiFi] Connecting to network...");
    WiFi.begin(SSID, PASSWORD);

    uint8_t retries = 0;
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
        if (++retries > 40) {
            Serial.println("\n[WiFi] Failed to connect. Rebooting...");
            delay(2000);
            ESP.restart();
        }
    }

    Serial.println("\n[WiFi] Connected.");
    Serial.print("[WiFi] IP address: ");
    Serial.println(WiFi.localIP());
}

//------------------------------------//
//           Main Loop                //
//------------------------------------//
void loop() {
    delay(REQUEST_INTERVAL_MS);
    fetchRemoteText();
}

//------------------------------------//
//     Helper Function: Fetch File    //
//------------------------------------//
void fetchRemoteText() {
    WiFiClient client;
    Serial.printf("\n[HTTP] Connecting to %s ...\n", HOST);

    if (!client.connect(HOST, HTTP_PORT)) {
        Serial.println("[HTTP] Connection failed.");
        return;
    }

    String request = String("GET ") + URL_PATH + " HTTP/1.1\r\n" +
                     "Host: " + HOST + "\r\n" +
                     "Connection: close\r\n\r\n";

    client.print(request);

    // Wait for server response
    unsigned long timeout = millis();
    while (!client.available()) {
        if (millis() - timeout > 5000) {
            Serial.println("[HTTP] Timeout while waiting for response.");
            client.stop();
            return;
        }
    }

    Serial.println("[HTTP] Response from server:");
    while (client.available()) {
        String line = client.readStringUntil('\n');
        Serial.println(line);
    }

    Serial.println("[HTTP] Connection closed.\n");
    client.stop();
}
