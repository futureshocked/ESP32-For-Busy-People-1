/*  05.040 - Photoresistor
 * 
 * This sketch shows you how read ambient light intensity with a photoresistor.
 * 
 * After uploading the sketch to your ESP32, open the serial monitor to see the
 * value from the photoresistor.
 * 
 * 
 * This sketch was written by Peter Dalmaris using information from the 
 * ESP32 datasheet and examples.
 * 
 * 
 * Components
 * ----------
 *  - ESP32 Dev Kit v4
 *  - Photoresistor, 10 KOhm
 *  - 10 K Ohm resistor
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
 *  Connect the photoresistor in a voltage ladder configuration with the resistor.
 *  Pin 1 of the photoresistor to 3.3V
 *  Pin 2 of the photoresistor to resistor.
 *  Pin 2 of the photoresistor to GPIO 39
 *  Free pin of the resistor to GND
 *        
 *  
 *  Other information
 *  -----------------
 *  
 *  1. ESP32 Datasheet: https://www.espressif.com/sites/default/files/documentation/esp32_datasheet_en.pdf 
 *  2. Header file esp32-hal-adc: https://github.com/espressif/arduino-esp32/blob/master/cores/esp32/esp32-hal-adc.h
 *  
 *  Created on March 27 2019 by Peter Dalmaris
 *  
 */

const byte PHOTO_GPIO = 39;

void setup() {  
  Serial.begin(115200);
//  analogSetWidth(9);  // Default resolution is 12 bits. Choose between 9-12.
}

void loop() {
  Serial.println(analogRead(PHOTO_GPIO));
  delay(500);
}
