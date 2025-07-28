/*  07.023 - Dual Servo Motor Control for ESP32-S3 (Minimal)
 * 
 * Course: IoT Development with ESP32-S3
 * Target Hardware: ESP32-S3-DevKitC-1 v1.1
 * Module: ESP32-S3-WROOM-1-N16R16V
 * 
 * This minimal sketch demonstrates how to control two hobby servo motors using
 * the ESP32-S3 and the Servo library. It sweeps both motors from 0° to 180° and back.
 * 
 * IDE REQUIREMENTS:
 * =================
 * - Arduino IDE 2.0+
 * - ESP32 Arduino Core v3.0+
 * - Board: "ESP32S3 Dev Module"
 * - Library: ESP32-Arduino-Servo-Library
 */

#include <ESP32Servo.h>

const int SERVO1_PIN = 10;
const int SERVO2_PIN = 11;

Servo servo1;
Servo servo2;

void setup() {
    Serial.begin(115200);
    servo1.attach(SERVO1_PIN);
    servo2.attach(SERVO2_PIN);
}

void loop() {
    for (int pos = 0; pos <= 180; pos++) {
        servo1.write(pos);
        servo2.write(pos);
        delay(20);
    }

    for (int pos = 180; pos >= 0; pos--) {
        servo1.write(pos);
        servo2.write(pos);
        delay(20);
    }
}
