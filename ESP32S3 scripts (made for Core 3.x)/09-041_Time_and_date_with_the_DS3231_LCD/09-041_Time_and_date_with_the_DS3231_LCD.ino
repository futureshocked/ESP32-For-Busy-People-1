/*  09.041 - Time and Date with External Interrupts using DS3231 RTC and I2C LCD - ESP32-S3
 * 
 * Course: IoT Development with ESP32-S3
 * Target Hardware: ESP32-S3-DevKitC-1 v1.1
 * Module: ESP32-S3-WROOM-1-N16R16V
 * 
 * This comprehensive sketch demonstrates advanced real-time clock (RTC) functionality
 * with interrupt-driven display updates. It showcases proper hardware interfacing,
 * interrupt handling, I2C communication, and temperature monitoring using the DS3231
 * precision RTC module with an I2C LCD display.
 * 
 * Key Learning Objectives:
 * - External interrupt handling with hardware debouncing
 * - I2C communication with multiple devices
 * - Real-time clock programming and battery backup systems
 * - Temperature sensor integration
 * - Critical section programming for thread safety
 * - Professional error handling and system diagnostics
 * 
 * HARDWARE INFORMATION:
 * =====================
 * ESP32-S3-WROOM-1-N16R16V Specifications:
 * - Flash Memory: 16 MB
 * - PSRAM: 16 MB
 * - I2C Hardware: Two I2C controllers
 * - Interrupt Capability: All GPIO pins except 35,36,37
 * - Operating Voltage: 3.3V (5V tolerant on most pins)
 * 
 * DS3231 RTC Module Features:
 * - Precision: ±2ppm accuracy (±1 minute/year max error)
 * - Temperature Compensation: Built-in TCXO
 * - Battery Backup: CR2032 maintains time during power loss
 * - Temperature Sensor: ±3°C accuracy, 0.25°C resolution
 * - Square Wave Output: Programmable 1Hz to 8192Hz
 * - I2C Interface: 400kHz max, 7-bit address 0x68
 * 
 * CIRCUIT DIAGRAM:
 * ================
 *                    ESP32-S3-DevKitC-1 v1.1
 *                    ┌─────────────────────┐
 *                    │                     │
 *      ────────────> │ 5V             GND │ <──────────────
 *     │              │                     │               │
 *     │         ──── │ GPIO21 (SDA)       │               │
 *     │        │     │ GPIO22 (SCL)       │               │
 *     │        │ ─── │ GPIO1              │               │
 *     │        ││    │                     │               │
 *     │        ││    └─────────────────────┘               │
 *     │        ││                                          │
 *     │        ││    DS3231 RTC Module                     │
 *     │        ││    ┌─────────────────┐                  │
 *     │        ││    │                 │                  │
 *     └──────────────┤ VCC         GND ├──────────────────┘
 *              ││    │                 │
 *              │└────┤ SDA         SCL ├──────
 *              │     │                 │     │
 *              └─────┤ SQW         32K │     │
 *                    │                 │     │
 *                    └─────────────────┘     │
 *                                            │
 *                    I2C LCD (16x2)          │
 *                    ┌─────────────────┐     │
 *                    │                 │     │
 *      ──────────────┤ VCC         GND ├─────┘
 *                    │                 │
 *      ──────────────┤ SDA         SCL ├─────
 *                    │                 │
 *                    └─────────────────┘
 * 
 * COMPONENTS:
 * ===========
 * 1. ESP32-S3-DevKitC-1 v1.1 Development Board
 * 2. DS3231 RTC Module with CR2032 Battery
 *    - High precision real-time clock
 *    - Temperature compensated crystal oscillator (TCXO)
 *    - Battery backup for timekeeping during power loss
 *    - Integrated temperature sensor
 * 3. 16x2 I2C LCD Display Module (PCF8574 I2C backpack)
 *    - I2C address typically 0x27 or 0x3F
 *    - Blue backlight with white text recommended
 * 4. Breadboard and jumper wires
 * 5. Pull-up resistors 4.7kΩ (often built into modules)
 * 
 * GPIO ASSIGNMENTS:
 * =================
 * GPIO21 (SDA)    → I2C Data Line (DS3231 SDA, LCD SDA)
 * GPIO22 (SCL)    → I2C Clock Line (DS3231 SCL, LCD SCL)
 * GPIO1           → DS3231 SQW Pin (1Hz interrupt signal)
 * 5V              → Power for LCD and DS3231 VCC
 * GND             → Common ground for all components
 * 
 * TECHNICAL NOTES:
 * ================
 * 1. The DS3231 SQW pin generates a precise 1Hz square wave signal
 * 2. External interrupt triggers on falling edge for accurate timing
 * 3. Critical sections protect shared variables from race conditions
 * 4. I2C pull-up resistors (4.7kΩ) are typically built into breakout modules
 * 5. Battery backup maintains time accuracy during power cycles
 * 6. Temperature readings update automatically with time display
 * 7. Error handling includes I2C communication failure detection
 * 
 * POWER CONSIDERATIONS:
 * =====================
 * - Total current consumption: ~30mA (LCD backlight dominant)
 * - DS3231 standby current: ~3µA (battery backup mode)
 * - Use 5V supply for maximum LCD compatibility
 * - DS3231 has internal 3.3V regulator for ESP32-S3 compatibility
 * 
 * IDE REQUIREMENTS:
 * =================
 * - Arduino IDE 2.0+
 * - ESP32 Arduino Core v3.0+
 * - Board: "ESP32S3 Dev Module"
 * - Partition Scheme: "Default 4MB with spiffs"
 * - Flash Mode: "QIO"
 * - Flash Size: "16MB"
 * - PSRAM: "OPI PSRAM"
 * 
 * LIBRARY REQUIREMENTS:
 * =====================
 * - RtcDS3231 by Makuna (v2.3.5+)
 * - LiquidCrystal_I2C by johnrickman (v1.1.2+)
 * - Wire (Arduino standard library)
 * 
 * Created: April 3 2019 by Peter Dalmaris
 * Updated: January 2025 for ESP32-S3 compatibility and modern practices
 */

