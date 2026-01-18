/*  09.011 - Time and Date with DS3231 RTC for ESP32-S3
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

#include <Wire.h>           // I2C communication library (built-in)
#include <RtcDS3231.h>      // DS3231 RTC library by Makuna

// ============================================================================
// CONFIGURATION CONSTANTS
// ============================================================================

// I2C Pin assignments for ESP32-S3-DevKitC-1
const uint8_t I2C_SDA_PIN = 47;    // Serial Data Line
const uint8_t I2C_SCL_PIN = 48;    // Serial Clock Line

// Timing constants
const uint32_t SERIAL_BAUD_RATE = 115200;     // Serial communication speed
const uint32_t LOOP_DELAY_MS = 10000;         // 10 seconds between readings
const uint32_t I2C_FREQUENCY = 100000;        // Standard I2C speed (100kHz)

// Display formatting
const uint8_t DATE_STRING_LENGTH = 20;        // Buffer size for date string

// ============================================================================
// GLOBAL OBJECTS AND VARIABLES
// ============================================================================

// Create RTC object using custom I2C pins
RtcDS3231<TwoWire> Rtc(Wire);

// Performance monitoring variables
uint32_t loopCounter = 0;
uint32_t startTime = 0;

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

/**
 * Helper macro for array size calculation
 * Safer than manual sizeof calculations
 */
#define countof(a) (sizeof(a) / sizeof(a[0]))

/**
 * Print formatted date and time to serial monitor
 * @param dt RtcDateTime object containing date/time data
 */
