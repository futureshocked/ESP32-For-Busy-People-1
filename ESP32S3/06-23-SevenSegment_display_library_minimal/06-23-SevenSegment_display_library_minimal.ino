/*  06.023 - Minimal 7-Segment Display Example for ESP32-S3
 * 
 * Course: IoT Development with ESP32-S3
 * Target Hardware: ESP32-S3-DevKitC-1 v1.1
 * Module: ESP32-S3-WROOM-1-N16R16V
 * 
 * This minimal sketch shows how to drive a single-digit common cathode 7-segment display 
 * using the digital pins of the ESP32-S3. It cycles through digits 0–9 by writing segment 
 * patterns directly to GPIO pins. This example is designed for beginners and students who 
 * want to quickly prototype and understand how 7-segment displays are controlled.
 * 
 * HARDWARE INFORMATION:
 * =====================
 * - ESP32-S3-WROOM-1-N16R16V
 * - Flash: 16 MB, PSRAM: 16 MB
 * - Use GPIO1 to GPIO8 for LED segments
 * - Common cathode display: connect cathode to GND
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
 * - Uses direct digitalWrite() for clarity
 * - Ideal for learning basic segment control logic
 * - Decimal point lights only for digit 9
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

// Segment pin assignments for A to DP
const uint8_t segmentPins[8] = {1, 2, 3, 4, 5, 6, 7, 8};

// Binary segment encoding for digits 0–9 (common cathode)
const uint8_t digits[10][8] = {
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

int digit = 0;

void setup() {
  for (int i = 0; i < 8; i++) {
    pinMode(segmentPins[i], OUTPUT);
  }
}

void loop() {
  for (int i = 0; i < 8; i++) {
    digitalWrite(segmentPins[i], digits[digit][i]);
  }
  delay(1000);
  digit = (digit + 1) % 10;
}
