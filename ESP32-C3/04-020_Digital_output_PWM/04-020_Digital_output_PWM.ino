/*  04.020 - ESP32-C3 PWM LED Fade Example
 *
 *  This sketch demonstrates how to use the PWM output of the ESP32-C3
 *  to gradually increase and decrease the brightness of an LED.
 *  
 *  This version is updated for compatibility with the ESP32-C3 Dev Kit and
 *  Arduino-ESP32 core v3.x or later.
 *
 *  Components
 *  ----------
 *  - ESP32-C3 Dev Kit
 *  - LED
 *  - 330 Ohm resistor
 *
 *  IDE
 *  ---
 *  Arduino IDE with ESP32 Arduino Core:
 *  https://github.com/espressif/arduino-esp32
 *
 *  Libraries
 *  ---------
 *  - None required
 *
 *  Connections
 *  -----------
 *  
 *  ESP32-C3 Dev Kit |     LED
 *  ------------------------------
 *        GND        |     Cathode (short leg)
 *        GPIO3      |     Anode (long leg) via resistor
 *
 *  You may use any PWM-capable GPIO. Common options for ESP32-C3 are GPIO2, GPIO3, GPIO10, GPIO21.
 *
 *  Notes
 *  -----
 *  - This version replaces GPIO32 (not available on ESP32-C3) with GPIO3.
 *  - Uses PWM channel 0 at 4kHz with 2-bit resolution.
 *  - Adjust resolution or fade limits to achieve different dimming effects.
 *
 *  Created by Peter Dalmaris
 *  Updated for ESP32-C3 in July 2025
 */

const byte led_gpio = 3;    // PWM-capable pin
int brightness = 0;         // current brightness level
int fadeAmount = 1;         // amount to change brightness each cycle

void setup() {
  // Attach pin to LEDC channel
  ledcAttach(led_gpio, 4000, 2); // freq=4kHz, resolution=2-bit, channel assigned automatically
}

void loop() {
  ledcWrite(led_gpio, brightness); // update brightness

  brightness += fadeAmount;

  // Reverse direction if we hit max or min
  if (brightness <= 0 || brightness >= 3) {
    fadeAmount = -fadeAmount;
  }

  delay(1000); // adjust delay for visible fading effect
}