void printDateTime(const RtcDateTime& dt) {
    char datestring[DATE_STRING_LENGTH];
    
    // Format: MM/DD/YYYY HH:MM:SS
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

/**
 * Print alternative date format (ISO 8601 style)
 * @param dt RtcDateTime object containing date/time data
 */
void printDateTimeISO(const RtcDateTime& dt) {
    char datestring[DATE_STRING_LENGTH];
    
    // Format: YYYY-MM-DD HH:MM:SS
    snprintf_P(datestring,
               countof(datestring),
               PSTR("%04u-%02u-%02u %02u:%02u:%02u"),
               dt.Year(),
               dt.Month(),
               dt.Day(),
               dt.Hour(),
               dt.Minute(),
               dt.Second());
    
    Serial.print(datestring);
}

/**
 * Initialize I2C communication with custom pins
 * @return true if successful, false otherwise
 */
bool initializeI2C() {
    // Initialize I2C with custom pins for ESP32-S3
    bool success = Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
    
    if (success) {
        Wire.setClock(I2C_FREQUENCY);
        Serial.printf("I2C initialized successfully on SDA=%d, SCL=%d\n", 
                     I2C_SDA_PIN, I2C_SCL_PIN);
        Serial.printf("I2C frequency set to %lu Hz\n", I2C_FREQUENCY);
    } else {
        Serial.println("ERROR: Failed to initialize I2C!");
    }
    
    return success;
}

/**
 * Perform comprehensive RTC diagnostics
 * @return true if RTC is functioning properly
 */
bool performRTCDiagnostics() {
    Serial.println("\n=== RTC DIAGNOSTICS ===");
    
    // Check if RTC is responding
    if (!Rtc.IsDateTimeValid()) {
        if (Rtc.LastError() != 0) {
            Serial.printf("ERROR: RTC communication failed (error code: %d)\n", 
                         Rtc.LastError());
            Serial.println("Possible causes:");
            Serial.println("  - Incorrect wiring");
            Serial.println("  - Faulty DS3231 module");
            Serial.println("  - Power supply issues");
            return false;
        } else {
            Serial.println("WARNING: RTC lost confidence in DateTime");
            Serial.println("This typically happens on first use or low battery");
        }
    }
    
    // Check if RTC oscillator is running
    if (!Rtc.GetIsRunning()) {
        Serial.println("WARNING: RTC oscillator was stopped");
        Serial.println("Starting RTC oscillator now...");
        Rtc.SetIsRunning(true);
    } else {
        Serial.println("✓ RTC oscillator is running");
    }
    
    // Display current RTC status
    Serial.printf("✓ RTC communication OK (error code: %d)\n", Rtc.LastError());
    Serial.printf("✓ DateTime valid: %s\n", 
                 Rtc.IsDateTimeValid() ? "Yes" : "No");
    Serial.printf("✓ Oscillator running: %s\n", 
                 Rtc.GetIsRunning() ? "Yes" : "No");
    
    return true;
}

/**
 * Display helpful system information
 */
void displaySystemInfo() {
    Serial.println("\n=== SYSTEM INFORMATION ===");
    Serial.printf("Chip Model: %s\n", ESP.getChipModel());
    Serial.printf("Chip Revision: %d\n", ESP.getChipRevision());
    Serial.printf("CPU Frequency: %lu MHz\n", ESP.getCpuFreqMHz());
    Serial.printf("Flash Size: %lu MB\n", ESP.getFlashChipSize() / (1024 * 1024));
    Serial.printf("Free Heap: %lu bytes\n", ESP.getFreeHeap());
    Serial.printf("Sketch Size: %lu bytes\n", ESP.getSketchSize());
}

// ============================================================================
// MAIN SETUP FUNCTION
// ============================================================================

void setup() {
    // Initialize serial communication
    Serial.begin(SERIAL_BAUD_RATE);
    delay(2000);  // Allow time for serial monitor to connect
    
    // Display startup banner
    Serial.println("\n" + String('=', 60));
    Serial.println("    ESP32-S3 DS3231 RTC Demonstration");
    Serial.println("    Course: IoT Development with ESP32-S3");
    Serial.println("" + String('=', 60));
    
    // Display compilation information
    Serial.printf("Compiled: %s at %s\n", __DATE__, __TIME__);
    RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);  // Fixed syntax error
    Serial.print("Compile timestamp: ");
    printDateTime(compiled);
    Serial.println();
    
    // Display system information
    displaySystemInfo();
    
    // Initialize I2C communication
    Serial.println("\n=== I2C INITIALIZATION ===");
    if (!initializeI2C()) {
        Serial.println("FATAL ERROR: Cannot proceed without I2C!");
        while (true) {
            delay(1000);  // Halt execution
        }
    }
    
    // Initialize RTC
    Serial.println("\n=== RTC INITIALIZATION ===");
    Rtc.Begin();
    
    // Perform comprehensive diagnostics
    if (!performRTCDiagnostics()) {
        Serial.println("WARNING: RTC diagnostics failed!");
        Serial.println("Attempting to continue anyway...");
    }
    
    // Handle initial time setting
    Serial.println("\n=== TIME SYNCHRONIZATION ===");
    RtcDateTime now = Rtc.GetDateTime();
    
    if (!Rtc.IsDateTimeValid() || now < compiled) {
        Serial.println("Setting RTC to compilation time...");
        Rtc.SetDateTime(compiled);
        Serial.print("RTC set to: ");
        printDateTime(compiled);
        Serial.println();
    } else if (now > compiled) {
        Serial.println("✓ RTC time is newer than compile time (expected)");
    } else {
        Serial.println("! RTC time matches compile time exactly (unusual but OK)");
    }
    
    // Configure RTC features
    Serial.println("\n=== RTC CONFIGURATION ===");
    Rtc.Enable32kHzPin(false);  // Disable 32kHz output to save power
    Rtc.SetSquareWavePin(DS3231SquareWavePin_ModeNone);  // Disable square wave
    Serial.println("✓ 32kHz output disabled");
    Serial.println("✓ Square wave output disabled");
    
    // Record start time for performance monitoring
    startTime = millis();
    
    Serial.println("\n=== STARTING MAIN LOOP ===");
    Serial.println("Format: MM/DD/YYYY HH:MM:SS | Temperature | Loop#");
    Serial.println(String('-', 60));
}

// ============================================================================
// MAIN LOOP FUNCTION
// ============================================================================

void loop() {
    loopCounter++;
    
    // Read current date/time from RTC
    RtcDateTime now = Rtc.GetDateTime();
    
    // Check for communication errors
    if (Rtc.LastError() != 0) {
        Serial.printf("ERROR: RTC communication failed (error: %d)\n", 
                     Rtc.LastError());
        delay(LOOP_DELAY_MS);
        return;
    }
    
    // Display standard format
    Serial.print("Standard: ");
    printDateTime(now);
    
    // Display ISO format
    Serial.print(" | ISO: ");
    printDateTimeISO(now);
    
    // Read and display temperature
    RtcTemperature temp = Rtc.GetTemperature();
    Serial.print(" | ");
    temp.Print(Serial);
    Serial.print("C");
    
    // Alternative temperature display as float
    float tempFloat = temp.AsFloatDegC();
    Serial.printf(" (%.2f°C)", tempFloat);
    
    // Display loop counter and uptime
    uint32_t uptime = (millis() - startTime) / 1000;
    Serial.printf(" | Loop: %lu | Uptime: %lu s", loopCounter, uptime);
    
    Serial.println();
    
    // Performance monitoring every 10 loops
    if (loopCounter % 10 == 0) {
        Serial.printf("Performance: Free heap: %lu bytes\n", ESP.getFreeHeap());
    }
    
    // Wait before next reading
    delay(LOOP_DELAY_MS);
}
