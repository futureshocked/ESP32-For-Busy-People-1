/*  10.051 - Dweet using HTTPS REST client for ESP32‑S3 (via dweet.cc)
 * 
 * Course: IoT Development with ESP32‑S3
 * Target Hardware: ESP32‑S3‑DevKitC‑1 v1.1
 * Module: ESP32‑S3‑WROOM‑1‑N16R16V
 * 
 * NOTE: The original dweet.io service was shut down in early 2025.
 *       dweet.cc now provides a drop‑in replacement with the same API,
 *       so changing your URLs from dweet.io to dweet.cc works seamlessly.
 *       All “dweet” operations function the same way under dweet.cc.  [oai_citation:0‡forums.raspberrypi.com](https://forums.raspberrypi.com/viewtopic.php?t=386762&utm_source=chatgpt.com)
 * 
 * This sketch reads temperature and humidity from a BME280 sensor
 * and sends them securely to dweet.cc using HTTPS.
 * Designed for clarity, debug output, error handling and robust WiFi use.
 * 
 * HARDWARE INFORMATION:
 * =====================
 * - ESP32‑S3‑DevKitC‑1 v1.1, 16 MB Flash, 16 MB PSRAM
 * - BME280 I2C sensor (temperature & humidity)
 * 
 * CIRCUIT DIAGRAM:
 * ================
 * ESP32‑S3        BME280
 * --------        ------
 * 3.3 V     <‑‑> VIN
 * GND       <‑‑> GND
 * GPIO10    <‑‑> SDA
 * GPIO9     <‑‑> SCL
 * 
 * COMPONENTS:
 * ===========
 * - ESP32‑S3‑DevKitC‑1 v1.1
 * - Adafruit BME280 sensor module
 * - Jumper wires, breadboard
 * 
 * GPIO ASSIGNMENTS:
 * =================
 * SDA → GPIO10
 * SCL → GPIO9
 * 
 * TECHNICAL NOTES:
 * ================
 * - I2C address typically 0x76 or 0x77
 * - Use WiFiClientSecure with valid CA certificate
 * - Use modern WiFi and HTTPS APIs
 * - Posting interval set to ≥ 15 s to respect dweet.cc rate limits
 * 
 * IDE REQUIREMENTS:
 * =================
 * - Arduino IDE 2.0+
 * - ESP32 Arduino Core v3.0+
 * - Board: “ESP32S3 Dev Module”
 * - Libraries:
 *    • WiFiClientSecure (built‑in)
 *    • Adafruit_Sensor
 *    • Adafruit_BME280
 * 
 * Created: April 8 2019 by Peter Dalmaris
 * Updated: July 30 2025 for ESP32‑S3 & dweet.cc compatibility
 */

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

constexpr const char* WIFI_SSID     = "yourSSID";
constexpr const char* WIFI_PASSWORD = "yourPassword";
constexpr const char* DWEET_HOST    = "dweet.cc";
constexpr int         DWEET_PORT    = 443;
constexpr const char* THING_NAME    = "peterslab";  // customize your thing
constexpr const char* DWEET_CA_CERT = R"RAW(
-----BEGIN CERTIFICATE-----
... same CA certificate as before ...
-----END CERTIFICATE-----
)RAW";

Adafruit_BME280 bme;
WiFiClientSecure client;

void setup() {
    Serial.begin(115200);
    delay(100);
    Serial.println("\n--- ESP32‑S3 -> dweet.cc HTTPS Client ---");

    if (!bme.begin(0x76)) {
        Serial.println("Error: BME280 not detected. Check wiring/I2C address.");
        while (true);
    }

    connectToWiFi();
    client.setCACert(DWEET_CA_CERT);
}

void loop() {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi lost. Reconnecting...");
        connectToWiFi();
    }

    sendSensorData();
    delay(20000);  // minimum interval to avoid rate limiting
}

void connectToWiFi() {
    Serial.printf("Connecting to WiFi: %s\n", WIFI_SSID);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < 10000) {
        Serial.print(".");
        delay(500);
    }
    if (WiFi.status() == WL_CONNECTED) {
        Serial.printf("\nWiFi connected, IP: %s\n", WiFi.localIP().toString().c_str());
    } else {
        Serial.println("\nFailed to connect to WiFi.");
    }
}

void sendSensorData() {
    float temperature = bme.readTemperature();
    float humidity = bme.readHumidity();

    String url = "/dweet/for/" + String(THING_NAME) +
                 "?temp=" + String(temperature, 2) +
                 "&hum=" + String(humidity, 2);

    Serial.println("HTTPS GET: " + url);

    if (!client.connect(DWEET_HOST, DWEET_PORT)) {
        Serial.println("Connection to dweet.cc failed.");
        return;
    }

    client.println("GET " + url + " HTTP/1.1");
    client.println("Host: " + String(DWEET_HOST));
    client.println("Connection: close");
    client.println();

    while (client.connected()) {
        String line = client.readStringUntil('\n');
        if (line == "\r") {
            Serial.println("Header end reached.");
            break;
        }
    }

    while (client.available()) {
        Serial.write(client.read());
    }
    client.stop();
    Serial.println("\nData sent. Waiting...");
}
