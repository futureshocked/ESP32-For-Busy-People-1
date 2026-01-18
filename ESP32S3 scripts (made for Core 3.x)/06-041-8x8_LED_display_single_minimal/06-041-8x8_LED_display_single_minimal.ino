/*  06.041 - Single 8x8 LED Matrix Display (Minimal Version) for ESP32-S3
 * 
 * Course: IoT Development with ESP32-S3
 * Target Hardware: ESP32-S3-DevKitC-1 v1.1
 * Module: ESP32-S3-WROOM-1-N16R16V
 * 
 * Simple sketch showing how to display a smiley face on an 8x8 LED matrix
 * using the MAX72xx driver IC via SPI communication. This minimal version
 * focuses on basic functionality perfect for beginners.
 * 
 * HARDWARE INFORMATION:
 * =====================
 * ESP32-S3-DevKitC-1 with 16MB Flash and 16MB PSRAM
 * MAX72xx 8x8 LED Matrix Display
 * 
 * CIRCUIT DIAGRAM:
 * ================
 *    ESP32-S3          8x8 LED Matrix
 *   ┌────────┐        ┌─────────────┐
 *   │   3.3V ├────────┤ VCC         │
 *   │    GND ├────────┤ GND         │
 *   │ GPIO11 ├────────┤ DIN         │
 *   │ GPIO12 ├────────┤ CLK         │
 *   │ GPIO10 ├────────┤ CS          │
 *   └────────┘        └─────────────┘
 * 
 * COMPONENTS:
 * ===========
 * 1x ESP32-S3-DevKitC-1 board
 * 1x 8x8 LED Matrix with MAX7219/MAX7221 driver
 * 5x Jumper wires
 * 
 * GPIO ASSIGNMENTS:
 * =================
 * GPIO10 - CS (Chip Select)
 * GPIO11 - MOSI (DIN - Data Input)
 * GPIO12 - SCK (CLK - Clock)
 * 
 * TECHNICAL NOTES:
 * ================
 * Uses default ESP32-S3 SPI pins (FSPI peripheral)
 * Matrix brightness: 0-15 (4 = medium brightness)
 * Coordinate system: (0,0) = top-left corner
 * 
 * IDE REQUIREMENTS:
 * =================
 * - Arduino IDE 2.0+
 * - ESP32 Arduino Core v3.0+
 * - Board: "ESP32S3 Dev Module"
 * - Libraries: SPI, Adafruit_GFX, Max72xxPanel
 * 
 * Created: April 1, 2019 by Peter Dalmaris
 * Updated: December 2024 for ESP32-S3 minimal implementation
 */

#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Max72xxPanel.h>

// ===============================================================================
// CONFIGURATION
// ===============================================================================

// Pin definitions for ESP32-S3 SPI
const int PIN_CS = 10;      // Chip Select

// Matrix settings
const int BRIGHTNESS = 4;   // Brightness level (0-15)

// Smiley face pattern coordinates
const int SMILEY_PIXELS = 25;
int smiley[SMILEY_PIXELS][2] = {
  // Left eye
  {1, 0}, {2, 0}, {1, 1}, {1, 2}, {2, 1}, {2, 2},
  // Right eye  
  {5, 0}, {6, 0}, {5, 1}, {5, 2}, {6, 1}, {6, 2},
  // Nose
  {3, 1}, {3, 2}, {3, 3}, {3, 4}, {4, 4},
  // Mouth
  {0, 5}, {1, 6}, {2, 7}, {3, 7}, {4, 7}, {5, 7}, {6, 6}, {7, 5}
};

// ===============================================================================
// OBJECTS
// ===============================================================================

// Create matrix object (CS pin, horizontal displays, vertical displays)
Max72xxPanel matrix = Max72xxPanel(PIN_CS, 1, 1);

// ===============================================================================
// SETUP
// ===============================================================================

void setup() {
  // Start serial communication
  Serial.begin(115200);
  Serial.println("ESP32-S3 LED Matrix Demo - Minimal Version");
  
  // Configure the LED matrix
  matrix.setIntensity(BRIGHTNESS);  // Set brightness
  matrix.fillScreen(LOW);           // Clear display
  
  // Draw smiley face
  drawSmiley();
  
  Serial.println("Smiley face displayed!");
  Serial.println("Uncomment animate_pixel() in loop() for animation");
}

// ===============================================================================
// MAIN LOOP
// ===============================================================================

void loop() {
  // Static display - uncomment line below for animation
  // animate_pixel();
  
  delay(100);  // Small delay to prevent overwhelming the processor
}

// ===============================================================================
// FUNCTIONS
// ===============================================================================

void drawSmiley() {
  // Draw each pixel of the smiley face
  for (int pixel = 0; pixel < SMILEY_PIXELS; pixel++) {
    int x = smiley[pixel][0];  // X coordinate
    int y = smiley[pixel][1];  // Y coordinate
    matrix.drawPixel(x, y, HIGH);  // Turn on pixel
  }
  
  matrix.write();  // Update the display
}

void animate_pixel() {
  // Simple animation: pixel moving left and right
  
  // Clear screen
  matrix.fillScreen(LOW);
  
  // Move pixel left to right
  for (int x = 0; x < 8; x++) {
    matrix.drawPixel(x, 3, HIGH);  // Turn on pixel
    matrix.write();                // Update display
    delay(100);                    // Wait
    matrix.drawPixel(x, 3, LOW);   // Turn off pixel
    matrix.write();                // Update display
  }
  
  // Move pixel right to left
  for (int x = 7; x >= 0; x--) {
    matrix.drawPixel(x, 3, HIGH);  // Turn on pixel
    matrix.write();                // Update display
    delay(100);                    // Wait
    matrix.drawPixel(x, 3, LOW);   // Turn off pixel
    matrix.write();                // Update display
  }
}
