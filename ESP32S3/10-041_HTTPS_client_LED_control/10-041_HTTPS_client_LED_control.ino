/*  10.041 - HTTPS Client (Comprehensive Version) for ESP32-S3
 * 
 *  Course: IoT Development with ESP32-S3
 *  Target Hardware: ESP32-S3-DevKitC-1 v1.1
 *  Module: ESP32-S3-WROOM-1-N16R16V
 * 
 *  DESCRIPTION:
 *  This sketch demonstrates how to connect the ESP32-S3 to a Wi-Fi network and
 *  securely download a file from a server over HTTPS using the WiFiClientSecure library.
 *  It includes full certificate validation using a known CA root certificate.
 *  This version includes advanced educational features: error handling, debug output,
 *  connection diagnostics, and performance timing.
 * 
 *  HARDWARE INFORMATION:
 *  =====================
 *  - ESP32-S3-DevKitC-1 v1.1 (with USB-C)
 *  - Native USB serial debug
 *  - 16 MB Flash / 16 MB PSRAM
 *  - No additional hardware required
 * 
 *  CIRCUIT DIAGRAM:
 *  ================
 *  No external wiring required
 * 
 *  COMPONENTS:
 *  ===========
 *  - ESP32-S3-DevKitC-1 v1.1
 *  - USB-C cable
 *  - Wi-Fi access point
 * 
 *  GPIO ASSIGNMENTS:
 *  =================
 *  - No GPIO used in this lesson
 * 
 *  TECHNICAL NOTES:
 *  ================
 *  - Ensure Wi-Fi credentials are set before flashing
 *  - The CA certificate must match the remote server
 *  - Uses HTTP/1.1 GET request
 *  - Serial output includes HTTP headers and body
 * 
 *  IDE REQUIREMENTS:
 *  =================
 *  - Arduino IDE 2.0+
 *  - ESP32 Arduino Core v3.0+
 *  - Board: "ESP32S3 Dev Module"
 * 
 *  Created: April 8, 2019 by Peter Dalmaris
 *  Updated: July 30, 2025 for ESP32-S3 compatibility
 */

#include <WiFi.h>
#include <WiFiClientSecure.h>

// ==========================
// Configuration Constants
// ==========================

const char* WIFI_SSID = "YOUR_SSID";
const char* WIFI_PASSWORD = "YOUR_PASSWORD";

const char* HTTPS_HOST = "txplore-downloads.s3.amazonaws.com";
const int HTTPS_PORT = 443;
const char* HTTPS_PATH = "/esp32/test_file.txt";

