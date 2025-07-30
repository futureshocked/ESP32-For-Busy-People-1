/*  09.031 - Advanced Square Wave Generation with DS3231 RTC for ESP32-S3
 * 
 * Course: IoT Development with ESP32-S3
 * Target Hardware: ESP32-S3-DevKitC-1 v1.1
 * Module: ESP32-S3-WROOM-1-N16R16V
 * 
 * This comprehensive sketch demonstrates advanced square wave generation using the DS3231 
 * real-time clock module with the ESP32-S3. The DS3231 can generate precise square wave 
 * signals at multiple frequencies, making it useful for timing applications, clock 
 * generation, and frequency reference circuits.
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
 * Created: April 3, 2019 by Peter Dalmaris
 * Updated: January 2025 for ESP32-S3 compatibility and educational enhancement
 */

// ===========================================================================================
// LIBRARY INCLUDES AND DEPENDENCIES
// ===========================================================================================

#include <Wire.h>        // ESP32 I2C communication library
#include <RtcDS3231.h>   // DS3231 RTC library by Makuna

// ===========================================================================================
// CONFIGURATION CONSTANTS
// ===========================================================================================

// I2C Configuration for ESP32-S3
constexpr uint8_t I2C_SDA_PIN = 21;         // Default SDA pin for ESP32-S3
constexpr uint8_t I2C_SCL_PIN = 22;         // Default SCL pin for ESP32-S3
constexpr uint32_t I2C_FREQUENCY = 100000;  // Standard I2C frequency (100kHz)

// Serial Communication Settings
constexpr uint32_t SERIAL_BAUD_RATE = 115200;  // Modern standard baud rate
constexpr uint16_t SERIAL_TIMEOUT_MS = 5000;   // Serial initialization timeout

// Timing Configuration
constexpr uint16_t FREQUENCY_CHANGE_INTERVAL_MS = 3000;  // Time between frequency changes
constexpr uint16_t STARTUP_DELAY_MS = 2000;              // Initial startup delay
constexpr uint16_t STATUS_UPDATE_INTERVAL_MS = 1000;     // Status message interval

// Square Wave Frequency Options (DS3231 supported frequencies)
constexpr uint8_t NUM_FREQUENCIES = 4;
const DS3231SquareWaveClock AVAILABLE_FREQUENCIES[NUM_FREQUENCIES] = {
    DS3231SquareWaveClock_1Hz,      // 1 Hz
    DS3231SquareWaveClock_1kHz,     // 1.024 kHz
    DS3231SquareWaveClock_4kHz,     // 4.096 kHz  
    DS3231SquareWaveClock_8kHz      // 8.192 kHz
};

const char* FREQUENCY_NAMES[NUM_FREQUENCIES] = {
    "1 Hz",
    "1.024 kHz", 
    "4.096 kHz",
    "8.192 kHz"
};

// ===========================================================================================
// GLOBAL VARIABLES AND OBJECTS
// ===========================================================================================

RtcDS3231<TwoWire> rtc(Wire);  // RTC object using hardware I2C

uint8_t currentFrequencyIndex = 0;           // Current frequency array index
uint32_t lastFrequencyChange = 0;            // Timestamp of last frequency change  
uint32_t lastStatusUpdate = 0;               // Timestamp of last status message
bool rtcInitialized = false;                // RTC initialization status flag
bool enable32kHzOutput = true;              // Enable/disable 32kHz output pin

// Performance monitoring variables
uint32_t setupStartTime = 0;
uint32_t setupDuration = 0;

// ===========================================================================================
// UTILITY AND HELPER FUNCTIONS
// ===========================================================================================

/**
 * Print a formatted separator line for better serial output readability
 * @param character Character to use for the separator
 * @param length Number of characters in the separator
 */
void printSeparator(char character = '=', uint8_t length = 60) {
    for (uint8_t i = 0; i < length; i++) {
        Serial.print(character);
    }
    Serial.println();
}

/**
 * Print system information and configuration details
 */
