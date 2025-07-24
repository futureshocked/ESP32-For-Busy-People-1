/*  04.030 - ESP32-C3 Onboard RGB LED Example (WS2812)
 *
 *  This sketch uses the onboard WS2812 RGB LED on the ESP32-C3 DevKit.
 *  It cycles through a range of hues using the Adafruit NeoPixel library.
 *
 *  Components
 *  ----------
 *  - ESP32-C3 Dev Kit (e.g. DevKitM-1, DevKitC-02)
 *  - Onboard RGB LED (WS2812/NeoPixel, GPIO8)
 *
 *  IDE
 *  ---
 *  Arduino IDE with ESP32 Arduino Core v3.x+
 *
 *  Libraries
 *  ---------
 *  - Adafruit NeoPixel (install via Library Manager)
 *    https://github.com/adafruit/Adafruit_NeoPixel
 *
 *  Connections
 *  -----------
 *  - GPIO8 -> WS2812 Data In
 *  - +3.3V and GND are already wired on-board
 *
 *  Notes
 *  -----
 *  - This LED is RGB **addressable**, not analog (PWM) like discrete LEDs.
 *  - Use only 3.3V logic level on GPIO8 (already correct for ESP32-C3).
 *
 *  Created by Peter Dalmaris
 *  Updated for ESP32-C3 onboard RGB LED, July 2025
 */

#include <Adafruit_NeoPixel.h>

#define LED_PIN     8     // Onboard RGB LED data pin
#define LED_COUNT   1     // Only one LED

Adafruit_NeoPixel rgb(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

uint8_t color = 0;          // hue (0–255)
uint8_t brightness = 50;    // brightness (0–255)

void setup() {
  rgb.begin();            // initialize the LED
  rgb.setBrightness(brightness);
  rgb.show();             // turn off all LEDs initially
}

void loop() {
  // Cycle through hue values from 0 to 255
  for (color = 0; color < 255; color++) {
    rgb.setPixelColor(0, rgb.ColorHSV(color * 256));  // 256 = full hue step
    rgb.show();
    delay(50);
  }
}
