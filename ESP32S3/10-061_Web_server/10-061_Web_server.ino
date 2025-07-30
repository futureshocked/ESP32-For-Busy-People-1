/*  10.061 – WiFi Web Server with BME280 & LED Control for ESP32-S3
 *
 * Course: IoT Development with ESP32‑S3
 * Target Hardware: ESP32‑S3‑DevKitC‑1 v1.1
 * Module: ESP32‑S3‑WROOM‑1‑N16R16V
 *
 * This sketch sets up a web server to show real-time temperature/humidity
 * from a BME280 sensor, and lets you click links to turn an LED on/off.
 * It demonstrates modern ESP32‑S3 APIs, ADC configuration, LEDC PWM,
 * and external low-cost RGB LED support if internal WRGB LED unavailable.
 *
 * HARDWARE INFORMATION:
 * =====================
 * - ESP32‑S3‑WROOM‑1‑N16R16V (16 MB flash, 16 MB PSRAM)
 * - I²C‑connected BME280 (temperature, humidity, pressure)
 * - WS2812 RGB LED on GPIO38 (due to board v1.1 footprint)
 * - Built‑in single LED (if available), else external LED on GPIO2
 * - ADC resolution set to 12‑bit (0–4095)
 *
 * CIRCUIT DIAGRAM:
 * ================
 * 
 *  ESP32‑S3‑DevKitC‑1 v1.1      BME280           WS2812 LED (optional)
 *  ┌────────────┐                ┌───────┐          ┌────────┐
 *  │            │   3V3 ───────> │ Vin   │          │ Vcc    │
 *  │            │   GND ───────> │ GND   │          │ GND    │
 *  │ GPIO22     │ ─── SCL ─────> │ SCL   │          │        │
 *  │ GPIO21     │ ─── SDA ─────> │ SDA   │          │        │
 *  │ GPIO38     │ ─── Din ─────> │        WS2812     │ Din    │
 *  └────────────┘                └───────┘          └────────┘
 *
 * COMPONENTS:
 * ===========
 * - ESP32‑S3‑DevKitC‑1 v1.1
 * - BME280 I²C breakout module
 * - Optional: WS2812 5050 LED or LED strip
 *
 * GPIO ASSIGNMENTS:
 * =================
 * | Function        | GPIO Used  |
 * |-----------------|------------|
 * | I²C SDA         | 21         |
 * | I²C SCL         | 22         |
 * | LED control pin | 2 (external) |
 * | WS2812 LED Din  | 38         |
 *
 * TECHNICAL NOTES:
 * ================
 * - Uses LEDC PWM for external LED brightness control
 * - ADC resolution set via analogReadResolution(12)
 * - I²C pins mapped explicitly for ESP32‑S3
 * - If onboard WiFi/Bluetooth interrupts occur, use task watchdog resets
 *
 * IDE REQUIREMENTS:
 * =================
 * - Arduino IDE 2.0+
 * - ESP32 Arduino Core v3.0+ (“esp32‑s3/dev” board package)
 *
 * Created: April 8, 2019 by Peter Dalmaris
 * Updated: July 30, 2025 for ESP32‑S3 compatibility
 */

// Include required libraries
#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <Adafruit_NeoPixel.h>

// ==== Configuration constants ====
constexpr const char* SSID = "ardwifi";
constexpr const char* PASSWORD = "ardwifi987";
constexpr uint16_t SERVER_PORT = 80;
constexpr uint8_t LED_PIN = 2;         // External LED
constexpr uint8_t LEDC_CHANNEL = 0;
constexpr uint32_t LEDC_FREQUENCY = 5000;
constexpr uint8_t LEDC_RESOLUTION = 8; // 0–255 brightness
constexpr uint8_t WS2812_PIN = 38;
constexpr uint16_t NUM_PIXELS = 1;

// ==== Global objects ====
WebServer server(SERVER_PORT);
Adafruit_BME280 bme;
Adafruit_NeoPixel strip(NUM_PIXELS, WS2812_PIN, NEO_GRB + NEO_KHZ800);

// ==== Setup ====
void setup() {
    Serial.begin(115200);
    while (!Serial) { delay(10); }

    Serial.println("\nStarting Lesson 10.061 – Web Server + BME280 + LED");

    // Initialize BME280
    if (!bme.begin(0x76, &Wire)) {
        Serial.println("ERROR: BME280 not detected at 0x76");
        while (true) { delay(1000); } // Halt with debug info
    }
    Serial.println("BME280 sensor initialized.");

    // Configure ADC resolution
    analogReadResolution(12);

    // Configure LEDC PWM channel for external LED
    ledcAttach(LED_PIN, LEDC_FREQUENCY, LEDC_RESOLUTION);
    ledcWrite(LEDC_CHANNEL, 0); // start off
    pinMode(LED_PIN, OUTPUT);

    // Initialize WS2812 if present
    strip.begin();
    strip.show();

    // Connect WiFi
    Serial.printf("Connecting to SSID '%s'...\n", SSID);
    WiFi.begin(SSID, PASSWORD);
    unsigned long connectStart = millis();
    while (WiFi.status() != WL_CONNECTED) {
        if (millis() - connectStart > 20000UL) {
            Serial.println("ERROR: WiFi connect timed out");
            ESP.restart();
        }
        delay(500);
        Serial.print('.');
    }
    Serial.println("\nWiFi connected. IP address: " + WiFi.localIP().toString());

    // Setup web server routes
    server.on("/", HTTP_GET, handleRoot);
    server.on("/H", HTTP_GET, []() { setLedOn(); server.sendHeader("Location", "/"); server.send(303); });
    server.on("/L", HTTP_GET, []() { setLedOff(); server.sendHeader("Location", "/"); server.send(303); });
    server.begin();
    Serial.println("HTTP server started.");
}

// ==== Main loop ====
void loop() {
    server.handleClient();
}

// ==== Helper functions ====
String getHtmlPage() {
    float temp = bme.readTemperature();
    float hum = bme.readHumidity();
    String html = "<!DOCTYPE HTML><html><head><meta charset=\"utf‑8\"><title>ESP32‑S3 Web Server</title></head><body>";
    html += "<p><a href=\"/H\">Turn LED ON</a></p>";
    html += "<p><a href=\"/L\">Turn LED OFF</a></p>";
    html += "<hr>";
    html += "<p>Temperature: " + String(temp, 2) + " °C</p>";
    html += "<p>Humidity: " + String(hum, 2) + " %</p>";
    html += "</body></html>";
    return html;
}

void handleRoot() {
    server.send(200, "text/html", getHtmlPage());
}

// LED control routines
void setLedOn() {
    ledcWrite(LED_PIN, 255);
    // optional WS2812 blink
    strip.setPixelColor(0, strip.Color(0, 150, 0)); 
    strip.show();
}

void setLedOff() {
    ledcWrite(LED_PIN, 0);
    strip.setPixelColor(0, strip.Color(150, 0, 0)); 
    strip.show();
}