void printSystemInfo() {
    printSeparator('=');
    Serial.println("ESP32-S3 DS3231 RTC Square Wave Generator");
    Serial.println("Course: IoT Development with ESP32-S3");
    printSeparator('-');
    
    Serial.print("ESP32-S3 Chip Model: ");
    Serial.println(ESP.getChipModel());
    Serial.print("Chip Revision: ");
    Serial.println(ESP.getChipRevision());
    Serial.print("CPU Frequency: ");
    Serial.print(ESP.getCpuFreqMHz());
    Serial.println(" MHz");
    Serial.print("Flash Size: ");
    Serial.print(ESP.getFlashChipSize() / (1024 * 1024));
    Serial.println(" MB");
    Serial.print("PSRAM Size: ");
    Serial.print(ESP.getPsramSize() / (1024 * 1024));
    Serial.println(" MB");
    
    printSeparator('-');
    Serial.print("I2C SDA Pin: GPIO");
    Serial.println(I2C_SDA_PIN);
    Serial.print("I2C SCL Pin: GPIO");
    Serial.println(I2C_SCL_PIN);
    Serial.print("I2C Frequency: ");
    Serial.print(I2C_FREQUENCY / 1000);
    Serial.println(" kHz");
    printSeparator('=');
}

/**
 * Initialize and validate RTC connection
 * @return true if RTC is successfully initialized, false otherwise
 */
bool initializeRTC() {
    Serial.println("Initializing DS3231 RTC...");
    
    // Begin RTC communication
    rtc.Begin();
    
    // Check if RTC is responsive
    if (!rtc.IsDateTimeValid()) {
        Serial.println("WARNING: RTC date/time may be invalid!");
        Serial.println("This could indicate:");
        Serial.println("  - First time setup required");  
        Serial.println("  - Dead backup battery");
        Serial.println("  - Connection issues");
    }
    
    // Ensure RTC is running
    if (!rtc.GetIsRunning()) {
        Serial.println("RTC was not actively running, starting now...");
        rtc.SetIsRunning(true);
        delay(100); // Allow time for RTC to start
        
        if (!rtc.GetIsRunning()) {
            Serial.println("ERROR: Failed to start RTC!");
            return false;
        }
        Serial.println("RTC started successfully.");
    } else {
        Serial.println("RTC is already running.");
    }
    
    // Configure square wave output
    Serial.println("Configuring square wave output...");
    
    // Enable or disable 32kHz output pin
    rtc.Enable32kHzPin(enable32kHzOutput);
    Serial.print("32kHz output pin: ");
    Serial.println(enable32kHzOutput ? "ENABLED" : "DISABLED");
    
    // Set square wave pin to clock mode
    rtc.SetSquareWavePin(DS3231SquareWavePin_ModeClock);
    Serial.println("Square wave pin set to clock mode.");
    
    // Set initial frequency
    rtc.SetSquareWavePinClockFrequency(AVAILABLE_FREQUENCIES[currentFrequencyIndex]);
    Serial.print("Initial square wave frequency: ");
    Serial.println(FREQUENCY_NAMES[currentFrequencyIndex]);
    
    return true;
}

/**
 * Change the square wave frequency to the next available option
 */
void changeSquareWaveFrequency() {
    currentFrequencyIndex = (currentFrequencyIndex + 1) % NUM_FREQUENCIES;
    
    rtc.SetSquareWavePinClockFrequency(AVAILABLE_FREQUENCIES[currentFrequencyIndex]);
    
    Serial.print("Square wave frequency changed to: ");
    Serial.println(FREQUENCY_NAMES[currentFrequencyIndex]);
    
    // Provide oscilloscope setup guidance
    if (currentFrequencyIndex == 0) {  // 1 Hz
        Serial.println("  Oscilloscope: Set timebase to 500ms/div");
    } else if (currentFrequencyIndex == 1) {  // 1kHz  
        Serial.println("  Oscilloscope: Set timebase to 500μs/div");
    } else if (currentFrequencyIndex == 2) {  // 4kHz
        Serial.println("  Oscilloscope: Set timebase to 100μs/div");
    } else {  // 8kHz
        Serial.println("  Oscilloscope: Set timebase to 50μs/div");
    }
}

/**
 * Read and display RTC temperature (diagnostic feature)
 */
void displayRTCTemperature() {
    RtcTemperature temperature = rtc.GetTemperature();
    
    // Convert temperature to float and check if it's within reasonable bounds
    float tempC = temperature.AsFloatDegC();
    
    // DS3231 temperature range is typically -40°C to +85°C
    if (tempC >= -40.0 && tempC <= 85.0) {
        Serial.print("RTC Temperature: ");
        Serial.print(tempC, 2);
        Serial.println("°C");
    } else {
        Serial.println("Warning: Unable to read valid RTC temperature");
        Serial.print("Raw temperature value: ");
        Serial.println(tempC);
    }
}

/**
 * Print current RTC status and configuration
 */
