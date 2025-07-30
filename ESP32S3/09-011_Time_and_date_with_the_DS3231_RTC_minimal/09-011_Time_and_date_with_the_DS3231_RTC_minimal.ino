/*  09.011 - Time and Date with DS3231 RTC for ESP32-S3 (MINIMAL VERSION)
 * 
 * Course: IoT Development with ESP32-S3
 * Target Hardware: ESP32-S3-DevKitC-1 v1.1
 * Module: ESP32-S3-WROOM-1-N16R16V
 * 
 * This sketch demonstrates how to set the time and date, and then display them
 * to the serial monitor using the highly accurate DS3231 real time clock.
 * When equipped with a coin battery (CR2032), the DS3231 will retain correct 
 * time and date even when main power is disconnected. The DS3231 also contains
 * an integrated temperature sensor accessible via the library.
 * 
 * LEARNING OBJECTIVES:
 * ===================
 * - Understanding I2C communication protocol
 * - Working with Real Time Clock (RTC) modules
 * - Handling date/time data structures
 * - Implementing error handling for I2C devices
 * - Reading temperature sensors
 * - Formatting date/time output strings
 * 
 * HARDWARE INFORMATION:
 * =====================
 * ESP32-S3-DevKitC-1 v1.1 Features:
 * - Dual-core Xtensa LX7 CPU up to 240 MHz
 * - 16 MB Flash Memory
 * - 16 MB PSRAM  
 * - WiFi 802.11 b/g/n + Bluetooth 5.0 LE
 * - 45 programmable GPIOs
 * - Multiple ADC, SPI, I2C, UART interfaces
 * - USB-C connector with native USB support
 * 
 * DS3231 RTC Module Features:
 * - Highly accurate ±2ppm from 0°C to +40°C
 * - Battery backup maintains time during power loss
 * - Integrated temperature sensor (-40°C to +85°C)
 * - I2C interface (address 0x68)
 * - 32kHz output and programmable square wave
 * - Automatic leap year compensation until 2100
 * 
 * CIRCUIT DIAGRAM:
 * ================
 * 
 *     ESP32-S3-DevKitC-1                    DS3231 RTC Module
 *     ==================                    ==================
 *                                          
 *           3.3V  ●─────────────────────────● VCC
 *            GND  ●─────────────────────────● GND  
 *    GPIO47 (SDA) ●─────────────────────────● SDA
 *    GPIO48 (SCL) ●─────────────────────────● SCL
 *                                          ● SQW (Square Wave - not used)
 *                                          ● 32K (32kHz output - not used)
 *                                          
 *                                          [CR2032 Battery] (for backup power)
 * 
 * COMPONENTS:
 * ===========
 * 1x ESP32-S3-DevKitC-1 v1.1 development board
 * 1x DS3231 RTC module breakout board  
 * 1x CR2032 coin battery (3V lithium)
 * 4x Jumper wires (male-to-female recommended)
 * 1x Breadboard (optional, for organized connections)
 * 
 * Note: Most DS3231 modules include pull-up resistors for I2C lines
 * 
 * GPIO ASSIGNMENTS:
 * =================
 * GPIO47 (SDA) - I2C Serial Data Line
 * GPIO48 (SCL) - I2C Serial Clock Line
 * 
 * These pins are chosen as they are commonly used for I2C on ESP32-S3
 * and are available on the expansion headers of the DevKitC-1 board.
 * 
 * TECHNICAL NOTES:
 * ================
 * - DS3231 uses I2C address 0x68 (fixed, not configurable)
 * - ESP32-S3 I2C supports both master and slave modes (we use master)
 * - Standard I2C speed is 100kHz, Fast mode supports up to 400kHz
 * - Battery backup allows RTC to run for years when main power is off
 * - Temperature readings are updated every 64 seconds automatically
 * - This implementation includes comprehensive error checking
 * 
 * POWER CONSUMPTION:
 * ==================
 * - DS3231 active current: ~100µA
 * - DS3231 battery backup: ~1µA (years of operation)
 * - ESP32-S3 active: ~40-150mA (depending on CPU frequency)
 * 
 * IDE REQUIREMENTS:
 * =================
 * - Arduino IDE 2.0+
 * - ESP32 Arduino Core v3.0+
 * - Board: "ESP32S3 Dev Module"
 * - Libraries: RtcDS3231, Wire (included with ESP32 core)
 * 
 * LIBRARY INSTALLATION:
 * =====================
 * 1. Open Arduino IDE
 * 2. Go to Tools > Manage Libraries
 * 3. Search for "Rtc by Makuna"
 * 4. Install the latest version
 * 
 * TROUBLESHOOTING:
 * ================
 * - If "RTC communications error" appears: Check wiring and connections
 * - If time keeps resetting: Replace CR2032 battery
 * - If compilation fails: Ensure ESP32 board package is installed
 * - If I2C errors occur: Try adding external pull-up resistors (4.7kΩ)
 * 
 * Created: April 3, 2019 by Peter Dalmaris
 * Updated: December 2024 for ESP32-S3 compatibility and educational enhancement
 */

