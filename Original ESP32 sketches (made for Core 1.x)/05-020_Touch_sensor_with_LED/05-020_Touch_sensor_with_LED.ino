/*  05.020 - Touch sensor with LED
 * 
 * This sketch shows you how to control an led using the integrated touch sensor.
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

void setup()
{
  pinMode(LED_GPIO, OUTPUT);
  Serial.begin(115200);
  delay(1000); // give me time to bring up serial monitor
  Serial.println("ESP32 Touch Test");
}

void loop()
{
  Serial.println(touchRead(T4));  // get value using T4
  
  if (touchRead(T4) < 20)         // The value 20 is determined experimentally
    digitalWrite(LED_GPIO, HIGH);
  else
    digitalWrite(LED_GPIO, LOW);
  
  delay(500);
}
