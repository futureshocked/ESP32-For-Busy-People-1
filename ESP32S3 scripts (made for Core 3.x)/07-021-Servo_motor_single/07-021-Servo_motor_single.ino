/*  07.021 - Servo Motor Control Using ESP32Servo Library for ESP32-S3
 * 
 * Course: IoT Development with ESP32-S3
 * Target Hardware: ESP32-S3-DevKitC-1 v1.1
 * Module: ESP32-S3-WROOM-1-N16R16V
 * 
 * This sketch uses the `ESP32Servo` library to control a hobby servo motor.
 * The servo sweeps from 0° to 180° and back continuously.
 * 
 * HARDWARE INFORMATION:
 * =====================
 * - Board: ESP32-S3-DevKitC-1 v1.1
 * - GPIO10 used for servo PWM signal
 * - Use external 5V source for servo power (not USB power)
 * 
 * CIRCUIT DIAGRAM:
 * ================
 * 
 *     +----------------------------+
 *     |     ESP32-S3 Dev Board    |
 *     |                            |
 *     |    GPIO10  -------------> Orange (Signal) Servo
 *     |    GND     -------------> Brown  (GND)    Servo
 *     |    5V ext. -------------> Red    (Power)  Servo
 *     +----------------------------+
 * 
 * COMPONENTS:
 * ===========
 * - ESP32-S3-DevKitC-1 v1.1
 * - SG90 (or similar) hobby servo
 * - 5V external power source
 * - Jumper wires
 * 
 * GPIO ASSIGNMENTS:
 * =================
 * - SERVO_PIN: GPIO10
 * 
 * IDE REQUIREMENTS:
 * =================
 * - Arduino IDE 2.0+
 * - ESP32 Arduino Core v3.3.0+
 * - Library: ESP32Servo v1.1.1 (https://github.com/RoboticsBrno/ESP32-Arduino-Servo-Library)
 * - Board: "ESP32S3 Dev Module"
 * 
 * Created: April 1, 2019 by Peter Dalmaris
 * Updated: July 28, 2025 for ESP32-S3 compatibility
 */

#include <ESP32Servo.h>

// === Configuration ===
const int SERVO_PIN = 10;
const int MIN_ANGLE = 0;
const int MAX_ANGLE = 180;
const int STEP_DELAY_MS = 20;

// === Global Servo Object ===
Servo myServo;

void setup() {
    Serial.begin(115200);
    delay(200);

    // Attach servo to GPIO10
    if (!myServo.attach(SERVO_PIN)) {
        Serial.println("ERROR: Servo attach failed. Check GPIO pin and wiring.");
        while (true);  // Halt if servo init fails
    }

    Serial.println("Servo attached. Starting sweep.");
}

void loop() {
    // Sweep from 0° to 180°
    for (int angle = MIN_ANGLE; angle <= MAX_ANGLE; angle++) {
        myServo.write(angle);
        Serial.print("Angle: ");
        Serial.println(angle);
        delay(STEP_DELAY_MS);
    }

    // Sweep from 180° to 0°
    for (int angle = MAX_ANGLE; angle >= MIN_ANGLE; angle--) {
        myServo.write(angle);
        Serial.print("Angle: ");
        Serial.println(angle);
        delay(STEP_DELAY_MS);
    }
}
