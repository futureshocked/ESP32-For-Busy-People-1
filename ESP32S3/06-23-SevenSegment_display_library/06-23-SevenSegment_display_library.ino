/*  06.023 - Single-Digit 7-Segment Display with Manual Control for ESP32-S3
 * 
 * Course: IoT Development with ESP32-S3
 * Target Hardware: ESP32-S3-DevKitC-1 v1.1
 * Module: ESP32-S3-WROOM-1-N16R16V
 * 
 * This sketch demonstrates how to control a single-digit common cathode 7-segment LED display
 * using digital output pins of the ESP32-S3. The code cycles through numbers 0–9 and lights up 
 * the appropriate segments using GPIOs, with the option to include the decimal point.
 * 
 * HARDWARE INFORMATION:
 * =====================
 * - ESP32-S3-WROOM-1-N16R16V
 * - Flash: 16 MB, PSRAM: 16 MB
 * - Digital I/O pins: use GPIOs 1–21
 * 
 * CIRCUIT DIAGRAM:
 * ================
 * 
 *         A
 *      ───────
 *     |       |
 *   F |       | B
 *     |   G   |
 *      ───────
 *     |       |
 *   E |       | C
 *     |   D   |
 *      ───────  ● DP
 * 
 * GPIO CONNECTIONS:
 * =================
 *   Segment | GPIO | Resistor (220 Ω)
 *   --------+------+-----------------
 *     A     |  1   | Yes
 *     B     |  2   | Yes
 *     C     |  3   | Yes
 *     D     |  4   | Yes
 *     E     |  5   | Yes
 *     F     |  6   | Yes
 *     G     |  7   | Yes
 *     DP    |  8   | Yes
 *   COM     | GND  | (Common Cathode)
 * 
 * COMPONENTS:
 * ===========
 * - 1x ESP32-S3-DevKitC-1 v1.1
 * - 1x Common cathode 7-segment display
 * - 8x 220-ohm resistors
 * - Breadboard and jumper wires
 * 
 * TECHNICAL NOTES:
 * ================
 * - We use digitalWrite() for maximum transparency.
 * - Decimal point is lit only with the number 9.
 * 
 * IDE REQUIREMENTS:
 * =================
 * - Arduino IDE 2.0+
 * - ESP32 Arduino Core v3.0+
 * - Board: "ESP32S3 Dev Module"
 * 
 * Created: April 1, 2019 by Peter Dalmaris
 * Updated: July 28, 2025 for ESP32-S3 compatibility
 */

#include <Arduino.h>

// Segment mapping (A to DP)
constexpr uint8_t segmentPins[8] = {1, 2, 3, 4, 5, 6, 7, 8};

// Digit-to-segment encoding for 0–9 with common cathode
const uint8_t digitEncoding[10][8] = {
  {1,1,1,1,1,1,0,0}, // 0
  {0,1,1,0,0,0,0,0}, // 1
  {1,1,0,1,1,0,1,0}, // 2
  {1,1,1,1,0,0,1,0}, // 3
  {0,1,1,0,0,1,1,0}, // 4
  {1,0,1,1,0,1,1,0}, // 5
  {1,0,1,1,1,1,1,0}, // 6
  {1,1,1,0,0,0,0,0}, // 7
  {1,1,1,1,1,1,1,0}, // 8
  {1,1,1,1,0,1,1,1}  // 9 with decimal point ON
};

int currentDigit = 0;

// ======= Initialization =======
void setup() {
  Serial.begin(115200);
  Serial.println("Starting 7-segment display test...");

  // Set all segment pins as OUTPUT
  for (int i = 0; i < 8; i++) {
    pinMode(segmentPins[i], OUTPUT);
    digitalWrite(segmentPins[i], LOW); // turn off initially
  }
}

// ======= Main Loop =======
void loop() {
  displayDigit(currentDigit);
  Serial.printf("Displaying: %d\n", currentDigit);
  delay(1000);

  currentDigit = (currentDigit + 1) % 10;
}

// ======= Helper Functions =======
void displayDigit(uint8_t digit) {
  if (digit > 9) return;

  for (int i = 0; i < 8; i++) {
    digitalWrite(segmentPins[i], digitEncoding[digit][i] ? HIGH : LOW);
  }
}