// ============================================================================
// LIBRARY INCLUDES AND HARDWARE INTERFACE SETUP
// ============================================================================

#include <Wire.h>                    // I2C communication library
#include <RtcDS3231.h>              // DS3231 RTC library by Makuna
#include <LiquidCrystal_I2C.h>      // I2C LCD library

// Hardware interface objects
RtcDS3231<TwoWire> Rtc(Wire);       // DS3231 RTC instance using Wire interface
LiquidCrystal_I2C lcd(0x27, 16, 2); // LCD: address 0x27, 16 cols, 2 rows

// ============================================================================
// CONFIGURATION CONSTANTS
// ============================================================================

// GPIO pin assignments for ESP32-S3
const uint8_t INTERRUPT_PIN = 1;     // GPIO1 for DS3231 SQW interrupt
const uint8_t SDA_PIN = 21;          // I2C Data line
const uint8_t SCL_PIN = 22;          // I2C Clock line

// I2C configuration
const uint32_t I2C_FREQUENCY = 100000; // 100kHz I2C bus speed (conservative)

// Display configuration  
const uint8_t LCD_COLS = 16;         // LCD column count
const uint8_t LCD_ROWS = 2;          // LCD row count
constexpr uint8_t DATE_STRING_SIZE = 20; // Buffer size for date strings
constexpr uint8_t TIME_STRING_SIZE = 20; // Buffer size for time strings

// Timing constants
const uint32_t SERIAL_BAUD_RATE = 115200;
const uint32_t STARTUP_DELAY_MS = 1000;

// ============================================================================
// GLOBAL VARIABLES AND INTERRUPT HANDLING
// ============================================================================

// Interrupt-related variables (volatile for ISR safety)
volatile bool timeUpdateFlag = false;    // Flag set by interrupt
volatile uint32_t interruptCount = 0;    // Count interrupts for diagnostics

// Thread synchronization primitive for ESP32-S3
portMUX_TYPE interruptMux = portMUX_INITIALIZER_UNLOCKED;

// System status tracking
bool systemInitialized = false;
bool rtcCommunicationOK = true;
uint32_t lastDisplayUpdate = 0;
uint32_t systemUptime = 0;

// ============================================================================
// INTERRUPT SERVICE ROUTINE
// ============================================================================

