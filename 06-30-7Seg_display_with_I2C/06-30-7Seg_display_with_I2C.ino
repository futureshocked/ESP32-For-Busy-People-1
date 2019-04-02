/*  06.030 - Clock seven segment display with I2C
 * 
 * This sketch shows you how to use a clock seven segment display to
 * implement a simple timer. The display is used to display the 
 * amount of time in minutes and seconds that has elapsed since the
 * sketch started running on the Arduino.
 * 
 * 
 * This sketch was written by Peter Dalmaris for ESP32 For Busy People.
 * 
 * Components
 * ----------
 *  - ESP32
 *  - Adafruit seven segment clock display I2C backpack HT16K33
 *  - Breadboard
 *  - Jumper wires
 *  
 *  Libraries
 *  ---------
 *  - Wire
 *  - Adafruit_GFX
 *  - Adafruit_LEDBackpack
 *
 * Connections
 * -----------
 *  
 *      ESP32     |    7-Segment clock display
 *  -----------------------------
 *      GND       |      GND
 *      3.3V      |      VCC 
 *      GPIO42    |      SDA
 *      GPIO39    |      SCL
 * 
 * Other information:
 * 
 * 1. Adafruit LED backpack library: https://github.com/adafruit/Adafruit_LED_Backpack
 * 2. Adafruit GFX library: https://github.com/adafruit/Adafruit-GFX-Library 
 *  
 *  Created on April 1 2019 by Peter Dalmaris
 * 
 */

#include <Wire.h>
#include <Adafruit_GFX.h>
#include "Adafruit_LEDBackpack.h"

unsigned long timer = 0;

long previousMillis = 0;        // will store last time LED was updated
 
// the follow variables is a long because the time, measured in miliseconds,
// will quickly become a bigger number than can be stored in an int.
long interval = 1000;           // interval at which to blink (milliseconds)

/* Useful Constants */
#define SECS_PER_MIN  (60UL)
#define SECS_PER_HOUR (3600UL)

/* Useful Macros for getting elapsed time */
#define numberOfSeconds(_time_) (_time_ % SECS_PER_MIN)  
#define numberOfMinutes(_time_) ((_time_ / SECS_PER_MIN) % SECS_PER_MIN) 

// I2C address of the display.  Stick with the default address of 0x70
// unless you've changed the address jumpers on the back of the display.
#define DISPLAY_ADDRESS   0x70


// Create display object.  These are global variables that
// can be accessed from both the setup and loop function below.
Adafruit_7segment clockDisplay = Adafruit_7segment();

// Remember if the colon was drawn on the display so it can be blinked
// on and off every second.
bool blinkColon = false;


void setup() {
  // Setup the display.
  clockDisplay.begin(DISPLAY_ADDRESS);
}

void loop() {
  unsigned long currentMillis = millis();
  if(currentMillis - previousMillis > interval) {
    // save the last time the timer and blinkColon variables were updated
    previousMillis = currentMillis;   
    timer++;
    blinkColon = !blinkColon;      // Flip the value of the blinkColon variable every second, so that
                                   // the colon blinks accordingly
  }

  printTimer();                   // uncoment this line to actually call the function that 
                                    // displays the timer.

// Use the following examples to learn how to control individual digits on the display.
//clockDisplay.writeDigitNum(0, 0);   // Write a "0" to digit 0
//clockDisplay.writeDigitNum(1, 1);   // Write a "1" to digit 1
//clockDisplay.writeDigitNum(2, 1);   //since position 2 is the colon, use the drawColon function to control it
//clockDisplay.drawColon(1);          // Turn on the colon
//clockDisplay.writeDigitNum(3, 3);   // Write a "3" to digit 3
//clockDisplay.writeDigitNum(4, 4);   // Write a "4" to digit 4
//clockDisplay.writeDisplay();        // Update the display with these changes
}
 
void printTimer()
{
  int minutes = numberOfMinutes(timer);
  int seconds = numberOfSeconds(timer);
  clockDisplay.print(minutes*100 + seconds, DEC);

  //Cater for missing zeros if elapsed time is less than 10 seconds,
  //less than 1 minute, or less than 10 minutes.
  if (minutes < 10) 
      clockDisplay.writeDigitNum(0, 0);
  if (minutes < 1) 
      clockDisplay.writeDigitNum(1, 0);
  if (seconds < 10) 
      clockDisplay.writeDigitNum(3, 0);
  
  // Blink the colon by writting the current blinkColon value
  clockDisplay.drawColon(blinkColon);

  clockDisplay.writeDisplay();
}
