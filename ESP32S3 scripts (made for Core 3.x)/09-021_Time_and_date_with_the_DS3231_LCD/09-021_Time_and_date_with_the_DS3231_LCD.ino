/*  09.021 - DS3231 Real-Time Clock with I2C LCD Display for ESP32-S3
 * 
 * Course: IoT Development with ESP32-S3
 * Target Hardware: ESP32-S3-DevKitC-1 v1.1
 * Module: ESP32-S3-WROOM-1-N16R16V
 * 
 * This comprehensive sketch demonstrates interfacing with the DS3231 Real-Time Clock
 * module and displaying time, date, and temperature on an I2C LCD. The DS3231 provides
 * high-precision timekeeping with battery backup capability and includes an integrated
 * temperature sensor for drift compensation.
 * 
 * LEARNING OBJECTIVES:
 * ===================
 * - Understanding I2C communication protocol
 * - Real-time clock concepts and battery backup systems
 * - LCD display interfacing and formatting
 * - Error handling in embedded systems
 * - Temperature sensor integration
 * - Memory management with string formatting
 * 
 * HARDWARE INFORMATION:
 * =====================
 * Target Board: ESP32-S3-DevKitC-1 v1.1
 * Microcontroller: ESP32-S3-WROOM-1-N16R16V
 * Flash Memory: 16 MB
 * PSRAM: 16 MB
 * I2C Interface: Hardware I2C (Wire library)
 * Default I2C Pins: GPIO21 (SDA), GPIO22 (SCL)
 * Power Supply: 5V capable for LCD and RTC modules
 * 
 * CIRCUIT DIAGRAM:
 * ================
 * 
 *                    ESP32-S3-DevKitC-1 v1.1
 *                    ┌─────────────────────┐
 *                    │                     │
 *     5V ────────────┤ 5V               21 ├──────── SDA (Pull-up via module)
 *    GND ────────────┤ GND              22 ├──────── SCL (Pull-up via module)
 *                    │                     │
 *                    └─────────────────────┘
 *                              │
 *                              │
 *    ┌─────────────────────────┼─────────────────────────┐
 *    │                         │                         │
 *    │     DS3231 RTC          │        I2C LCD          │
 *    │   ┌──────────────┐      │      ┌──────────────┐   │
 *    │   │     VCC ─────┼──────┼──────┤ VCC          │   │
 *    │   │     GND ─────┼──────┼──────┤ GND          │   │
 *    │   │     SDA ─────┼──────┼──────┤ SDA          │   │
 *    │   │     SCL ─────┼──────┼──────┤ SCL          │   │
 *    │   │     SQW      │      │      │              │   │
 *    │   │     32K      │      │      │              │   │
 *    │   └──────────────┘      │      └──────────────┘   │
 *    │                         │                         │
 *    └─────────────────────────┴─────────────────────────┘
 * 
 * COMPONENTS:
 * ===========
 * 1. ESP32-S3-DevKitC-1 v1.1 Development Board
 * 2. DS3231 RTC Module (with battery backup)
 *    - High precision temperature-compensated crystal oscillator
 *    - ±2ppm accuracy from 0°C to +40°C
 *    - Integrated temperature sensor
 *    - Battery backup capability (CR2032 recommended)
 *    - I2C interface (address: 0x68)
 * 3. I2C LCD Display (16x2 characters)
 *    - HD44780 compatible controller with I2C backpack
 *    - PCF8574 I2C expander (typical address: 0x27 or 0x3F)
 *    - 5V operation (with voltage regulator on module)
 * 4. Connecting wires (male-to-male jumper wires)
 * 5. Breadboard (optional, for organized connections)
 * 
 * GPIO ASSIGNMENTS:
 * =================
 * GPIO21 (SDA)  - I2C Data Line (both RTC and LCD)
 * GPIO22 (SCL)  - I2C Clock Line (both RTC and LCD)
 * 5V            - Power supply for LCD and RTC modules
 * GND           - Common ground reference
 * 
 * TECHNICAL NOTES:
 * ================
 * 1. The DS3231 operates on both 3.3V and 5V supply voltages
 * 2. I2C pull-up resistors are typically integrated on the modules
 * 3. The ESP32-S3 I2C interface is 5V tolerant when using external pull-ups
 * 4. Battery backup maintains timekeeping during power loss
 * 5. Temperature readings are used internally for crystal compensation
 * 6. The RTC maintains accuracy within ±2ppm at room temperature
 * 
 * TROUBLESHOOTING:
 * ================
 * - If LCD doesn't display: Check I2C address (try 0x3F if 0x27 fails)
 * - If time resets: Ensure DS3231 has a working battery installed
 * - If communication errors: Verify wiring and I2C pull-ups
 * - If temperature readings seem off: Allow 10+ seconds for sensor stabilization
 * 
 * IDE REQUIREMENTS:
 * =================
 * - Arduino IDE 2.0+
 * - ESP32 Arduino Core v3.0+
 * - Board: "ESP32S3 Dev Module"
 * - Libraries: RtcDS3231, Wire, LiquidCrystal_I2C
 * 
 * Created: April 3, 2019 by Peter Dalmaris
 * Updated: December 2024 for ESP32-S3 compatibility and educational enhancement
 */