void printRTCStatus() {
    printSeparator('-', 40);
    Serial.println("RTC STATUS REPORT");
    printSeparator('-', 40);
    
    Serial.print("Current Frequency: ");
    Serial.println(FREQUENCY_NAMES[currentFrequencyIndex]);
    
    Serial.print("32kHz Output: ");
    Serial.println(enable32kHzOutput ? "ENABLED" : "DISABLED");
    
    Serial.print("RTC Running: ");
    Serial.println(rtc.GetIsRunning() ? "YES" : "NO");
    
    displayRTCTemperature();
    
    Serial.print("Uptime: ");
    Serial.print(millis() / 1000);
    Serial.println(" seconds");
    
    printSeparator('-', 40);
}

/**
 * Handle any RTC errors or warnings
 */
void checkRTCHealth() {
    if (!rtc.GetIsRunning()) {
        Serial.println("ERROR: RTC has stopped running!");
        Serial.println("Attempting to restart...");
        rtc.SetIsRunning(true);
        delay(100);
        
        if (rtc.GetIsRunning()) {
            Serial.println("RTC restart successful.");
        } else {
            Serial.println("CRITICAL: Unable to restart RTC!");
        }
    }
}

// ===========================================================================================
// ARDUINO SETUP FUNCTION
// ===========================================================================================

void setup() {
    setupStartTime = millis();
    
    // Initialize serial communication with timeout
    Serial.begin(SERIAL_BAUD_RATE);
    uint32_t serialStartTime = millis();
    while (!Serial && (millis() - serialStartTime < SERIAL_TIMEOUT_MS)) {
        delay(10); // Wait for native USB serial connection
    }
    
    // Print system information
    delay(1000); // Ensure serial is ready
    printSystemInfo();
    
    // Initialize I2C with custom pins and frequency
    Serial.println("Initializing I2C communication...");
    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
    Wire.setClock(I2C_FREQUENCY);
    Serial.println("I2C initialized successfully.");
    
    // Initialize RTC
    rtcInitialized = initializeRTC();
    
    if (rtcInitialized) {
        Serial.println("RTC initialization completed successfully!");
        
        // Initial status display
        printRTCStatus();
        
        // Provide measurement instructions
        Serial.println();
        Serial.println("MEASUREMENT SETUP:");
        Serial.println("==================");
        Serial.println("1. Connect oscilloscope probe to DS3231 SQW pin");
        Serial.println("2. Connect oscilloscope ground to common GND");
        Serial.println("3. Set oscilloscope to AC coupling");
        Serial.println("4. Adjust timebase according to frequency");
        if (enable32kHzOutput) {
            Serial.println("5. Optional: Connect second probe to 32K pin");
        }
        Serial.println();
        
    } else {
        Serial.println("CRITICAL ERROR: RTC initialization failed!");
        Serial.println("Please check:");
        Serial.println("  - Wiring connections (SDA, SCL, VCC, GND)");
        Serial.println("  - DS3231 module power supply");
        Serial.println("  - I2C pull-up resistors");
        Serial.println();
        Serial.println("System will continue with limited functionality.");
    }
    
    // Record setup completion time
    setupDuration = millis() - setupStartTime;
    Serial.print("Setup completed in ");
    Serial.print(setupDuration);
    Serial.println(" milliseconds.");
    
    printSeparator('=');
    Serial.println("Starting square wave generation...");
    Serial.println("Frequency will change every 3 seconds.");
    Serial.println("Status updates every 1 second.");
    printSeparator('=');
    
    delay(STARTUP_DELAY_MS);
}

// ===========================================================================================
// ARDUINO MAIN LOOP FUNCTION  
// ===========================================================================================

void loop() {
    uint32_t currentTime = millis();
    
    // Only proceed if RTC is properly initialized
    if (!rtcInitialized) {
        Serial.println("Waiting for RTC initialization...");
        delay(1000);
        return;
    }
    
    // Check RTC health periodically
    checkRTCHealth();
    
    // Change frequency at specified intervals
    if (currentTime - lastFrequencyChange >= FREQUENCY_CHANGE_INTERVAL_MS) {
        changeSquareWaveFrequency();
        lastFrequencyChange = currentTime;
    }
    
    // Print status updates at specified intervals
    if (currentTime - lastStatusUpdate >= STATUS_UPDATE_INTERVAL_MS) {
        printRTCStatus();
        lastStatusUpdate = currentTime;
    }
    
    // Small delay to prevent overwhelming the processor
    delay(10);
}
