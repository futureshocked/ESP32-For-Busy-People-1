/*  06.011 - Simple I2C LCD Display for ESP32-S3
 * 
 * Course: IoT Development with ESP32-S3
 * Target Hardware: ESP32-S3-DevKitC-1 v1.1  
 * Module: ESP32-S3-WROOM-1-N16R16V
 * 
 * This simple sketch shows how to use a 16x2 LCD display with an I2C backpack
 * on the ESP32-S3. This version focuses on basic functionality and is perfect
 * for beginners learning LCD interfacing.
 * 
 * HARDWARE INFORMATION:
 * =====================
 * Uses I2C communication to reduce wiring to just 4 connections.
 * The I2C backpack contains a PCF8574 port expander chip.
 * 
 * CIRCUIT DIAGRAM:
 * ================
 * 
 *   ESP32-S3          I2C LCD Backpack
 *   ┌─────────┐      ┌─────────────────┐
 *   │ 3V3 ────┼──────┼─ VCC            │
 *   │ GND ────┼──────┼─ GND            │
 *   │ GPIO21 ─┼──────┼─ SDA  [16x2 LCD]│
 *   │ GPIO8 ──┼──────┼─ SCL  [Display] │
 *   └─────────┘      └─────────────────┘
 * 
 * COMPONENTS:
 * ===========
 * - ESP32-S3-DevKitC-1 v1.1
 * - 16x2 LCD with I2C backpack (PCF8574)
 * - 4x jumper wires
 * 
 * GPIO ASSIGNMENTS:
 * =================
 * GPIO21: I2C SDA (data line)
 * GPIO8:  I2C SCL (clock line)
 * 3V3:    Power (+3.3V)
 * GND:    Ground
 * 
 * TECHNICAL NOTES:
 * ================
 * - Most I2C LCD backpacks use address 0x27 or 0x3F
 * - If display doesn't work, try changing the address in code
 * - ESP32-S3 has built-in I2C pull-up resistors
 * 
 * IDE REQUIREMENTS:
 * =================
 * - Arduino IDE 2.0+
 * - ESP32 Arduino Core v3.0+
 * - Board: "ESP32S3 Dev Module"
 * - Library: LiquidCrystal_I2C (install from Library Manager)
 * 
 * Created: April 1, 2019 by Peter Dalmaris
 * Updated: December 2024 for ESP32-S3 compatibility
 */

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Pin definitions for ESP32-S3
const int SDA_PIN = 21;
const int SCL_PIN = 8;

// Create LCD object (address, columns, rows)
// Try 0x3F if 0x27 doesn't work
LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
    // Start serial communication for debugging
    Serial.begin(115200);
    Serial.println("ESP32-S3 LCD Demo Starting...");
    
    // Initialize I2C with custom pins
    Wire.begin(SDA_PIN, SCL_PIN);
    
    // Initialize the LCD
    lcd.init();
    lcd.backlight();
    
    // Display welcome message
    lcd.setCursor(0, 0);
    lcd.print("Hello ESP32-S3!");
    lcd.setCursor(0, 1);
    lcd.print("I2C LCD Works!");
    
    delay(2000);
    lcd.clear();
    
    // Show row positions
    lcd.setCursor(0, 0);
    lcd.print("Row 1: Line 0");
    lcd.setCursor(0, 1);
    lcd.print("Row 2: Line 1");
    
    delay(2000);
    lcd.clear();
    
    // Demonstrate backlight control
    lcd.setCursor(0, 0);
    lcd.print("Backlight Test");
    
    // Turn backlight off and on
    lcd.setCursor(0, 1);
    lcd.print("Backlight OFF");
    lcd.noBacklight();
    delay(2000);
    
    lcd.backlight();
    lcd.setCursor(0, 1);
    lcd.print("Backlight ON ");
    delay(2000);
    
    // Scrolling demonstration
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Scrolling Text");
    
    // Scroll right
    for (int i = 0; i < 5; i++) {
        delay(500);
        lcd.scrollDisplayRight();
    }
    
    delay(1000);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Demo Complete!");
    
    Serial.println("Setup finished. Starting main loop...");
}

void loop() {
    // Simple counter display
    static int counter = 0;
    
    lcd.setCursor(0, 1);
    lcd.print("Count: ");
    lcd.print(counter);
    lcd.print("    "); // Clear extra digits
    
    counter++;
    if (counter > 9999) {
        counter = 0;
    }
    
    delay(1000);
}
