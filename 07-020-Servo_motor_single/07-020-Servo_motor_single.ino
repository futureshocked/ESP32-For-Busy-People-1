/*  07-020 - Simple servo motor control with the ESP32
 * 
 * This sketch shows you how to control a small servo motor using an ESP32.
 * breakout.
 * 
 * The sketch rotates the motor in a full 180 degree range.
 * 
 * 
 * Components
 * ----------
 *  - ESP32
 *  - a small hobby servo motor
 *  
 *  Libraries
 *  ---------
 *  - Servo (for the ESP32)
 *
 * Connections
 * -----------
 *  
 *   ESP32   |   Servo motor
 * ---------------------
 *   GND     |   Brown
 *   GPI12   |   Orange
 *   5V      |   Red
 *     
 * More information:
 *
 * 1. Library: https://github.com/RoboticsBrno/ESP32-Arduino-Servo-Library
 * Please use version 1.0.3 of this library. Tested to work with Arduino IDE v2.2.1.
 *  
 *  Created on April 1 2019 by Peter Dalmaris
 * 
 */

#include <Servo.h>

static const int servoPin = 12;

Servo servo1;

void setup() {
    Serial.begin(115200);
    servo1.attach(servoPin);
}

void loop() {
    for(int posDegrees = 0; posDegrees <= 180; posDegrees++) {
        servo1.write(posDegrees);
        Serial.println(posDegrees);
        delay(20);
    }

    for(int posDegrees = 180; posDegrees >= 0; posDegrees--) {
        servo1.write(posDegrees);
        Serial.println(posDegrees);
        delay(20);
    }
}
