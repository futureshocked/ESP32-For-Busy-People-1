/*  ESP32-S3 LED Fade Example using ledcAttach()
 *
 *  This sketch gradually increases and decreases the brightness
 *  of an LED connected to GPIO4 using PWM.
 *
 *  Board: ESP32-S3 DevKit (e.g. DevKitC-1 N16R8)
 *  Pin:   GPIO4 (PWM-capable)
 *  Brightness: 0–255 (8-bit resolution)
 *
 *  Author: Peter Dalmaris
 *  Updated: July 2025
 */

const byte LED_PIN = 5;
const int PWM_FREQ = 5000;
const int PWM_RESOLUTION = 8;
int brightness = 0;
int fadeAmount = 5;

void setup() {
  ledcAttach(LED_PIN, PWM_FREQ, PWM_RESOLUTION);
}

void loop() {
  ledcWrite(LED_PIN, brightness);
  brightness += fadeAmount;

  if (brightness <= 0 || brightness >= 255) {
    fadeAmount = -fadeAmount;
  }

  delay(20); // Slower fade
}