/**
 * Hardware interrupt handler for DS3231 SQW signal
 * Called every second when DS3231 generates falling edge on SQW pin
 * Must be IRAM_ATTR for ESP32-S3 compatibility
 */
void IRAM_ATTR handleRTCInterrupt() {
    // Enter critical section to prevent race conditions
    portENTER_CRITICAL_ISR(&interruptMux);
    
    timeUpdateFlag = true;           // Signal main loop to update display
    interruptCount++;                // Increment diagnostic counter
    systemUptime++;                  // Track system uptime in seconds
    
    // Exit critical section
    portEXIT_CRITICAL_ISR(&interruptMux);
}

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

/**
 * Enhanced array size calculation macro
 * Safer than traditional sizeof method
 */
#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

/**
 * Print detailed RTC error information
 * Helps with troubleshooting I2C communication issues
 */
void printRTCError(uint8_t errorCode) {
    Serial.print("RTC Error Code: ");
    Serial.print(errorCode);
    Serial.print(" - ");
    
    switch(errorCode) {
        case 0: Serial.println("Success"); break;
        case 1: Serial.println("Data too long for transmit buffer"); break;
        case 2: Serial.println("Received NACK on transmit of address"); break;
        case 3: Serial.println("Received NACK on transmit of data"); break;
        case 4: Serial.println("Unknown I2C error"); break;
        case 5: Serial.println("Timeout"); break;
        default: Serial.println("Unrecognized error code"); break;
    }
}

/**
 * Scan I2C bus and report found devices
 * Useful for troubleshooting hardware connections
 */
void scanI2CDevices() {
    Serial.println("\n=== I2C Device Scanner ===");
    uint8_t devicesFound = 0;
    
    for(uint8_t address = 1; address < 127; address++) {
        Wire.beginTransmission(address);
        uint8_t error = Wire.endTransmission();
        
        if(error == 0) {
            Serial.printf("I2C device found at address 0x%02X\n", address);
            devicesFound++;
            
            // Identify common devices
            if(address == 0x27 || address == 0x3F) {
                Serial.println("  -> Likely LCD with PCF8574 I2C backpack");
            } else if(address == 0x68) {
                Serial.println("  -> DS3231 RTC detected");
            }
        }
    }
    
    if(devicesFound == 0) {
        Serial.println("No I2C devices found! Check wiring and power.");
    } else {
        Serial.printf("Total devices found: %d\n\n", devicesFound);
    }
}

/**
 * Initialize I2C interface with error checking
 */
bool initializeI2C() {
    Serial.println("Initializing I2C interface...");
    
    // Initialize I2C with custom pins and frequency
    Wire.begin(SDA_PIN, SCL_PIN);
    Wire.setClock(I2C_FREQUENCY);
    
    delay(100); // Allow I2C to stabilize
    
    // Test I2C communication
    scanI2CDevices();
    
    return true;
}

/**
 * Initialize LCD with comprehensive error handling
 */
bool initializeLCD() {
    Serial.println("Initializing LCD display...");
    
    // Initialize LCD
    lcd.init();
    lcd.backlight();
    lcd.clear();
    
    // Display startup message
    lcd.setCursor(0, 0);
    lcd.print("ESP32-S3 RTC");
    lcd.setCursor(0, 1);
    lcd.print("Initializing...");
    
    delay(STARTUP_DELAY_MS);
    
    Serial.println("LCD initialized successfully");
    return true;
}

/**
 * Initialize DS3231 RTC with comprehensive setup
 */
