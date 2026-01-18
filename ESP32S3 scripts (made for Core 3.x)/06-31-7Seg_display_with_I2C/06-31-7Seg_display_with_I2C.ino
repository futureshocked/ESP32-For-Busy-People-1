/*  06.031 - Clock-Style 7-Segment Timer Using I2C Display (ESP32-S3)
 * 
 * Course: IoT Development with ESP32-S3
 * Target Hardware: ESP32-S3-DevKitC-1 v1.1
 * Module: ESP32-S3-WROOM-1-N16R16V
 * 
 * This sketch demonstrates how to use an Adafruit HT16K33-based 4-digit 
 * 7-segment display to implement a digital timer. It displays elapsed time 
 * in MM:SS format, updating every second. The colon blinks each second for clarity.
 * 
 * HARDWARE INFORMATION:
 * =====================
 * - ESP32-S3-DevKitC-1 v1.1
 * - I2C-compatible 7-segment display with HT16K33 driver (Adafruit LED Backpack)
 * - 16 MB Flash, 16 MB PSRAM
 * - 12-bit ADC, LEDC PWM, Native USB
 * 
 * CIRCUIT DIAGRAM:
 * ================
 * 
 *     ESP32-S3-DevKitC-1 v1.1       HT16K33 Display
 *     ------------------------     -----------------
 *          3V3                    →     VCC
 *          GND                    →     GND
 *         GPIO10 (SDA)           →     SDA
 *         GPIO8  (SCL)           →     SCL
 * 
 * COMPONENTS:
 * ===========
 * - ESP32-S3-DevKitC-1 v1.1
 * - HT16K33-based I2C 4-digit 7-segment display
 * - Breadboard and jumper wires
 * 
 * GPIO ASSIGNMENTS:
 * =================
 * SDA: GPIO10 (ADC1_10, I2C-capable)
 * SCL: GPIO8  (ADC1_8, I2C-capable)
 * 
 * TECHNICAL NOTES:
 * ================
 * - Use 3.3V for power (not 5V) to avoid damaging the ESP32-S3
 * - I2C default address for HT16K33 is 0x70
 * - Colon is toggled every second using a blink flag
 * - Timer uses millis() with 1 second interval
 * 
 * IDE REQUIREMENTS:
 * =================
 * - Arduino IDE 2.0+
 * - ESP32 Arduino Core v3.0+
 * - Board: "ESP32S3 Dev Module"
 * - Libraries:
 *     - Adafruit_GFX (latest)
 *     - Adafruit_LEDBackpack (latest)
 *     - Wire (included)
 * 
 * Created: April 1, 2019 by Peter Dalmaris
 * Updated: July 28, 2025 for ESP32-S3 compatibility
 */

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_LEDBackpack.h>

// Constants
constexpr uint8_t I2C_SDA_PIN = 10;
constexpr uint8_t I2C_SCL_PIN = 8;
constexpr uint8_t DISPLAY_I2C_ADDRESS = 0x70;
constexpr unsigned long INTERVAL_MS = 1000;
constexpr unsigned long SECS_PER_MIN = 60UL;

// Globals
Adafruit_7segment clockDisplay;
unsigned long lastUpdateTime = 0;
unsigned long elapsedSeconds = 0;
bool blinkColon = false;

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("Starting 7-segment timer...");

    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
    if (!clockDisplay.begin(DISPLAY_I2C_ADDRESS)) {
        Serial.println("Error: Could not initialize 7-segment display!");
        while (true); // Halt execution
    }

    clockDisplay.clear();
    clockDisplay.writeDisplay();
}

void loop() {
    unsigned long currentMillis = millis();
    if (currentMillis - lastUpdateTime >= INTERVAL_MS) {
        lastUpdateTime = currentMillis;
        elapsedSeconds++;
        blinkColon = !blinkColon;
        displayTime(elapsedSeconds, blinkColon);
    }
}

void displayTime(unsigned long totalSeconds, bool showColon) {
    uint8_t minutes = (totalSeconds / 60) % 60;
    uint8_t seconds = totalSeconds % 60;

    int displayValue = minutes * 100 + seconds;
    clockDisplay.print(displayValue);

    // Padding for display clarity
    if (minutes < 10) clockDisplay.writeDigitNum(0, 0);
    if (minutes < 1)  clockDisplay.writeDigitNum(1, 0);
    if (seconds < 10) clockDisplay.writeDigitNum(3, 0);

    clockDisplay.drawColon(showColon);
    clockDisplay.writeDisplay();

    Serial.printf("Time: %02d:%02d\n", minutes, seconds);
}
