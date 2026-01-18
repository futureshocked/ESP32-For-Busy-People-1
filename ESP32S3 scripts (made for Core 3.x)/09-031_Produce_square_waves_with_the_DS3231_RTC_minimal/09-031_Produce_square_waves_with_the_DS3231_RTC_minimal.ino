/*  09.031 - Basic Square Wave Generation with DS3231 RTC for ESP32-S3
 * 
 * Course: IoT Development with ESP32-S3
 * Target Hardware: ESP32-S3-DevKitC-1 v1.1
 * Module: ESP32-S3-WROOM-1-N16R16V
 * 
 * This minimal sketch demonstrates basic square wave generation using the DS3231 
 * real-time clock module with the ESP32-S3. This simplified version focuses on 
 * core functionality to help beginners learn the essential concepts without 
 * complexity that might cause confusion.
 * 
 * LEARNING OBJECTIVES:
 * - Understand I2C communication protocols with RTC modules
 * - Learn about precision timing and clock generation
 * - Practice oscilloscope signal measurement techniques
 * - Explore frequency synthesis and signal generation
 * - Implement real-time system monitoring and control
 * 
 * HARDWARE INFORMATION:
 * =====================
 * ESP32-S3-DevKitC-1 v1.1 Specifications:
 * - Dual-core Xtensa LX7 processor (240 MHz)
 * - 16 MB Flash memory, 16 MB PSRAM
 * - Native USB support via GPIO19/20
 * - I2C0: SDA=GPIO21, SCL=GPIO22 (default)
 * - Operating voltage: 3.3V (5V tolerant on some pins)
 * 
 * DS3231 RTC Module Specifications:
 * - High-precision temperature-compensated crystal oscillator
 * - I2C communication interface (3.3V/5V compatible)
 * - Programmable square wave output: 1Hz, 1.024kHz, 4.096kHz, 8.192kHz
 * - 32.768kHz crystal oscillator output available
 * - Battery backup capability for timekeeping
 * - Temperature sensor with ±3°C accuracy
 * 
 * CIRCUIT DIAGRAM:
 * ================
 *                    ESP32-S3-DevKitC-1              DS3231 RTC Module
 *                  ┌─────────────────────┐         ┌─────────────────────┐
 *                  │                     │         │                     │
 *              5V  │ 5V              3V3 │────────▶│ VCC                 │
 *             GND  │ GND             GND │────────▶│ GND                 │
 *                  │                     │         │                     │
 *                  │ GPIO21 (SDA)        │◄───────▶│ SDA                 │
 *                  │ GPIO22 (SCL)        │────────▶│ SCL                 │
 *                  │                     │         │                     │
 *                  │                     │         │ SQW ────┐           │
 *                  │                     │         │ 32K ────┼───┐       │
 *                  └─────────────────────┘         └─────────┼───┼───────┘
 *                                                            │   │
 *                                                   ┌────────┼───┼────────┐
 *                                                   │ Oscilloscope        │
 *                                                   │ Ch1: SQW signal     │
 *                                                   │ Ch2: 32K signal     │
 *                                                   │ GND: Common ground  │
 *                                                   └─────────────────────┘
 * 
 * COMPONENTS:
 * ===========
 * 1x ESP32-S3-DevKitC-1 v1.1 development board
 * 1x DS3231 RTC breakout module (with battery backup)
 * 4x Jumper wires (male-to-female)
 * 1x Breadboard (optional, for organized connections)
 * 1x Oscilloscope (for signal measurement and analysis)
 * 1x CR2032 battery (for RTC backup power)
 * 
 * Part Numbers and Sources:
 * - DS3231 RTC: Adafruit #3013, SparkFun #BOB-13314
 * - Jumper wires: Standard 0.1" pitch connector wires
 * - Oscilloscope: Any scope with 2+ channels, 20MHz+ bandwidth
 * 
 * GPIO ASSIGNMENTS:
 * =================
 * ESP32-S3 Pin    | Function        | DS3231 Pin    | Description
 * ----------------|-----------------|---------------|---------------------------
 * 5V              | Power Supply    | VCC           | 5V power (regulated on module)
 * GND             | Ground          | GND           | Common ground reference
 * GPIO21 (SDA)    | I2C Data        | SDA           | Serial data line (pull-up)
 * GPIO22 (SCL)    | I2C Clock       | SCL           | Serial clock line (pull-up)
 * N/C             | Square Wave     | SQW           | Programmable frequency output
 * N/C             | 32kHz Output    | 32K           | Fixed 32.768kHz crystal output
 * 
 * TECHNICAL NOTES:
 * ================
 * 1. I2C Pull-up Resistors: DS3231 modules typically include 10kΩ pull-ups
 * 2. Power Supply: Use 5V for better noise immunity, 3.3V also supported
 * 3. Signal Levels: SQW and 32K outputs are 3.3V CMOS compatible
 * 4. Frequency Accuracy: ±2ppm typical (±63 seconds/year maximum drift)
 * 5. Temperature Compensation: Automatic adjustment from -40°C to +85°C
 * 6. Battery Backup: Maintains timekeeping and square wave during power loss
 * 
 * OSCILLOSCOPE SETTINGS:
 * - Time base: 100µs/div for 8kHz, 500µs/div for 1kHz
 * - Voltage: 1V/div, AC coupling for square waves
 * - Trigger: Rising edge, auto mode
 * - Probe: 1x setting for direct connection
 * 
 * IDE REQUIREMENTS:
 * =================
 * - Arduino IDE 2.0+
 * - ESP32 Arduino Core v3.0+
 * - Board: "ESP32S3 Dev Module"
 * - USB CDC On Boot: "Enabled" (for native USB serial)
 * - Flash Size: "16MB"
 * - PSRAM: "OPI PSRAM"
 * 
 * LIBRARIES REQUIRED:
 * - RtcDS3231 by Makuna (latest version)
 * - Wire (built-in I2C library)
 * 
 * IMPORTANT LIBRARY NOTES:
 * - The RtcDS3231 library API may vary between versions
 * - Ensure you have the latest version installed from Library Manager
 * - If compilation errors occur, check library documentation for API changes
 * 
 * Created: April 3, 2019 by Peter Dalmaris
 * Updated: January 2025 for ESP32-S3 compatibility and educational enhancement
 */

