/*  06-022 - Single seven segment display using a library
 * 
 * This sketch shows you how to drive a single common cathode seven segment 
 * display using only the ESP32's digital pins.
 * 
 * The sketch displays numbers from 0 to 9, plus the dot point.
 * 
 * This sketch was written by Peter Dalmaris for ESP32 For Busy People based on the library example.
 * 
 * Components
 * ----------
 *  - ESP32
 *  - Common cathode seven segment display
 *  - 8 x 220 Ohm resistor for the 8 LED segments
 *  - Breadboard
 *  - Jumper wires
 *  
 *  Libraries
 *  ---------
 *  - uSevenSegmentLib
 *  - uTimerLib
 *
 * Connections
 * -----------
 *  Arduino Uno and seven segment display:
 *  
 *       ESP32    |   Seven Seg
 *  ------------------------------
 *        GPIO15  |  Common cathode
 *        GPIO33  |       A
 *        GPIO32  |       B
 *        GPIO19  |       C 
 *        GPIO18  |       D
 *        GPIO05  |       E
 *        GPIO27  |       F 
 *        GPIO14  |       G    
 *        GPIO04  |       DpDp
 *     
 *  Other information:
 *  ------------------
 *  
 *  1. uSevenSegmentLib library: https://github.com/Naguissa/uSevenSegmentLib
 *  2. uTimerLib library: https://github.com/Naguissa/uTimerLib
 * 
 *  
 *  Created on April 1 2019 by Peter Dalmaris
 * 
 */

#include "Arduino.h"
#include "uSevenSegmentLib.h"


int pins[8]  = {33,32,19,18,5,27,14,4};
int muxes[1] = {15};

uSevenSegmentLib sevenSegments(1, pins, muxes);

int i = 0;

void setup() {
	sevenSegments.set(i);
	sevenSegments.attachInterrupt();
}

void loop() {
	delay(1000);
  i = (i + 1) % 10;
	sevenSegments.set(i);
}
