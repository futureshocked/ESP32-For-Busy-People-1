/*  07.023 - Dual Servo Motor Control for ESP32-S3
 * 
 * Course: IoT Development with ESP32-S3
 * Target Hardware: ESP32-S3-DevKitC-1 v1.1
 * Module: ESP32-S3-WROOM-1-N16R16V
 * 
 * This sketch demonstrates how to control two servo motors using the updated
 * ESP32-S3 platform and the modern Servo library. The motors are swept across
 * a full 180-degree range in both directions using the updated API and GPIOs.
 * 
 * HARDWARE INFORMATION:
 * =====================
 * Board: ESP32-S3-DevKitC-1 v1.1
 * USB-C power with native USB support
 * Flash: 16 MB, PSRAM: 16 MB
 * GPIOs: Use GPIO1–21 for general I/O. Avoid GPIO35–37 (reserved).
 * 
 * CIRCUIT DIAGRAM:
 * ================
 * 
 *        +-------------------+          +-------------------+
 *        |    ESP32-S3 Dev   |          |   Servo Motor 1   |
 *        |    GPIO10 (PWM1)  |----------| Orange (signal)   |
 *        |    GND            |----------| Brown (GND)       |
 *        |    5V             |----------| Red (V+)          |
 *        +-------------------+          
 *                                         +-------------------+
 *        |    GPIO11 (PWM2)  |----------| Orange (signal)   |
 *                                 Servo Motor 2             |
 *                                      GND / 5V as above
 * 
 * COMPONENTS:
 * ===========
 * - ESP32-S3-DevKitC-1 v1.1
 * - 2x Hobby servo motors (e.g., SG90)
 * - External 5V power supply (recommended for servo power)
 * - Breadboard and jumper wires
 * 
 * GPIO ASSIGNMENTS:
 * =================
 * - Servo 1 Signal: GPIO10
 * - Servo 2 Signal: GPIO11
 * 
 * TECHNICAL NOTES:
 * ================
 * - Use an external 5V source capable of supplying at least 1A for both servos.
 * - The ESP32-S3 pins can only supply a limited amount of current—never power
 *   motors directly from ESP32 5V rail.
 * - Use common GND between ESP32 and external servo power.
 * 
 * IDE REQUIREMENTS:
 * =================
 * - Arduino IDE 2.0+
 * - ESP32 Arduino Core v3.0+
 * - Board: "ESP32S3 Dev Module"
 * - Library: ESP32-Arduino-Servo-Library
 *   https://github.com/RoboticsBrno/ESP32-Arduino-Servo-Library
 * 
 * Created: April 1, 2019 by Peter Dalmaris
 * Updated: July 28, 2025 for ESP32-S3 compatibility
 */

#include <ESP32Servo.h>

// ===== Configuration Constants =====
constexpr int SERVO1_PIN = 10;
constexpr int SERVO2_PIN = 11;
constexpr int SWEEP_DELAY_MS = 20;

// ===== Global Objects =====
Servo servo1;
Servo servo2;

// ====== Initialization =====
void setup() {
    Serial.begin(115200);
    Serial.println("Dual Servo Motor Control - ESP32-S3");

    // Attach servos to GPIO pins
    if (!servo1.attach(SERVO1_PIN)) {
        Serial.println("Error: Failed to attach Servo 1");
    }
    if (!servo2.attach(SERVO2_PIN)) {
        Serial.println("Error: Failed to attach Servo 2");
    }
}

// ====== Main Loop =====
void loop() {
    sweepServos(0, 180, SWEEP_DELAY_MS);     // Forward sweep
    sweepServos(180, 0, SWEEP_DELAY_MS);     // Backward sweep
}

// ====== Helper Function =====
void sweepServos(int fromDeg, int toDeg, int delayMs) {
    int step = (fromDeg < toDeg) ? 1 : -1;
    for (int pos = fromDeg; pos != toDeg + step; pos += step) {
        servo1.write(pos);
        servo2.write(pos);
        Serial.printf("Angle: %d°\n", pos);
        delay(delayMs);
    }
}