#include <Wire.h>           // I2C communication library
#include <RtcDS3231.h>      // DS3231 RTC library

// Pin definitions for ESP32-S3
const int SDA_PIN = 47;     // I2C Data pin
const int SCL_PIN = 48;     // I2C Clock pin

// Create RTC object
RtcDS3231<TwoWire> Rtc(Wire);

// Helper function to print date and time
#define countof(a) (sizeof(a) / sizeof(a[0]))

void printDateTime(const RtcDateTime& dt) {
    char datestring[20];
    
    snprintf_P(datestring,
               countof(datestring),
               PSTR("%02u/%02u/%04u %02u:%02u:%02u"),
               dt.Month(),
               dt.Day(),
               dt.Year(),
               dt.Hour(),
               dt.Minute(),
               dt.Second());
    
    Serial.print(datestring);
}

void setup() {
    // Start serial communication
    Serial.begin(115200);
    Serial.println("ESP32-S3 RTC Demo Starting...");
    
    // Show when this sketch was compiled
    Serial.print("Compiled: ");
    Serial.print(__DATE__);
    Serial.print(" ");
    Serial.println(__TIME__);
    
    // Initialize I2C with custom pins for ESP32-S3
    Wire.begin(SDA_PIN, SCL_PIN);
    
    // Initialize the RTC
    Rtc.Begin();
    
    // Get compilation time to set RTC if needed
    RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
    Serial.print("Compile time: ");
    printDateTime(compiled);
    Serial.println();
    
    // Check if RTC has valid time
    if (!Rtc.IsDateTimeValid()) {
        if (Rtc.LastError() != 0) {
            // Communication error with RTC
            Serial.print("RTC communication error = ");
            Serial.println(Rtc.LastError());
        } else {
            // RTC lost time (first use or dead battery)
            Serial.println("RTC lost time! Setting to compile time.");
            Rtc.SetDateTime(compiled);
        }
    }
    
    // Make sure RTC is running
    if (!Rtc.GetIsRunning()) {
        Serial.println("RTC was not running, starting now");
        Rtc.SetIsRunning(true);
    }
    
    // Check if RTC time is older than compile time
    RtcDateTime now = Rtc.GetDateTime();
    if (now < compiled) {
        Serial.println("RTC is older than compile time! Updating...");
        Rtc.SetDateTime(compiled);
    } else if (now > compiled) {
        Serial.println("RTC is newer than compile time (this is normal)");
    } else {
        Serial.println("RTC matches compile time exactly");
    }
    
    // Turn off unneeded features to save power
    Rtc.Enable32kHzPin(false);
    Rtc.SetSquareWavePin(DS3231SquareWavePin_ModeNone);
    
    Serial.println("Setup complete! Starting time display...");
    Serial.println("Format: MM/DD/YYYY HH:MM:SS Temperature");
}

void loop() {
    // Get current time from RTC
    RtcDateTime now = Rtc.GetDateTime();
    
    // Print the current date and time
    printDateTime(now);
    Serial.print("  ");
    
    // Get and print temperature
    RtcTemperature temp = Rtc.GetTemperature();
    temp.Print(Serial);
    Serial.println("C");
    
    // Wait 10 seconds before next reading
    delay(10000);
}