#include <Wire.h>           // I2C communication library
#include <RtcDS3231.h>      // DS3231 RTC library by Makuna
#include <LiquidCrystal_I2C.h>  // I2C LCD library

// =====================================================
// HARDWARE CONFIGURATION CONSTANTS
// =====================================================

// I2C LCD Configuration
const uint8_t LCD_I2C_ADDRESS = 0x27;    // Common I2C address (try 0x3F if this fails)
const uint8_t LCD_COLUMNS = 16;          // LCD width in characters
const uint8_t LCD_ROWS = 2;              // LCD height in rows

// Update intervals
const unsigned long DISPLAY_UPDATE_INTERVAL = 10000;  // 10 seconds in milliseconds
const unsigned long SERIAL_OUTPUT_INTERVAL = 1000;    // 1 second for serial debug

// String buffer sizes
const uint8_t DATE_STRING_SIZE = 20;
const uint8_t TIME_STRING_SIZE = 20;
const uint8_t DATETIME_STRING_SIZE = 30;

// =====================================================
// GLOBAL OBJECTS AND VARIABLES
// =====================================================

// Initialize RTC object using default I2C (Wire)
RtcDS3231<TwoWire> rtc(Wire);

// Initialize LCD object
LiquidCrystal_I2C lcd(LCD_I2C_ADDRESS, LCD_COLUMNS, LCD_ROWS);

// Timing variables for non-blocking updates
unsigned long lastDisplayUpdate = 0;
unsigned long lastSerialOutput = 0;
unsigned long systemStartTime = 0;

// Status tracking variables
bool rtcInitialized = false;
bool lcdInitialized = false;
uint32_t errorCount = 0;

// =====================================================
// HELPER FUNCTION DECLARATIONS
// =====================================================

void initializeHardware();
void initializeRTC();
void initializeLCD();
void updateDisplay();
void updateSerialOutput();
void updateLCDDisplay(const RtcDateTime& dt, RtcTemperature temp);
void formatDateString(const RtcDateTime& dt, char* buffer, size_t bufferSize);
void formatTimeString(const RtcDateTime& dt, char* buffer, size_t bufferSize);
void formatDateTimeString(const RtcDateTime& dt, char* buffer, size_t bufferSize);
void displaySystemStatus();
void handleRTCError();
void printSystemInfo();

// =====================================================
// ARDUINO SETUP FUNCTION
// =====================================================

void setup() {
    // Initialize serial communication for debugging
    Serial.begin(115200);
    delay(1000);  // Allow serial monitor to connect
    
    // Record system start time
    systemStartTime = millis();
    
    // Print startup banner
    Serial.println("\n" + String("=").substring(0, 60));
    Serial.println("ESP32-S3 DS3231 RTC with I2C LCD Demo");
    Serial.println("Course: IoT Development with ESP32-S3");
    Serial.println("Target: ESP32-S3-DevKitC-1 v1.1");
    Serial.println(String("=").substring(0, 60));
    
    // Print compilation information
    Serial.printf("Compiled: %s %s\n", __DATE__, __TIME__);
    Serial.printf("ESP32-S3 Chip Revision: %d\n", ESP.getChipRevision());
    Serial.printf("CPU Frequency: %d MHz\n", ESP.getCpuFreqMHz());
    Serial.printf("Free Heap: %d bytes\n", ESP.getFreeHeap());
    Serial.println();
    
    // Initialize all hardware components
    initializeHardware();
    
    // Display initial system status
    displaySystemStatus();
    
    Serial.println("Setup complete. Starting main loop...\n");
}

// =====================================================
// ARDUINO MAIN LOOP FUNCTION
// =====================================================

void loop() {
    unsigned long currentTime = millis();
    
    // Update display at specified interval
    if (currentTime - lastDisplayUpdate >= DISPLAY_UPDATE_INTERVAL) {
        updateDisplay();
        lastDisplayUpdate = currentTime;
    }
    
    // Update serial output at specified interval
    if (currentTime - lastSerialOutput >= SERIAL_OUTPUT_INTERVAL) {
        updateSerialOutput();
        lastSerialOutput = currentTime;
    }
    
    // Small delay to prevent watchdog triggers
    delay(10);
}

