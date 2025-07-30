/*  10.041 - HTTPS Client (Minimal Version) for ESP32-S3
 *
 *  Course: IoT Development with ESP32-S3
 *  Target Hardware: ESP32-S3-DevKitC-1 v1.1
 *  Module: ESP32-S3-WROOM-1-N16R16V
 *
 *  A minimal sketch that connects to Wi-Fi and downloads a file via HTTPS.
 *  Suitable for beginner students testing secure connections.
 *
 *  HARDWARE INFORMATION:
 *  =====================
 *  - ESP32-S3-DevKitC-1 v1.1
 *  - No external hardware required
 *
 *  CIRCUIT DIAGRAM:
 *  ================
 *  None
 *
 *  COMPONENTS:
 *  ===========
 *  - ESP32-S3-DevKitC-1 v1.1
 *  - USB-C cable
 *  - Wi-Fi access point
 *
 *  GPIO ASSIGNMENTS:
 *  =================
 *  None used
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

const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";

const char* server = "txplore-downloads.s3.amazonaws.com";
const char* url = "/esp32/test_file.txt";

const char* root_ca = R"rawliteral(
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

void setup() {
    Serial.begin(115200);
    delay(100);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.print(".");
    }

    WiFiClientSecure client;
    client.setCACert(root_ca);

    if (client.connect(server, 443)) {
        client.println(String("GET ") + url + " HTTP/1.1");
        client.println(String("Host: ") + server);
        client.println("Connection: close");
        client.println();

        while (client.connected()) {
            String line = client.readStringUntil('\n');
            if (line == "\r") break;
        }

        while (client.available()) {
            Serial.write(client.read());
        }

        client.stop();
    } else {
        Serial.println("Connection failed");
    }
}

void loop() {
    delay(10000);
}
