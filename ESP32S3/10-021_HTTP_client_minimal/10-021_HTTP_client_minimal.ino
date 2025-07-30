/*  10.021 - Simple HTTP GET Client (Minimal Version)
 * 
 * Course: IoT Development with ESP32-S3
 * Target Hardware: ESP32-S3-DevKitC-1 v1.1
 * Module: ESP32-S3-WROOM-1-N16R16V
 * 
 * This is a simplified version of the HTTP GET example.
 * It connects to Wi-Fi and retrieves a file from a remote server.
 * Designed for beginners to quickly prototype and understand basic HTTP operations.
 * 
 * IDE REQUIREMENTS:
 * =================
 * - Arduino IDE 2.0+
 * - ESP32 Arduino Core v3.0+
 */

#include <WiFi.h>

const char* ssid = "Your_SSID";
const char* password = "Your_PASSWORD";

const char* host = "txplore-downloads.s3.amazonaws.com";
String url = "/esp32/test_file.txt";

void setup() {
    Serial.begin(115200);
    delay(100);

    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("\nWiFi connected.");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
}

void loop() {
    delay(10000);

    WiFiClient client;
    if (!client.connect(host, 80)) {
        Serial.println("Connection failed.");
        return;
    }

    client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                 "Host: " + host + "\r\n" +
                 "Connection: close\r\n\r\n");

    while (client.available()) {
        String line = client.readStringUntil('\n');
        Serial.println(line);
    }

    client.stop();
}
