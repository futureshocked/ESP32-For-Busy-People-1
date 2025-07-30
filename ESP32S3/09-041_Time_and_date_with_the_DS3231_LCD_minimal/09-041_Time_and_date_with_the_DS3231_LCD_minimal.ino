/*  09.041 - Time and Date with External Interrupts using DS3231 RTC and I2C LCD - ESP32-S3
 * 
 * Course: IoT Development with ESP32-S3
 * Target Hardware: ESP32-S3-DevKitC-1 v1.1
 * Module: ESP32-S3-WROOM-1-N16R16V
 * 
 * This minimal sketch demonstrates real-time clock (RTC) functionality with 
 * interrupt-driven display updates. Perfect for beginners learning RTC basics,
 * I2C communication, and external interrupt handling with the DS3231 precision
 * RTC module and I2C LCD display.
 * 
 * Key Learning Objectives:
 * - External interrupt handling with DS3231 SQW signal
 * - I2C communication with multiple devices (RTC + LCD)
 * - Real-time clock programming and time display
 * - Temperature sensor reading from DS3231
 * - Critical section programming for interrupt safety
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
 * 7. This minimal version focuses on core functionality for learning
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
 * Updated: January 2025 for ESP32-S3 compatibility and minimal learning approach
 */

// ============================================================================
// LIBRARY INCLUDES
// ============================================================================

#include <Wire.h>
#include <RtcDS3231.h>
#include <LiquidCrystal_I2C.h>

// ============================================================================
// HARDWARE CONFIGURATION
// ============================================================================

// Create hardware interface objects
RtcDS3231<TwoWire> Rtc(Wire);
LiquidCrystal_I2C lcd(0x27, 16, 2);

// GPIO pin assignments
const uint8_t INTERRUPT_PIN = 1;

// ============================================================================
// GLOBAL VARIABLES
// ============================================================================

// Interrupt handling variables
volatile bool updateTime = false;
portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

// ============================================================================
// INTERRUPT SERVICE ROUTINE
// ============================================================================

void IRAM_ATTR handleInterrupt() {
    portENTER_CRITICAL_ISR(&mux);
    updateTime = true;
    portEXIT_CRITICAL_ISR(&mux);
}

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

#define countof(a) (sizeof(a) / sizeof(a[0]))

void updateLCD(const RtcDateTime& dt, RtcTemperature temp) {
    char datestring[20];
    char timestring[20];
    
    // Format date and time strings
    snprintf(datestring, countof(datestring), "%02u/%02u/%04u", 
             dt.Month(), dt.Day(), dt.Year());
    snprintf(timestring, countof(timestring), "%02u:%02u:%02u", 
             dt.Hour(), dt.Minute(), dt.Second());
    
    // Update LCD display
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(datestring);
    lcd.setCursor(0, 1);
    lcd.print(timestring);
    lcd.setCursor(10, 1);
    lcd.print(temp.AsFloatDegC());
    lcd.print("C");
}

void printDateTime(const RtcDateTime& dt) {
    char datestring[20];
    snprintf(datestring, countof(datestring), "%02u/%02u/%04u %02u:%02u:%02u",
             dt.Month(), dt.Day(), dt.Year(),
             dt.Hour(), dt.Minute(), dt.Second());
    Serial.print(datestring);
}

// ============================================================================
// SETUP FUNCTION
// ============================================================================

void setup() {
    // Initialize serial communication
    Serial.begin(115200);
    
    // Initialize LCD
    lcd.init();
    lcd.backlight();
    
    Serial.print("Compiled: ");
    Serial.print(__DATE__);
    Serial.println(__TIME__);
    
    // Initialize RTC
    Rtc.Begin();
    
    RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
    printDateTime(compiled);
    Serial.println();
    
    // Check RTC status and set time if needed
    if (!Rtc.IsDateTimeValid()) {
        if (Rtc.LastError() != 0) {
            Serial.print("RTC communications error = ");
            Serial.println(Rtc.LastError());
        } else {
            Serial.println("RTC lost confidence in the DateTime!");
            Rtc.SetDateTime(compiled);
        }
    }
    
    if (!Rtc.GetIsRunning()) {
        Serial.println("RTC was not actively running, starting now");
        Rtc.SetIsRunning(true);
    }
    
    RtcDateTime now = Rtc.GetDateTime();
    if (now < compiled) {
        Serial.println("RTC is older than compile time! Updating DateTime");
        Rtc.SetDateTime(compiled);
    } else if (now > compiled) {
        Serial.println("RTC is newer than compile time (expected)");
    } else {
        Serial.println("RTC is the same as compile time");
    }
    
    // Configure RTC square wave output
    Rtc.Enable32kHzPin(false);
    Rtc.SetSquareWavePin(DS3231SquareWavePin_ModeClock);
    Rtc.SetSquareWavePinClockFrequency(DS3231SquareWaveClock_1Hz);
    
    // Setup interrupt
    pinMode(INTERRUPT_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN), handleInterrupt, FALLING);
    
    Serial.println("Setup complete!");
}

// ============================================================================
// MAIN LOOP
// ============================================================================

void loop() {
    if (updateTime) {
        // Reset the flag safely
        portENTER_CRITICAL(&mux);
        updateTime = false;
        portEXIT_CRITICAL(&mux);
        
        // Check RTC communication
        if (!Rtc.IsDateTimeValid()) {
            if (Rtc.LastError() != 0) {
                Serial.print("RTC communications error = ");
                Serial.println(Rtc.LastError());
            } else {
                Serial.println("RTC lost confidence in the DateTime!");
            }
        }
        
        // Get current time and temperature
        RtcDateTime now = Rtc.GetDateTime();
        RtcTemperature temp = Rtc.GetTemperature();
        
        // Update LCD and print to serial
        updateLCD(now, temp);
        printDateTime(now);
        Serial.println();
    }
}