// =====================================================
// HARDWARE INITIALIZATION FUNCTIONS
// =====================================================

void initializeHardware() {
    Serial.println("Initializing hardware components...");
    
    // Initialize I2C bus
    Wire.begin();
    Wire.setClock(100000);  // Set I2C to standard mode (100kHz)
    Serial.println("✓ I2C bus initialized");
    
    // Initialize RTC
    initializeRTC();
    
    // Initialize LCD
    initializeLCD();
    
    Serial.println("Hardware initialization complete.\n");
}

void initializeRTC() {
    Serial.println("Initializing DS3231 RTC...");
    
    try {
        // Begin RTC communication
        rtc.Begin();
        
        // Get compilation time for reference
        RtcDateTime compiledTime = RtcDateTime(__DATE__, __TIME__);
        Serial.print("Compiled DateTime: ");
        char buffer[DATETIME_STRING_SIZE];
        formatDateTimeString(compiledTime, buffer, sizeof(buffer));
        Serial.println(buffer);
        
        // Check if RTC has valid date/time
        if (!rtc.IsDateTimeValid()) {
            if (rtc.LastError() != 0) {
                Serial.printf("⚠ RTC communication error: %d\n", rtc.LastError());
                handleRTCError();
                return;
            } else {
                Serial.println("⚠ RTC lost confidence in DateTime - setting to compile time");
                rtc.SetDateTime(compiledTime);
            }
        }
        
        // Ensure RTC is running
        if (!rtc.GetIsRunning()) {
            Serial.println("⚠ RTC was not running - starting now");
            rtc.SetIsRunning(true);
        }
        
        // Check if RTC time needs updating
        RtcDateTime now = rtc.GetDateTime();
        if (now < compiledTime) {
            Serial.println("⚠ RTC is older than compile time - updating");
            rtc.SetDateTime(compiledTime);
        } else if (now > compiledTime) {
            Serial.println("✓ RTC is newer than compile time (expected)");
        } else {
            Serial.println("ℹ RTC matches compile time exactly");
        }
        
        // Configure RTC settings
        rtc.Enable32kHzPin(false);
        rtc.SetSquareWavePin(DS3231SquareWavePin_ModeNone);
        
        rtcInitialized = true;
        Serial.println("✓ DS3231 RTC initialized successfully");
        
    } catch (...) {
        Serial.println("✗ Failed to initialize DS3231 RTC");
        rtcInitialized = false;
        errorCount++;
    }
}

void initializeLCD() {
    Serial.println("Initializing I2C LCD...");
    
    try {
        // Initialize the LCD
        lcd.init();
        lcd.backlight();
        lcd.clear();
        
        // Test LCD with startup message
        lcd.setCursor(0, 0);
        lcd.print("ESP32-S3 RTC");
        lcd.setCursor(0, 1);
        lcd.print("Initializing...");
        
        lcdInitialized = true;
        Serial.println("✓ I2C LCD initialized successfully");
        
        delay(2000);  // Show startup message for 2 seconds
        lcd.clear();
        
    } catch (...) {
        Serial.println("✗ Failed to initialize I2C LCD");
        Serial.println("  Check I2C address - try 0x3F if 0x27 fails");
        lcdInitialized = false;
        errorCount++;
    }
}

// =====================================================
// DISPLAY UPDATE FUNCTIONS
// =====================================================

void updateDisplay() {
    if (!rtcInitialized) {
        Serial.println("⚠ RTC not initialized - skipping display update");
        return;
    }
    
    // Get current date/time and temperature
    RtcDateTime now = rtc.GetDateTime();
    RtcTemperature temperature = rtc.GetTemperature();
    
    // Check for RTC errors
    if (!rtc.IsDateTimeValid()) {
        Serial.println("⚠ RTC DateTime invalid during update");
        handleRTCError();
        return;
    }
    
    // Update LCD display if available
    if (lcdInitialized) {
        updateLCDDisplay(now, temperature);
    }
    
    // Always update serial output for debugging
    Serial.println("=== Display Update ===");
    char buffer[DATETIME_STRING_SIZE];
    formatDateTimeString(now, buffer, sizeof(buffer));
    Serial.printf("DateTime: %s\n", buffer);
    Serial.print("Temperature: ");
    Serial.print(temperature.AsFloatDegC());
    Serial.println("°C");
    Serial.printf("Free Heap: %d bytes\n", ESP.getFreeHeap());
    Serial.printf("Uptime: %lu seconds\n", (millis() - systemStartTime) / 1000);
    Serial.println();
}

