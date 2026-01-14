/*  04.020 - ESP32 Digital out PWM LED example (Updated)
 * 
 * This sketch shows you how to use the PWM output capability of the ESP32
 * using the modern LEDC API (ESP32 Arduino Core 3.0+).
 * 
 * This sketch was written by Peter Dalmaris using information from the 
 * ESP32 datasheet and examples.
 * Updated for ESP32 Arduino Core 3.x API.
 * 
 * Components
 * ----------
 *  - ESP32 Dev Kit v4
 *  - LED
 *  - 320 Ohm resistor
 *  
 *  IDE
 *  ---
 *  Arduino IDE with ESP32 Arduino Core 3.0 or later
 *  (https://github.com/espressif/arduino-esp32)
 *  
 *  Libraries
 *  ---------
 *  - None required
 *
 * Connections
 * -----------
 *  
 *  ESP32 Dev Kit |     LED
 *  ------------------------------
 *        GND      |     Cathode
 *        GPIO32   |     Anode via resistor
 *     
 *  It is possible to use any other PWM-capable GPIO.
 *  
 *  Other information
 *  -----------------
 *  
 *  1. ESP32 Arduino Core: https://github.com/espressif/arduino-esp32
 *  2. ESP32 Datasheet: https://www.espressif.com/sites/default/files/documentation/esp32_datasheet_en.pdf
 *  
 *  Created on March 14 2019 by Peter Dalmaris
 *  Updated January 2025 for ESP32 Arduino Core 3.x
 * 
 */
 
const byte led_gpio = 32;  // the PWM pin the LED is attached to
int brightness = 0;        // how bright the LED is
int fadeAmount = 1;        // how many points to fade the LED by

// the setup routine runs once when you press reset:
void setup() {
  // Modern ESP32 Arduino Core 3.x API - simplified single function
  // ledcAttach(pin, freq, resolution_bits);
  // Resolution: 1-14 bits (was 1-16 in older cores)
  // Frequency: depends on resolution, typically 1-40MHz
  
  // Using 8-bit resolution for smooth fading (0-255 range)
  ledcAttach(led_gpio, 4000, 8); // 4 kHz PWM, 8-bit resolution (0-255)
  
  // Other resolution examples:
  // ledcAttach(led_gpio, 4000, 10); // 10-bit: 0-1023
  // ledcAttach(led_gpio, 4000, 12); // 12-bit: 0-4095
  // ledcAttach(led_gpio, 5000, 8);  // 5 kHz, 8-bit
}

// the loop routine runs over and over again forever:
void loop() {
  // Modern API - write directly to the pin (no channel number needed)
  ledcWrite(led_gpio, brightness); 
  
  // change the brightness for next time through the loop:
  brightness = brightness + fadeAmount;
  
  // reverse the direction of the fading at the ends of the fade:
  // For 8-bit resolution: range is 0-255
  if (brightness <= 0 || brightness >= 255) {
    fadeAmount = -fadeAmount;
  }
  
  // wait to see the dimming effect
  delay(30); // Changed from 1000ms to 30ms for smoother fading
}
