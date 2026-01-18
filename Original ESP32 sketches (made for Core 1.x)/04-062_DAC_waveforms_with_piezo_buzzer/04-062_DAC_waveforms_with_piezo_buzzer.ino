/*  04.062 - Digital to Analog (DAC) waveforms with piezo buzzer
 * 
 * This sketch shows you how to create sounds from various waveforms and a piezo buzzer.
 * 
 * Connect your buzzer to GPIO 25 (DAC 1).
 * 
 * Uncomment the waveform you want to listen to.
 * 
 * Run the sketch.
 * 
 * Listen to the waveform as it is converted to sound by the buzzer.
 * 
 * This sketch was written by Peter Dalmaris using information from the 
 * ESP32 datasheet and examples.
 * 
 * 
 * Components
 * ----------
 *  - ESP32 Dev Kit v4
 *  - Piezo buzzer
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
 *  Buzzer: Connect on pin to DAC 1 and the other to GND.
 *  
 *  Other information
 *  -----------------
 *  
 *  1. ESP32 Datasheet: https://www.espressif.com/sites/default/files/documentation/esp32_datasheet_en.pdf
 *  2. ESP32 has two 8-bit DAC (digital to analog converter) channels, connected to GPIO25 (Channel 1) and GPIO26 (Channel 2)
 *  3. ESP32 Arduino Core header file: https://github.com/espressif/arduino-esp32/blob/master/cores/esp32/esp32-hal-dac.h
 *  
 *  Created on March 26 2019 by Peter Dalmaris
 * 
 */

void setup() {
  
}

void loop() { 

  // this will create a triangle waveform
  for (int value = 0; value < 255; value=value+5)
  {
    dacWrite(25,value);
  }

  for (int value = 255; value > -1; value=value-5)
  {
    dacWrite(25,value);
  }

  // Uncomment one of the following
 // for (int deg = 0; deg < 360; deg++){
 //    dacWrite(25, int(128 + 80 * (sin(deg*PI/180)+sin(3*deg*PI/180)/3+sin(5*deg*PI/180)/5+sin(7*deg*PI/180)/7+sin(9*deg*PI/180)/9+sin(11*deg*PI/180)/11))); // Square
 //    dacWrite(25, int(128 + 80 * (sin(deg*PI/180)+1/pow(3,2)*sin(3*deg*PI/180)+1/pow(5,2)*sin(5*deg*PI/180)+1/pow(7,2)*sin(7*deg*PI/180)+1/pow(9,2)*sin(9*deg*PI/180))));  // Triangle
 //    dacWrite(25, int(128 + 80 * (sin(deg*PI/180)))); // Sine wave
 // }
}
