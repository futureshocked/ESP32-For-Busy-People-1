/*  05.010 - Integrated Hall effect (use a magnet)
 * 
 * This sketch shows you how to use the Hall efect sensor that is 
 * integrated in the ESP32.
 * 
 * No external components are necessary.
 * 
 * Run the sketch and place a magnet on the ESP32. Start the serial monitor.
 * Stronger magnect will produce higher values.
 * 
 * This sketch was written by Peter Dalmaris using information from the 
 * ESP32 datasheet and examples.
 * 
 * 
 * Components
 * ----------
 *  - ESP32 Dev Kit v4
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
 *  No external components needed.
 *  
 *  Other information
 *  -----------------
 *  
 *  1. ESP32 Datasheet: https://www.espressif.com/sites/default/files/documentation/esp32_datasheet_en.pdf
 *  2. What is a Hall effect sensor? https://en.wikipedia.org/wiki/Hall_effect_sensor
 *  
 *  Created on March 26 2019 by Peter Dalmaris
 * 
 */

//Simple sketch to access the internal hall effect detector on the esp32.
//values can be quite low. 
//Brian Degger / @sctv  

int val = 0;
void setup() {
  Serial.begin(9600);
    }

void loop() {
  // put your main code here, to run repeatedly:
  val = hallRead();
  // print the results to the serial monitor:
  //Serial.print("sensor = ");
  Serial.println(val);//to graph 
}