void updateLCDDisplay(const RtcDateTime& dt, RtcTemperature temp) {
    char dateString[DATE_STRING_SIZE];
    char timeString[TIME_STRING_SIZE];
    
    // Format date and time strings
    formatDateString(dt, dateString, sizeof(dateString));
    formatTimeString(dt, timeString, sizeof(timeString));
    
    // Clear display and update content
    lcd.clear();
    
    // Display date on first row
    lcd.setCursor(0, 0);
    lcd.print(dateString);
    
    // Display time on second row
    lcd.setCursor(0, 1);
    lcd.print(timeString);
    
    // Display temperature on second row (right side)
    lcd.setCursor(10, 1);
    lcd.print(temp.AsFloatDegC(), 1);
    lcd.print("C");
}

void updateSerialOutput() {
    if (!rtcInitialized) return;
    
    RtcDateTime now = rtc.GetDateTime();
    RtcTemperature temperature = rtc.GetTemperature();
    
    // Format and print current time
    char buffer[DATETIME_STRING_SIZE];
    formatDateTimeString(now, buffer, sizeof(buffer));
    Serial.print("[");
    Serial.print(buffer);
    Serial.print("] Temp: ");
    Serial.print(temperature.AsFloatDegC());
    Serial.print("°C | Heap: ");
    Serial.print(ESP.getFreeHeap());
    Serial.print(" | Errors: ");
    Serial.println(errorCount);
}

// =====================================================
// STRING FORMATTING HELPER FUNCTIONS
// =====================================================

void formatDateString(const RtcDateTime& dt, char* buffer, size_t bufferSize) {
    snprintf(buffer, bufferSize, "%02u/%02u/%04u", 
             dt.Month(), dt.Day(), dt.Year());
}

void formatTimeString(const RtcDateTime& dt, char* buffer, size_t bufferSize) {
    snprintf(buffer, bufferSize, "%02u:%02u:%02u", 
             dt.Hour(), dt.Minute(), dt.Second());
}

void formatDateTimeString(const RtcDateTime& dt, char* buffer, size_t bufferSize) {
    snprintf(buffer, bufferSize, "%02u/%02u/%04u %02u:%02u:%02u", 
             dt.Month(), dt.Day(), dt.Year(), 
             dt.Hour(), dt.Minute(), dt.Second());
}

// =====================================================
// ERROR HANDLING AND DIAGNOSTIC FUNCTIONS
// =====================================================

void handleRTCError() {
    errorCount++;
    Serial.printf("RTC Error #%d occurred\n", errorCount);
    
    if (lcdInitialized) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("RTC ERROR");
        lcd.setCursor(0, 1);
        lcd.printf("Count: %d", errorCount);
    }
    
    // Attempt to reinitialize RTC after multiple errors
    if (errorCount > 5) {
        Serial.println("Multiple RTC errors - attempting reinitialization");
        delay(1000);
        initializeRTC();
    }
}

void displaySystemStatus() {
    Serial.println("=== SYSTEM STATUS ===");
    Serial.printf("RTC Initialized: %s\n", rtcInitialized ? "YES" : "NO");
    Serial.printf("LCD Initialized: %s\n", lcdInitialized ? "YES" : "NO");
    Serial.printf("Error Count: %d\n", errorCount);
    Serial.printf("I2C Clock Speed: %d Hz\n", Wire.getClock());
    
    if (rtcInitialized) {
        Serial.printf("RTC Running: %s\n", rtc.GetIsRunning() ? "YES" : "NO");
        Serial.printf("RTC Valid: %s\n", rtc.IsDateTimeValid() ? "YES" : "NO");
    }
    
    Serial.println("====================\n");
}

void printSystemInfo() {
    Serial.println("=== ESP32-S3 SYSTEM INFO ===");
    Serial.printf("Chip Model: %s\n", ESP.getChipModel());
    Serial.printf("Chip Revision: %d\n", ESP.getChipRevision());
    Serial.printf("CPU Frequency: %d MHz\n", ESP.getCpuFreqMHz());
    Serial.printf("Flash Size: %d bytes\n", ESP.getFlashChipSize());
    Serial.printf("PSRAM Size: %d bytes\n", ESP.getPsramSize());
    Serial.printf("Free Heap: %d bytes\n", ESP.getFreeHeap());
    Serial.printf("Free PSRAM: %d bytes\n", ESP.getFreePsram());
    Serial.println("=============================\n");
}
