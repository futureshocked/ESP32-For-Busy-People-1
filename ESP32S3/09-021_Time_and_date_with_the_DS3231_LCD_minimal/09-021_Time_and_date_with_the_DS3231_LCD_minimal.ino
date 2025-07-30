/*  09.021 - DS3231 Real-Time Clock with I2C LCD Display for ESP32-S3 (Minimal Version)
 * 
 * Course: IoT Development with ESP32-S3
 * Target Hardware: ESP32-S3-DevKitC-1 v1.1
 * Module: ESP32-S3-WROOM-1-N16R16V
 * 
 * This minimal sketch demonstrates basic interfacing with the DS3231 Real-Time Clock
 * and displaying time, date, and temperature on an I2C LCD. This version focuses on
 * simplicity and core functionality for easy learning.
 * 
 * LEARNING OBJECTIVES:
 * ===================
 * - Basic I2C communication
 * - Real-time clock reading
 * - LCD display control
 * - Simple embedded programming structure
 * 
 * HARDWARE INFORMATION:
 * =====================
 * Target Board: ESP32-S3-DevKitC-1 v1.1
 * Microcontroller: ESP32-S3-WROOM-1-N16R16V
 * I2C Interface: GPIO21 (SDA), GPIO22 (SCL)
 * Power Supply: 5V for LCD and RTC modules
 * 
 * CIRCUIT DIAGRAM:
 * ================
 * 
 *                    ESP32-S3-DevKitC-1 v1.1
 *                    ┌─────────────────────┐
 *                    │                     │
 *     5V ────────────┤ 5V               21 ├──────── SDA
 *    GND ────────────┤ GND              22 ├──────── SCL
 *                    │                     │
 *                    └─────────────────────┘
 *                              │
 *                              │
 *    ┌─────────────────────────┼─────────────────────────┐
 *    │                         │                         │
 *    │     DS3231 RTC          │        I2C LCD          │
 *    │   ┌──────────────┐      │      ┌──────────────┐   │
 *    │   │ VCC ─────────┼──────┼──────┤ VCC          │   │
 *    │   │ GND ─────────┼──────┼──────┤ GND          │   │
 *    │   │ SDA ─────────┼──────┼──────┤ SDA          │   │
 *    │   │ SCL ─────────┼──────┼──────┤ SCL          │   │
 *    │   └──────────────┘      │      └──────────────┘   │
 *    └─────────────────────────┴─────────────────────────┘
 * 
 * COMPONENTS:
 * ===========
 * 1. ESP32-S3-DevKitC-1 v1.1 Development Board
 * 2. DS3231 RTC Module with battery backup
 * 3. I2C LCD Display (16x2 characters)
 * 4. Connecting wires
 * 
 * GPIO ASSIGNMENTS:
 * =================
 * GPIO21 (SDA)  - I2C Data Line
 * GPIO22 (SCL)  - I2C Clock Line
 * 5V            - Power supply
 * GND           - Ground
 * 
 * TECHNICAL NOTES:
 * ================
 * - Uses default I2C pins for ESP32-S3
 * - LCD I2C address is typically 0x27 (try 0x3F if needed)
 * - Updates display every 10 seconds
 * - Shows date, time, and temperature
 * 
 * IDE REQUIREMENTS:
 * =================
 * - Arduino IDE 2.0+
 * - ESP32 Arduino Core v3.0+
 * - Board: "ESP32S3 Dev Module"
 * - Libraries: RtcDS3231, Wire, LiquidCrystal_I2C
 * 
 * Created: April 3, 2019 by Peter Dalmaris
 * Updated: December 2024 for ESP32-S3 compatibility (Minimal Version)
 */

#include <Wire.h>
#include <RtcDS3231.h>
#include <LiquidCrystal_I2C.h>

// Hardware configuration
const uint8_t LCD_ADDRESS = 0x27;    // LCD I2C address
RtcDS3231<TwoWire> rtc(Wire);        // RTC object
LiquidCrystal_I2C lcd(LCD_ADDRESS, 16, 2);  // LCD object

void setup() {
    // Start serial communication
    Serial.begin(115200);
    Serial.println("ESP32-S3 DS3231 RTC Demo - Minimal Version");
    Serial.printf("Compiled: %s %s\n", __DATE__, __TIME__);
    
    // Initialize LCD
    lcd.init();
    lcd.backlight();
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Starting...");
    
    // Initialize RTC
    rtc.Begin();
    
    // Set RTC to compile time if needed
    RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
    
    if (!rtc.IsDateTimeValid()) {
        Serial.println("RTC setting to compile time");
        rtc.SetDateTime(compiled);
    }
    
    if (!rtc.GetIsRunning()) {
        Serial.println("RTC starting");
        rtc.SetIsRunning(true);
    }
    
    RtcDateTime now = rtc.GetDateTime();
    if (now < compiled) {
        Serial.println("RTC updating to compile time");
        rtc.SetDateTime(compiled);
    }
    
    // Configure RTC
    rtc.Enable32kHzPin(false);
    rtc.SetSquareWavePin(DS3231SquareWavePin_ModeNone);
    
    Serial.println("Setup complete");
    lcd.clear();
}

void loop() {
    // Get current date/time and temperature
    RtcDateTime now = rtc.GetDateTime();
    RtcTemperature temp = rtc.GetTemperature();
    
    // Update LCD display
    updateLCD(now, temp);
    
    // Print to serial monitor
    printDateTime(now);
    Serial.print(" - Temperature: ");
    Serial.print(temp.AsFloatDegC());
    Serial.println("°C");
    
    // Wait 10 seconds before next update
    delay(10000);
}

void updateLCD(const RtcDateTime& dt, RtcTemperature temp) {
    char dateString[20];
    char timeString[20];
    
    // Format date string (MM/DD/YYYY)
    snprintf(dateString, sizeof(dateString), "%02u/%02u/%04u", 
             dt.Month(), dt.Day(), dt.Year());
    
    // Format time string (HH:MM:SS)
    snprintf(timeString, sizeof(timeString), "%02u:%02u:%02u", 
             dt.Hour(), dt.Minute(), dt.Second());
    
    // Display on LCD
    lcd.clear();
    lcd.setCursor(0, 0);        // First row
    lcd.print(dateString);
    
    lcd.setCursor(0, 1);        // Second row
    lcd.print(timeString);
    
    lcd.setCursor(10, 1);       // Temperature on right side
    lcd.print(temp.AsFloatDegC(), 1);
    lcd.print("C");
}

void printDateTime(const RtcDateTime& dt) {
    char dateTimeString[30];
    
    snprintf(dateTimeString, sizeof(dateTimeString), 
             "%02u/%02u/%04u %02u:%02u:%02u", 
             dt.Month(), dt.Day(), dt.Year(),
             dt.Hour(), dt.Minute(), dt.Second());
    
    Serial.print(dateTimeString);
}
