/*  10.051 - Dweet using HTTPS REST client for ESP32‑S3 (via dweet.cc)
 *
 * NOTE: dweet.io shut down in early 2025. dweet.cc provides an API-compatible
 *       drop-in replacement—just replace .io with .cc in all URLs.  [oai_citation:1‡dweet.cc](https://dweet.cc/?utm_source=chatgpt.com) [oai_citation:2‡GoatLab](https://goatlab.wordpress.com/2018/01/10/talking-to-your-devices-with-dweet/?utm_source=chatgpt.com)
 *
 * Minimal example: reads BME280 and sends temperature & humidity to dweet.cc
 * using HTTPS GET. No retry logic or advanced error handling.
 *
 * Created: April 8 2019 by Peter Dalmaris
 * Updated: July 30 2025 for ESP32‑S3 & dweet.cc
 */

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <Adafruit_BME280.h>

constexpr const char* WIFI_SSID     = "yourSSID";
constexpr const char* WIFI_PASSWORD = "yourPassword";
constexpr const char* DWEET_HOST    = "dweet.cc";
constexpr int         DWEET_PORT    = 443;
constexpr const char* THING_NAME    = "peterslab";
constexpr const char* DWEET_CA_CERT = R"RAW(
-----BEGIN CERTIFICATE-----
... same CA certificate ...
-----END CERTIFICATE-----
)RAW";

Adafruit_BME280 bme;
WiFiClientSecure client;

void setup() {
    Serial.begin(115200);
    bme.begin(0x76);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) delay(500);
    client.setCACert(DWEET_CA_CERT);
}

void loop() {
    float t = bme.readTemperature();
    float h = bme.readHumidity();
    String u = "/dweet/for/" + String(THING_NAME) +
               "?temp=" + String(t,1) + "&hum=" + String(h,1);

    if (client.connect(DWEET_HOST, DWEET_PORT)) {
        client.println("GET " + u + " HTTP/1.1");
        client.println("Host: " + String(DWEET_HOST));
        client.println("Connection: close");
        client.println();
        while (client.available()) Serial.write(client.read());
        client.stop();
    }
    delay(20000);
}
