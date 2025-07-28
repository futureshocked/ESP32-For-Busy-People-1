/*  07.021 - Minimal Servo Control Using ESP32Servo
 * 
 * Course: IoT Development with ESP32-S3
 * Target Hardware: ESP32-S3-DevKitC-1 v1.1
 * Module: ESP32-S3-WROOM-1-N16R16V
 * 
 * This sketch demonstrates basic servo control using the ESP32Servo library.
 * A hobby servo is connected to GPIO10 and sweeps between 0° and 180°.
 * 
 * HARDWARE INFORMATION:
 * =====================
 * - Board: ESP32-S3-DevKitC-1 v1.1
 * - GPIO voltage: 3.3V
 * - Servo signal pin: GPIO10
 * - External 5V recommended for servo power
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
 * - External 5V power source
 * - Jumper wires
 * 
 * GPIO ASSIGNMENTS:
 * =================
 * - SERVO_PIN: GPIO10
 * 
 * TECHNICAL NOTES:
 * ================
 * - GPIO10 is PWM-capable and safe on ESP32-S3
 * - The ESP32Servo library internally uses the LEDC peripheral
 * - Avoid powering servo directly from USB port
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

const int SERVO_PIN = 10;
Servo servo1;

void setup() {
    Serial.begin(115200);
    servo1.attach(SERVO_PIN);
}

void loop() {
    for (int angle = 0; angle <= 180; angle++) {
        servo1.write(angle);
        delay(20);
    }

    for (int angle = 180; angle >= 0; angle--) {
        servo1.write(angle);
        delay(20);
    }
}