// ===========================================================================================
// LIBRARY INCLUDES
// ===========================================================================================

#include <Wire.h>        // ESP32 I2C communication library
#include <RtcDS3231.h>   // DS3231 RTC library by Makuna

// ===========================================================================================
// CONFIGURATION
// ===========================================================================================

RtcDS3231<TwoWire> rtc(Wire);  // Create RTC object using hardware I2C

// Available square wave frequencies
const DS3231SquareWaveClock frequencies[] = {
    DS3231SquareWaveClock_1Hz,      // 1 Hz
    DS3231SquareWaveClock_1kHz,     // 1.024 kHz  
    DS3231SquareWaveClock_4kHz,     // 4.096 kHz
    DS3231SquareWaveClock_8kHz      // 8.192 kHz
};

const char* frequencyNames[] = {
    "1 Hz",
    "1.024 kHz",
    "4.096 kHz", 
    "8.192 kHz"
};

int currentFrequency = 0;           // Current frequency index
unsigned long lastChange = 0;       // Last frequency change time

// ===========================================================================================
// ARDUINO SETUP FUNCTION
// ===========================================================================================

void setup() {
    // Start serial communication
    Serial.begin(115200);
    delay(1000);  // Wait for serial to initialize
    
    Serial.println("ESP32-S3 DS3231 Square Wave Generator");
    Serial.println("=====================================");
    
    // Initialize RTC
    Serial.println("Initializing DS3231 RTC...");
    rtc.Begin();
    
    // Start RTC if not running
    if (!rtc.GetIsRunning()) {
        Serial.println("Starting RTC...");
        rtc.SetIsRunning(true);
    }
    
    // Enable 32kHz output pin (optional)
    rtc.Enable32kHzPin(true);
    Serial.println("32kHz output enabled");
    
    // Configure square wave output
    rtc.SetSquareWavePin(DS3231SquareWavePin_ModeClock);
    
    // Set initial frequency (8kHz)
    rtc.SetSquareWavePinClockFrequency(frequencies[currentFrequency]);
    Serial.print("Initial frequency: ");
    Serial.println(frequencyNames[currentFrequency]);
    
    Serial.println();
    Serial.println("OSCILLOSCOPE SETUP:");
    Serial.println("- Connect probe to DS3231 SQW pin");
    Serial.println("- Connect ground to common GND");
    Serial.println("- Set timebase to 50μs/div for 8kHz");
    Serial.println();
    Serial.println("Frequency changes every 3 seconds...");
    
    delay(2000);  // Initial delay
}

// ===========================================================================================
// ARDUINO MAIN LOOP FUNCTION
// ===========================================================================================

void loop() {
    // Change frequency every 3 seconds
    if (millis() - lastChange >= 3000) {
        // Move to next frequency
        currentFrequency = (currentFrequency + 1) % 4;
        
        // Update RTC frequency
        rtc.SetSquareWavePinClockFrequency(frequencies[currentFrequency]);
        
        // Print current frequency
        Serial.print("Frequency changed to: ");
        Serial.println(frequencyNames[currentFrequency]);
        
        // Provide oscilloscope guidance
        if (currentFrequency == 0) {        // 1 Hz
            Serial.println("  Oscilloscope: 500ms/div");
        } else if (currentFrequency == 1) { // 1kHz
            Serial.println("  Oscilloscope: 500μs/div");
        } else if (currentFrequency == 2) { // 4kHz
            Serial.println("  Oscilloscope: 100μs/div");
        } else {                            // 8kHz
            Serial.println("  Oscilloscope: 50μs/div");
        }
        
        lastChange = millis();  // Update last change time
    }
    
    delay(100);  // Small delay for stability
}