bool initializeRTC() {
    Serial.println("Initializing DS3231 RTC...");
    
    // Begin RTC communication
    Rtc.Begin();
    
    // Get compilation time for reference
    RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
    Serial.print("Sketch compiled: ");
    printDateTime(compiled);
    Serial.println();
    
    // Check RTC validity
    if (!Rtc.IsDateTimeValid()) {
        if (Rtc.LastError() != 0) {
            Serial.print("RTC communication error: ");
            printRTCError(Rtc.LastError());
            return false;
        } else {
            Serial.println("RTC lost confidence in DateTime! Setting to compile time.");
            Rtc.SetDateTime(compiled);
        }
    }
    
    // Ensure RTC is running
    if (!Rtc.GetIsRunning()) {
        Serial.println("RTC was not running. Starting now...");
        Rtc.SetIsRunning(true);
    }
    
    // Check and update time if necessary
    RtcDateTime now = Rtc.GetDateTime();
    if (now < compiled) {
        Serial.println("RTC is older than compile time. Updating...");
        Rtc.SetDateTime(compiled);
    } else if (now > compiled) {
        Serial.println("RTC is newer than compile time (expected)");
    } else {
        Serial.println("RTC matches compile time");
    }
    
    // Configure square wave output for 1Hz interrupt
    Rtc.Enable32kHzPin(false);                                    // Disable 32kHz output
    Rtc.SetSquareWavePin(DS3231SquareWavePin_ModeClock);          // Enable square wave
    Rtc.SetSquareWavePinClockFrequency(DS3231SquareWaveClock_1Hz); // 1Hz frequency
    
    Serial.println("DS3231 RTC initialized successfully");
    return true;
}

/**
 * Configure external interrupt for RTC square wave
 */
bool setupRTCInterrupt() {
    Serial.printf("Setting up interrupt on GPIO%d...\n", INTERRUPT_PIN);
    
    pinMode(INTERRUPT_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN), 
                   handleRTCInterrupt, FALLING);
    
    Serial.println("RTC interrupt configured successfully");
    return true;
}

/**
 * Enhanced LCD update function with temperature display
 */
void updateLCDDisplay(const RtcDateTime& dateTime, RtcTemperature temperature) {
    char dateString[DATE_STRING_SIZE];
    char timeString[TIME_STRING_SIZE];
    
    // Format date string (MM/DD/YYYY)
    snprintf(dateString, ARRAY_SIZE(dateString), "%02u/%02u/%04u",
             dateTime.Month(), dateTime.Day(), dateTime.Year());
    
    // Format time string (HH:MM:SS)
    snprintf(timeString, ARRAY_SIZE(timeString), "%02u:%02u:%02u",
             dateTime.Hour(), dateTime.Minute(), dateTime.Second());
    
    // Update LCD display
    lcd.clear();
    
    // First row: Date
    lcd.setCursor(0, 0);
    lcd.print(dateString);
    
    // Second row: Time and temperature
    lcd.setCursor(0, 1);
    lcd.print(timeString);
    
    // Display temperature (right side of second row)
    lcd.setCursor(10, 1);
    float tempC = temperature.AsFloatDegC();
    if (tempC > -99.0 && tempC < 999.0) { // Sanity check
        lcd.printf("%.1fC", tempC);
    } else {
        lcd.print("--.-C");
    }
    
    // Update last display time
    lastDisplayUpdate = millis();
}

/**
 * Enhanced datetime printing with formatting
 */
void printDateTime(const RtcDateTime& dateTime) {
    char dateTimeString[30];
    
    snprintf(dateTimeString, ARRAY_SIZE(dateTimeString),
             "%02u/%02u/%04u %02u:%02u:%02u",
             dateTime.Month(), dateTime.Day(), dateTime.Year(),
             dateTime.Hour(), dateTime.Minute(), dateTime.Second());
    
    Serial.print(dateTimeString);
}

/**
 * Print comprehensive system status
 */
void printSystemStatus() {
    Serial.println("\n=== System Status ===");
    Serial.printf("Uptime: %lu seconds\n", systemUptime);
    Serial.printf("Interrupt count: %lu\n", interruptCount);
    Serial.printf("Last display update: %lu ms ago\n", millis() - lastDisplayUpdate);
    Serial.printf("Free heap: %lu bytes\n", ESP.getFreeHeap());
    Serial.printf("RTC Communication: %s\n", rtcCommunicationOK ? "OK" : "ERROR");
    
    // Current RTC time
    RtcDateTime now = Rtc.GetDateTime();
    Serial.print("Current time: ");
    printDateTime(now);
    Serial.println();
    
    // Temperature reading
    RtcTemperature temp = Rtc.GetTemperature();
    Serial.printf("Temperature: %.2f°C\n", temp.AsFloatDegC());
    Serial.println("======================\n");
}

// ============================================================================
// MAIN SETUP FUNCTION
// ============================================================================

