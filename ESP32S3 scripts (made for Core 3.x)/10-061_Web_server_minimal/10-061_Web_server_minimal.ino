/*  10.061 – WiFi Web Server with BME280 & LED Control for ESP32-S3
 *
 * Course: IoT Development with ESP32‑S3
 * Target Hardware: ESP32‑S3‑DevKitC‑1 v1.1
 * Module: ESP32‑S3‑WROOM‑1‑N16R16V
 *
 * This sketch sets up a web server to show real-time temperature/humidity
 * from a BME280 sensor, and lets you click links to turn an LED on/off.
 * It demonstrates modern ESP32‑S3 APIs, ADC configuration, LEDC PWM,
 * and optional RGB LED feedback via WS2812.
 *
 * HARDWARE INFORMATION:
 * =====================
 * - ESP32‑S3‑WROOM‑1‑N16R16V (16 MB flash, 16 MB PSRAM)
 * - I²C‑connected BME280 (temperature, humidity, pressure)
 * - WS2812 RGB LED on GPIO38 (v1.1 board)
 * - External LED on GPIO2 (optional, used for visual feedback)
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
 * - WS2812 RGB LED (optional)
 * - LED + resistor (for GPIO2 control)
 *
 * GPIO ASSIGNMENTS:
 * =================
 * | Function        | GPIO Used  |
 * |-----------------|------------|
 * | I²C SDA         | 21         |
 * | I²C SCL         | 22         |
 * | LED control pin | 2          |
 * | WS2812 LED Din  | 38         |
 *
 * TECHNICAL NOTES:
 * ================
 * - Uses LEDC PWM for LED brightness
 * - Uses lambda functions for server route handlers
 * - Uses HTTP 303 redirect to keep interface centralized at "/"
 * - Circuit uses standard I²C BME280 wiring
 *
 * IDE REQUIREMENTS:
 * =================
 * - Arduino IDE 2.0+
 * - ESP32 Arduino Core v3.0+
 * - Board: "ESP32S3 Dev Module"
 *
 * Created: April 8, 2019 by Peter Dalmaris
 * Updated: July 30, 2025 for ESP32‑S3 compatibility
 */

#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <Adafruit_NeoPixel.h>

// ==== Configuration constants ====
constexpr const char* SSID = "__SSID__";
constexpr const char* PASSWORD = "__SSID_password__";
constexpr uint16_t SERVER_PORT = 80;
constexpr uint8_t LED_PIN = 2;         // External LED
constexpr uint8_t LEDC_RESOLUTION = 8; // 8-bit (0–255)
constexpr uint32_t LEDC_FREQUENCY = 5000;
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

    // Set ADC resolution to 12-bit (0–4095)
    analogReadResolution(12);

    // Configure LEDC PWM for external LED
    ledcAttach(LED_PIN, LEDC_FREQUENCY, LEDC_RESOLUTION);
    ledcWrite(LED_PIN, 0);  // Start OFF
    pinMode(LED_PIN, OUTPUT);

    // Initialize WS2812 (optional RGB LED)
    strip.begin();
    strip.show();

    // Connect to WiFi
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

    // Register web server route handlers
    // Each route uses a lambda function: an inline, anonymous function
    // that captures variables from the surrounding scope and executes
    // response logic when the client accesses a specific URL path.
    server.on("/", HTTP_GET, handleRoot);

    server.on("/H", HTTP_GET, []() {
        digitalWrite(LED_PIN, HIGH);
        ledcWrite(LED_PIN, 255);
        strip.setPixelColor(0, strip.Color(0, 150, 0));
        strip.show();

        // Instead of serving a new HTML page from this route, we:
        // 1. Turn the LED on or off (depending on the route logic)
        // 2. Send an HTTP 303 redirect response, which tells the browser:
        //    "Action completed, now go load the main page at '/'"
        // This keeps the user interface consistent and centralizes all page content
        // in the root ("/") handler, which shows the current sensor values and LED status.
        server.sendHeader("Location", "/");
        server.send(303);
    });

    server.on("/L", HTTP_GET, []() {
        digitalWrite(LED_PIN, LOW);
        ledcWrite(LED_PIN, 0);
        strip.setPixelColor(0, strip.Color(150, 0, 0));
        strip.show();

        // See explanation above – this route also redirects to "/"
        server.sendHeader("Location", "/");
        server.send(303);
    });

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
