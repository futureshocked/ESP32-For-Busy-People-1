/*  04.030 - ESP32-C3 Onboard RGB LED Example (WS2812, GPIO48)
 *
 *  This sketch controls the onboard WS2812 RGB LED connected to GPIO48.
 *  It cycles through red, green, blue, white, and off every second at 50% brightness.
 *
 *  Board
 *  -----
 *  - ESP32-C3 DevKit with WS2812 RGB LED (GPIO48)
 *
 *  Libraries
 *  ---------
 *  - Adafruit NeoPixel
 *    https://github.com/adafruit/Adafruit_NeoPixel
 *
 *  Notes
 *  -----
 *  - Make sure to install the Adafruit NeoPixel library via Library Manager.
 *  - GPIO48 is used for data input to the onboard addressable RGB LED.
 *  - Colors are driven at 50% brightness.
 *
 *  Created by Peter Dalmaris
 *  Updated for ESP32-C3 and GPIO48 on July 2025
 */

#include <Adafruit_NeoPixel.h>

#define LED_PIN     48    // GPIO48 for onboard RGB LED
#define LED_COUNT   1     // Only one onboard LED

Adafruit_NeoPixel rgb(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

// Define RGB color values at 50% brightness (0–127)
uint32_t RED    = rgb.Color(127, 0, 0);
uint32_t GREEN  = rgb.Color(0, 127, 0);
uint32_t BLUE   = rgb.Color(0, 0, 127);
uint32_t WHITE  = rgb.Color(127, 127, 127);
uint32_t OFF    = rgb.Color(0, 0, 0);

void setup() {
  rgb.begin();
  rgb.setBrightness(127);  // 50% of 255
  rgb.show();              // Turn off initially
}

void loop() {
  rgb.setPixelColor(0, RED);
  rgb.show();
  delay(1000);

  rgb.setPixelColor(0, GREEN);
  rgb.show();
  delay(1000);

  rgb.setPixelColor(0, BLUE);
  rgb.show();
  delay(1000);

  rgb.setPixelColor(0, WHITE);
  rgb.show();
  delay(1000);

  rgb.setPixelColor(0, OFF);
  rgb.show();
  delay(1000);
}