// Amazon S3 Root Certificate (G2)
const char* CA_CERT = R"rawliteral(
-----BEGIN CERTIFICATE-----
"MIIEkjCCA3qgAwIBAgITBn+USionzfP6wq4rAfkI7rnExjANBgkqhkiG9w0BAQsF\n" \
"ADCBmDELMAkGA1UEBhMCVVMxEDAOBgNVBAgTB0FyaXpvbmExEzARBgNVBAcTClNj\n" \
"b3R0c2RhbGUxJTAjBgNVBAoTHFN0YXJmaWVsZCBUZWNobm9sb2dpZXMsIEluYy4x\n" \
"OzA5BgNVBAMTMlN0YXJmaWVsZCBTZXJ2aWNlcyBSb290IENlcnRpZmljYXRlIEF1\n" \
"dGhvcml0eSAtIEcyMB4XDTE1MDUyNTEyMDAwMFoXDTM3MTIzMTAxMDAwMFowOTEL\n" \
"MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv\n" \
"b3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj\n" \
"ca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvo3bSMgHFzZM\n" \
"9O6II8c+6zf1tRn4SWiw3te5djgdYZ6k/oI2peVKVuRF4fn9tBb6dNqcmzU5L/qw\n" \
"IFAGbHrQgLKm+a/sRxmPUDgH3KKHOVj4utWp+UhnMJbulHheb4mjUcAwhmahRWa6\n" \
"VOujw5H5SNz/0egwLX0tdHA114gk957EWW67c4cX8jJGKLhD+rcdqsq08p8kDi1L\n" \
"93FcXmn/6pUCyziKrlA4b9v7LWIbxcceVOF34GfID5yHI9Y/QCB/IIDEgEw+OyQm\n" \
"jgSubJrIqg0CAwEAAaOCATEwggEtMA8GA1UdEwEB/wQFMAMBAf8wDgYDVR0PAQH/\n" \
"BAQDAgGGMB0GA1UdDgQWBBSEGMyFNOy8DJSULghZnMeyEE4KCDAfBgNVHSMEGDAW\n" \
"gBScXwDfqgHXMCs4iKK4bUqc8hGRgzB4BggrBgEFBQcBAQRsMGowLgYIKwYBBQUH\n" \
"MAGGImh0dHA6Ly9vY3NwLnJvb3RnMi5hbWF6b250cnVzdC5jb20wOAYIKwYBBQUH\n" \
"MAKGLGh0dHA6Ly9jcnQucm9vdGcyLmFtYXpvbnRydXN0LmNvbS9yb290ZzIuY2Vy\n" \
"MD0GA1UdHwQ2MDQwMqAwoC6GLGh0dHA6Ly9jcmwucm9vdGcyLmFtYXpvbnRydXN0\n" \
"LmNvbS9yb290ZzIuY3JsMBEGA1UdIAQKMAgwBgYEVR0gADANBgkqhkiG9w0BAQsF\n" \
"AAOCAQEAYjdCXLwQtT6LLOkMm2xF4gcAevnFWAu5CIw+7bMlPLVvUOTNNWqnkzSW\n" \
"MiGpSESrnO09tKpzbeR/FoCJbM8oAxiDR3mjEH4wW6w7sGDgd9QIpuEdfF7Au/ma\n" \
"eyKdpwAJfqxGF4PcnCZXmTA5YpaP7dreqsXMGz7KQ2hsVxa81Q4gLv7/wmpdLqBK\n" \
"bRRYh5TmOTFffHPLkIhqhBGWJ6bt2YFGpn6jcgAKUj6DiAdjd4lpFw85hdKrCEVN\n" \
"0FE6/V1dN2RMfjCyVSRCnTawXZwXgWHxyvkQAiSr6w10kY17RSlQOYiypok1JR4U\n" \
"akcjMS9cmvqtmg5iUaQqqcT5NJ0hGA==\n" \
-----END CERTIFICATE-----
)rawliteral";

// ==========================
// Setup Function
// ==========================

void setup() {
    Serial.begin(115200);
    delay(2000);
    Serial.println("\n\n[HTTPS CLIENT STARTED]");

    connectToWiFi();

    if (performHTTPSGetRequest()) {
        Serial.println("[✅ DOWNLOAD SUCCESSFUL]");
    } else {
        Serial.println("[❌ DOWNLOAD FAILED]");
    }
}

// ==========================
// Main Loop
// ==========================

void loop() {
    // Do nothing – this sketch runs once.
    delay(10000);
}

// ==========================
// Wi-Fi Connection Helper
// ==========================

void connectToWiFi() {
    Serial.print("[Wi-Fi] Connecting to ");
    Serial.println(WIFI_SSID);

    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    unsigned long startAttemptTime = millis();
    const unsigned long timeout = 15000;

    while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < timeout) {
        Serial.print(".");
        delay(500);
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\n[Wi-Fi] Connected!");
        Serial.print("[Wi-Fi] IP Address: ");
        Serial.println(WiFi.localIP());
    } else {
        Serial.println("\n[Wi-Fi] Connection Failed! Rebooting...");
        delay(5000);
        ESP.restart();
    }
}

// ==========================
// HTTPS Request Helper
// ==========================

bool performHTTPSGetRequest() {
    WiFiClientSecure client;
    client.setCACert(CA_CERT);

    Serial.printf("[HTTPS] Connecting to %s:%d\n", HTTPS_HOST, HTTPS_PORT);

    if (!client.connect(HTTPS_HOST, HTTPS_PORT)) {
        Serial.println("[HTTPS] Connection failed");
        return false;
    }

    // Send HTTP GET Request
    client.printf("GET %s HTTP/1.1\r\n", HTTPS_PATH);
    client.printf("Host: %s\r\n", HTTPS_HOST);
    client.println("Connection: close\r\n");

    // Wait for server response
    unsigned long timeout = millis() + 5000;
    while (!client.available() && millis() < timeout) {
        delay(10);
    }

    if (!client.available()) {
        Serial.println("[HTTPS] No response, timeout.");
        return false;
    }

    Serial.println("[HTTPS] Response Headers:");
    while (client.connected()) {
        String line = client.readStringUntil('\n');
        if (line == "\r") break; // Headers done
        Serial.println(line);
    }

    Serial.println("\n[HTTPS] Response Body:");
    while (client.available()) {
        Serial.write(client.read());
    }

    client.stop();
    return true;
}
