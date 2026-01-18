/*  06.043 - Simple 8x8 LED Matrix Display for ESP32-S3 (Minimal Version)
 * 
 * Course: IoT Development with ESP32-S3
 * Target Hardware: ESP32-S3-DevKitC-1 v1.1
 * Module: ESP32-S3-WROOM-1-N16R16V
 * 
 * This minimal sketch shows how to display simple patterns and animations
 * on four 8x8 LED matrix displays. Perfect for beginners to learn the basics
 * of SPI communication and LED matrix control.
 * 
 * HARDWARE INFORMATION:
 * =====================
 * - ESP32-S3-DevKitC-1 v1.1 development board
 * - 4x 8x8 LED Matrix displays with MAX7219/MAX7221 driver
 * - Breadboard and jumper wires
 * - USB-C cable for power and programming
 * 
 * CIRCUIT DIAGRAM:
 * ================
 * 
 *     ESP32-S3          LED Matrix Chain
 *     ┌─────────┐       ┌─────┐ ┌─────┐ ┌─────┐ ┌─────┐
 *     │      3V3├───────┤VCC  │─│VCC  │─│VCC  │─│VCC  │
 *     │      GND├───────┤GND  │─│GND  │─│GND  │─│GND  │
 *     │  GPIO11 ├───────┤DIN  ├─┤DIN  ├─┤DIN  ├─┤DIN  │
 *     │  GPIO12 ├───────┤CLK  ├─┤CLK  ├─┤CLK  ├─┤CLK  │
 *     │  GPIO10 ├───────┤CS   ├─┤CS   ├─┤CS   ├─┤CS   │
 *     └─────────┘       └─────┘ └─────┘ └─────┘ └─────┘
 * 
 * COMPONENTS:
 * ===========
 * - 1x ESP32-S3-DevKitC-1 v1.1
 * - 4x MAX7219 8x8 LED Matrix modules
 * - 8x Jumper wires (Male-to-Male)
 * 
 * GPIO ASSIGNMENTS:
 * =================
 * GPIO10 → CS (Chip Select)
 * GPIO11 → DIN (Data Input)  
 * GPIO12 → CLK (Clock)
 * 3V3    → VCC (Power)
 * GND    → GND (Ground)
 * 
 * TECHNICAL NOTES:
 * ================
 * - Connect displays in chain: ESP32 → Display0 → Display1 → Display2 → Display3
 * - Each display uses about 150mA at full brightness
 * - Total power: 4 × 150mA = 600mA (within ESP32-S3 limits)
 * 
 * IDE REQUIREMENTS:
 * =================
 * - Arduino IDE 2.0+
 * - ESP32 Arduino Core v3.0+
 * - Board: "ESP32S3 Dev Module"
 * - Libraries: SPI, Adafruit_GFX, Max72xxPanel
 * 
 * Created: May 22, 2017 by Peter Dalmaris
 * Updated: December 28, 2024 for ESP32-S3 compatibility
 */

#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Max72xxPanel.h>

// Pin connections for ESP32-S3
const int CS_PIN = 10;    // Chip Select pin

// Create matrix object (CS pin, horizontal displays, vertical displays)
Max72xxPanel matrix = Max72xxPanel(CS_PIN, 4, 1);

// Smiley face pattern - each pair is {x, y} coordinates
const int smiley[25][2] = {
  // Left Eye
  {1,0}, {2,0}, {1,1}, {1,2}, {2,1}, {2,2},
  // Right Eye  
  {5,0}, {6,0}, {5,1}, {5,2}, {6,1}, {6,2},
  // Nose
  {3,1}, {3,2}, {3,3}, {3,4}, {4,4},
  // Mouth
  {0,5}, {1,6}, {2,7}, {3,7}, {4,7}, {5,7}, {6,6}, {7,5}
};

void setup() {
  // Start serial communication for debugging
  Serial.begin(115200);
  Serial.println("ESP32-S3 LED Matrix Demo Starting...");
  
  // Configure the displays
  matrix.setIntensity(4);    // Set brightness (0-15)
  matrix.fillScreen(LOW);    // Clear the screen
  
  // Set display positions (important for 4-display chain)
  matrix.setPosition(0, 3, 0);  // Display 0 at position 3 (rightmost)
  matrix.setPosition(1, 2, 0);  // Display 1 at position 2
  matrix.setPosition(2, 1, 0);  // Display 2 at position 1  
  matrix.setPosition(3, 0, 0);  // Display 3 at position 0 (leftmost)
  
  // Show smiley face pattern
  drawSmiley();
  delay(3000);
  
  Serial.println("Setup complete!");
}

void loop() {
  // Run different animations
  animatePixel();     // Moving pixel animation
  delay(500);
  
  animateCharacter(); // Moving character animation  
  delay(500);
  
  drawSmiley();       // Show smiley face
  delay(2000);
}

// Draw the smiley face using the pattern array
void drawSmiley() {
  matrix.fillScreen(LOW);  // Clear screen
  
  // Draw each pixel of the smiley face
  for (int i = 0; i < 25; i++) {
    int x = smiley[i][0];
    int y = smiley[i][1]; 
    matrix.drawPixel(x, y, HIGH);
  }
  
  matrix.write();  // Update the display
}

// Animate a single pixel moving back and forth
void animatePixel() {
  matrix.fillScreen(LOW);
  
  // Move pixel left to right
  for (int x = 0; x < 32; x++) {
    matrix.drawPixel(x, 3, HIGH);  // Turn on pixel
    matrix.write();
    delay(50);
    matrix.drawPixel(x, 3, LOW);   // Turn off pixel
  }
  
  // Move pixel right to left
  for (int x = 31; x >= 0; x--) {
    matrix.drawPixel(x, 3, HIGH);  // Turn on pixel
    matrix.write();
    delay(50);
    matrix.drawPixel(x, 3, LOW);   // Turn off pixel
  }
}

// Animate a character moving across the display
void animateCharacter() {
  matrix.fillScreen(LOW);
  
  // Move character 'A' left to right
  for (int x = 0; x < 32; x++) {
    matrix.drawChar(x, 0, 'A', HIGH, LOW, 1);  // Draw character
    matrix.write();
    delay(40);
    matrix.drawChar(x, 0, 'A', LOW, LOW, 1);   // Erase character
  }
  
  // Move character 'A' right to left
  for (int x = 31; x >= 0; x--) {
    matrix.drawChar(x, 0, 'A', HIGH, LOW, 1);  // Draw character
    matrix.write();
    delay(40);
    matrix.drawChar(x, 0, 'A', LOW, LOW, 1);   // Erase character
  }
}
