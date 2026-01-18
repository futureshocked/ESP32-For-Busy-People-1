/*  06.031 - Clock-Style 7-Segment Timer Using I2C Display (ESP32-S3)
 * 
 * Course: IoT Development with ESP32-S3
 * Target Hardware: ESP32-S3-DevKitC-1 v1.1
 * Module: ESP32-S3-WROOM-1-N16R16V
 * 
 * This sketch implements a simple MM:SS timer using an I2C 7-segment display.
 * The colon blinks every second. Useful for beginners learning I2C and display control.
 * 
 * COMPONENTS:
 * ===========
 * - ESP32-S3-DevKitC-1 v1.1
 * - HT16K33-based I2C 4-digit display
 * - Breadboard, jumper wires
 * 
 * GPIO ASSIGNMENTS:
 * =================
 * SDA: GPIO10
 * SCL: GPIO8
 * 
 * IDE REQUIREMENTS:
 * =================
 * - Arduino IDE 2.0+
 * - ESP32 Arduino Core v3.0+
 */

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_LEDBackpack.h>

Adafruit_7segment display;

constexpr uint8_t SDA_PIN = 10;
constexpr uint8_t SCL_PIN = 8;
constexpr uint8_t I2C_ADDRESS = 0x70;

unsigned long lastMillis = 0;
unsigned long secondsElapsed = 0;
bool colonState = false;

void setup() {
    Wire.begin(SDA_PIN, SCL_PIN);
    display.begin(I2C_ADDRESS);
}

void loop() {
    if (millis() - lastMillis >= 1000) {
        lastMillis = millis();
        secondsElapsed++;
        colonState = !colonState;

        int minutes = secondsElapsed / 60;
        int seconds = secondsElapsed % 60;
        display.print(minutes * 100 + seconds);

        if (minutes < 10) display.writeDigitNum(0, 0);
        if (minutes < 1) display.writeDigitNum(1, 0);
        if (seconds < 10) display.writeDigitNum(3, 0);

        display.drawColon(colonState);
        display.writeDisplay();
    }
}
