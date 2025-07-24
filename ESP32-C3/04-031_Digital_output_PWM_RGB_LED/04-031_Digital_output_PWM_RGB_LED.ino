/*  04.030 - ESP32-C3 PWM RGB LED Example
 *
 *  This sketch demonstrates how to use PWM to drive a common anode RGB LED.
 *  Updated to work with the ESP32-C3 and the latest Arduino-ESP32 core (v3.x+).
 *
 *  Components
 *  ----------
 *  - ESP32-C3 Dev Kit
 *  - RGB LED (common anode or cathode)
 *  - 3 × 330 Ohm resistors
 *
 *  IDE
 *  ---
 *  Arduino IDE with ESP32 Arduino Core:
 *  https://github.com/espressif/arduino-esp32
 *
 *  Connections
 *  -----------
 *  ESP32-C3 Dev Kit |     RGB LED
 *  ------------------------------
 *        GND        |  Common Anode
 *        GPIO2      |  Red pin via resistor
 *        GPIO3      |  Green pin via resistor
 *        GPIO10     |  Blue pin via resistor
 *
 *  Notes
 *  -----
 *  - GPIO32/33/25 used in the original ESP32 are not available on the ESP32-C3.
 *  - Uses `ledcAttach(pin, freq, resolution)` from Arduino-ESP32 v3+.
 *  - The code supports common anode or cathode LEDs via the `invert` flag.
 *
 *  Created by Peter Dalmaris
 *  Updated for ESP32-C3 in July 2025
 */

// PWM-capable GPIOs for ESP32-C3
const uint8_t ledR = 2;
const uint8_t ledG = 3;
const uint8_t ledB = 10;

const bool invert = true; // true = common anode, false = common cathode

uint8_t color = 0;
uint8_t brightness = 255;

uint32_t R, G, B;

void setup() {
  Serial.begin(115200);
  delay(10);

  // Attach pins with PWM: freq 12kHz, 8-bit resolution
  ledcAttach(ledR, 12000, 8);
  ledcAttach(ledG, 12000, 8);
  ledcAttach(ledB, 12000, 8);
}

void loop() {
  Serial.println("All LEDs ON");
  ledcWrite(ledR, invert ? 0 : 255);
  ledcWrite(ledG, invert ? 0 : 255);
  ledcWrite(ledB, invert ? 0 : 255);
  delay(2000);

  Serial.println("All LEDs OFF");
  ledcWrite(ledR, invert ? 255 : 0);
  ledcWrite(ledG, invert ? 255 : 0);
  ledcWrite(ledB, invert ? 255 : 0);
  delay(2000);

  Serial.println("Starting color fade...");

  for (color = 0; color < 255; color++) {
    hueToRGB(color, brightness);

    ledcWrite(ledR, R);
    ledcWrite(ledG, G);
    ledcWrite(ledB, B);

    delay(100);
  }
}

// Convert hue to RGB values (0–255)
void hueToRGB(uint8_t hue, uint8_t brightness) {
  uint16_t scaledHue = hue * 6;
  uint8_t segment = scaledHue / 256;
  uint16_t segmentOffset = scaledHue - (segment * 256);

  uint8_t complement = 0;
  uint16_t prev = (brightness * (255 - segmentOffset)) / 256;
  uint16_t next = (brightness * segmentOffset) / 256;

  if (invert) {
    brightness = 255 - brightness;
    complement = 255;
    prev = 255 - prev;
    next = 255 - next;
  }

  switch (segment) {
    case 0: R = brightness; G = next;       B = complement; break;
    case 1: R = prev;       G = brightness; B = complement; break;
    case 2: R = complement; G = brightness; B = next;       break;
    case 3: R = complement; G = prev;       B = brightness; break;
    case 4: R = next;       G = complement; B = brightness; break;
    case 5:
    default: R = brightness; G = complement; B = prev;      break;
  }
}