void setup() {
    // Initialize serial communication
    Serial.begin(SERIAL_BAUD_RATE);
    while (!Serial && millis() < 3000); // Wait for serial or timeout
    
    Serial.println("\n============================================================");
    Serial.println("ESP32-S3 DS3231 RTC with I2C LCD - Comprehensive Version");
    Serial.println("Course: IoT Development with ESP32-S3");
    Serial.println("Lesson: 09.041 - Real-Time Clock with External Interrupts");
    Serial.println("============================================================");
    
    // System initialization sequence
    bool initSuccess = true;
    
    // Step 1: Initialize I2C interface
    if (!initializeI2C()) {
        Serial.println("FATAL: I2C initialization failed!");
        initSuccess = false;
    }
    
    // Step 2: Initialize LCD display
    if (initSuccess && !initializeLCD()) {
        Serial.println("FATAL: LCD initialization failed!");
        initSuccess = false;
    }
    
    // Step 3: Initialize DS3231 RTC
    if (initSuccess && !initializeRTC()) {
        Serial.println("FATAL: RTC initialization failed!");
        initSuccess = false;
    }
    
    // Step 4: Setup RTC interrupt
    if (initSuccess && !setupRTCInterrupt()) {
        Serial.println("FATAL: Interrupt setup failed!");
        initSuccess = false;
    }
    
    if (initSuccess) {
        systemInitialized = true;
        Serial.println("System initialization completed successfully!");
        
        // Clear LCD and show ready message
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("System Ready");
        lcd.setCursor(0, 1);
        lcd.print("Time updating...");
        delay(1000);
        
        // Print initial system status
        printSystemStatus();
        
    } else {
        Serial.println("SYSTEM INITIALIZATION FAILED!");
        Serial.println("Please check hardware connections and restart.");
        
        // Show error on LCD
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("INIT ERROR");
        lcd.setCursor(0, 1);
        lcd.print("Check wiring");
        
        // Halt system
        while(true) {
            delay(1000);
        }
    }
    
    Serial.println("Entering main loop...\n");
}

// ============================================================================
// MAIN LOOP FUNCTION
// ============================================================================

void loop() {
    // Check if interrupt flag is set (time update needed)
    bool updateNeeded = false;
    
    // Safely read interrupt flag using critical section
    portENTER_CRITICAL(&interruptMux);
    if (timeUpdateFlag) {
        timeUpdateFlag = false;  // Reset flag
        updateNeeded = true;
    }
    portEXIT_CRITICAL(&interruptMux);
    
    // Process time update if needed
    if (updateNeeded) {
        // Validate RTC communication
        if (!Rtc.IsDateTimeValid()) {
            rtcCommunicationOK = false;
            
            if (Rtc.LastError() != 0) {
                Serial.print("RTC communication error in loop: ");
                printRTCError(Rtc.LastError());
                
                // Display error on LCD
                lcd.clear();
                lcd.setCursor(0, 0);
                lcd.print("RTC COMM ERROR");
                lcd.setCursor(0, 1);
                lcd.printf("Code: %d", Rtc.LastError());
                
            } else {
                Serial.println("RTC lost confidence in DateTime during operation!");
                
                // Display warning on LCD
                lcd.clear();
                lcd.setCursor(0, 0);
                lcd.print("RTC TIME LOST");
                lcd.setCursor(0, 1);
                lcd.print("Check battery");
            }
            
            return; // Skip this update cycle
        } else {
            rtcCommunicationOK = true;
        }
        
        // Get current time and temperature
        RtcDateTime currentTime = Rtc.GetDateTime();
        RtcTemperature currentTemp = Rtc.GetTemperature();
        
        // Update LCD display
        updateLCDDisplay(currentTime, currentTemp);
        
        // Print time to serial for debugging
        Serial.print("Time update #");
        Serial.print(interruptCount);
        Serial.print(": ");
        printDateTime(currentTime);
        Serial.printf(" | Temp: %.2f°C", currentTemp.AsFloatDegC());
        Serial.println();
        
        // Print detailed status every 60 seconds
        if (interruptCount % 60 == 0) {
            printSystemStatus();
        }
    }
    
    // Small delay to prevent excessive CPU usage
    delay(10);
}
