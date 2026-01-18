/*  07-022 - Dual servo motor control with the ESP32
 * 
 * This sketch shows you how to control two servo motors using an ESP32.
 * breakout.
 * 
 * The sketch rotates the motors in a full 180 degree range.
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
 * ------------------------
 *   GND     |   (1) Brown
 *   GPIO12  |   (1) Orange
 *   5V      |   (1) Red
 *   GND     |   (2) Brown
 *   GPIO14  |   (2) Orange
 *   5V      |   (2) Red
 *     
 * More information:
 *
 * 1. Library: https://github.com/RoboticsBrno/ESP32-Arduino-Servo-Library
 *  
 *  Created on April 1 2019 by Peter Dalmaris
 * 
 */

#include <Servo.h>

static const int servoPin1 = 12;
static const int servoPin2 = 14;

Servo servo1;
Servo servo2;

void setup() {
    Serial.begin(115200);
    servo1.attach(servoPin1);
    servo2.attach(servoPin2);
}

void loop() {
    for(int posDegrees = 0; posDegrees <= 180; posDegrees++) {
        servo1.write(posDegrees);
        servo2.write(posDegrees);
        Serial.println(posDegrees);
        delay(20);
    }

    for(int posDegrees = 180; posDegrees >= 0; posDegrees--) {
        servo1.write(posDegrees);
        servo2.write(posDegrees);
        Serial.println(posDegrees);
        delay(20);
    }
}
