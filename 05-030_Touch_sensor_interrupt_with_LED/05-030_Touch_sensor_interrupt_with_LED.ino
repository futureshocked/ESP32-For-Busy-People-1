/*  05.030 - Touch sensor with interrupt and LED
 * 
 * This sketch shows you how to control an led using the integrated touch sensor.
 * 
 * Instead of reading the touch sensor in the loop, in this example
 * we will use an interrupt.
 * 
 * When you touch GPIO13 (TOUCH 4), the LED turns on.
 * 
 * 
 * This sketch was written by Peter Dalmaris using information from the 
 * ESP32 datasheet and examples.
 * 
 * 
 * Components
 * ----------
 *  - ESP32 Dev Kit v4
 *  - LED
 *  - 320 Ohm resistor
 *  - Jumper wire attached to the touch sensor pin
 *  
 *  IDE
 *  ---
 *  Arduino IDE with ESP32 Arduino Code 
 *  (https://github.com/espressif/arduino-esp32)
 *  
 *  
 *  Libraries
 *  ---------
 *  - None
 *  - 
 *
 * Connections
 * -----------
 *  
 *  ESP32 Dev Kit |     LED
 *  ------------------------------
 *        GND      |    Cathode
 *        GPIO32   |    Anode via resistor
 *        
 *  Connect a jumper wire to GPIO 13.
 *  
 *  Other information
 *  -----------------
 *  
 *  1. ESP32 Datasheet: https://www.espressif.com/sites/default/files/documentation/esp32_datasheet_en.pdf
 *  2. esp32-hal-touch.h: https://github.com/espressif/arduino-esp32/blob/master/cores/esp32/esp32-hal-touch.h
 *  
 *  Created on March 27 2019 by Peter Dalmaris
 * 
 */

const byte LED_GPIO = 32;  // Marked volatile so it can be read inside the ISR
int threshold = 20;  // This threshhold is determined experimentally. If the touch 
                      // sensor returns a value below this number, the interrupt is triggered.
bool touch4detected = false; // Use this variable to communicate between the loop and the interrupt
                              // routine.

void gotTouch(){
 touch4detected = true;
}

void setup()
{
  pinMode(LED_GPIO, OUTPUT);
  Serial.begin(115200);
  delay(1000); // give me time to bring up serial monitor
  Serial.println("ESP32 Touch Test with interrupt");
  touchAttachInterrupt(T4, gotTouch, threshold); // Attach the interrupt pin T0 to the service routine
}

void loop()
{ 
  if (touch4detected)         // The value 20 is determined experimentally
  {
    digitalWrite(LED_GPIO, HIGH);
    Serial.println("Touch detected");
    touch4detected = false;
    delay(500);
    digitalWrite(LED_GPIO, LOW);
  }
}
