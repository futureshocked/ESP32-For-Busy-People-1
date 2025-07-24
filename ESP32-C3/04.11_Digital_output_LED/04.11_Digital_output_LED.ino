/*  04.010 - ESP32-C3 Digital Output Blink LED Example
 *
 *  This sketch toggles the state of an LED connected to a GPIO pin of the ESP32-C3 Dev Kit.
 *  
 *  This version is updated to be compatible with the ESP32-C3 using the latest Arduino-ESP32
 *  core (v3.x and later).
 *
 *  Components
 *  ----------
 *  - ESP32-C3 Dev Kit
 *  - LED
 *  - 330 Ohm resistor
 *
 *  IDE
 *  ---
 *  Arduino IDE with ESP32 Arduino Core
 *  https://github.com/espressif/arduino-esp32
 *
 *  Libraries
 *  ---------
 *  - None
 *
 *  Connections
 *  -----------
 *  
 *  ESP32-C3 Dev Kit |     LED
 *  ------------------------------
 *        GND        |     Cathode (short leg)
 *        GPIO3      |     Anode (long leg) via resistor
 *
 *  You may use a different GPIO. Make sure it supports digital output.
 *  For ESP32-C3, available GPIOs include 0–21 (excluding special-purpose pins).
 *
 *  Notes
 *  -----
 *  - This sketch replaces GPIO32 from the original ESP32 version with GPIO3,
 *    which is available on the ESP32-C3.
 *  - The built-in LED (if available) may also be connected to GPIO3 or GPIO8 depending on the board.
 *
 *  Created by Peter Dalmaris
 *  Updated for ESP32-C3 in July 2025
 */

const byte LED_GPIO = 3;

void setup() {
  pinMode(LED_GPIO, OUTPUT);
}

void loop() {
  digitalWrite(LED_GPIO, HIGH);  // turn the LED on
  delay(1000);                   // wait for a second
  digitalWrite(LED_GPIO, LOW);   // turn the LED off
  delay(1000);                   // wait for a second
}
