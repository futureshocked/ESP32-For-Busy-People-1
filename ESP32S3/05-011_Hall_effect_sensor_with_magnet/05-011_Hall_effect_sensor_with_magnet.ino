/*  05.011 - Hall Effect Sensor Implementation for ESP32-S3
 * 
 * Course: IoT Development with ESP32-S3
 * Target Hardware: ESP32-S3-DevKitC-1 v1.1
 * Module: ESP32-S3-WROOM-1-N16R16V
 * 
 * IMPORTANT HARDWARE LIMITATION NOTICE:
 * ====================================
 * The ESP32-S3 does NOT have a built-in Hall effect sensor like the original ESP32.
 * This script demonstrates this limitation and provides alternatives using external
 * Hall sensors for educational purposes.
 * 
 * HARDWARE INFORMATION:
 * =====================
 * Board: ESP32-S3-DevKitC-1 v1.1
 * Module: ESP32-S3-WROOM-1-N16R16V with 16MB Flash + 16MB PSRAM
 * CPU: Dual-core Xtensa LX7 @ 240MHz
 * ADC: 12-bit resolution (0-4095), multiple channels available
 * GPIO: Avoid GPIO35-37 (reserved for internal SPI flash/PSRAM)
 * USB: Native USB support via GPIO19/20
 * RGB LED: WS2812 on GPIO38 (v1.1)
 * 
 * CIRCUIT DIAGRAM (External Hall Sensor Option):
 * ==============================================
 * 
 *                    ESP32-S3-DevKitC-1
 *                  ┌─────────────────────┐
 *                  │                     │
 *                  │               GPIO2 ├─────┐
 *                  │                     │     │
 *                  │               3.3V  ├─────┼─────┐
 *                  │                     │     │     │
 *                  │               GND   ├─────┼─────┼─────┐
 *                  │                     │     │     │     │
 *                  └─────────────────────┘     │     │     │
 *                                              │     │     │
 *                  ┌─────────────────────┐     │     │     │
 *                  │   A3144 Hall Sensor │     │     │     │
 *                  │                     │     │     │     │
 *                  │     [1] [2] [3]     │     │     │     │
 *                  └──────┬───┬───┬──────┘     │     │     │
 *                         │   │   │            │     │     │
 *                       VCC OUT GND            │     │     │
 *                         │   │   │            │     │     │
 *                         └───┼───┼────────────┘     │     │
 *                             │   │                  │     │
 *                             └───┼──────────────────┘     │
 *                                 │                        │
 *                                 └────────────────────────┘
 * 
 * Alternative: Use analog Hall sensor (A1302, SS495A) connected to ADC pin
 * 
 * COMPONENTS:
 * ===========
 * Required:
 * - ESP32-S3-DevKitC-1 v1.1 board
 * - USB-C cable for programming and power
 * 
 * Optional (for external Hall sensor demo):
 * - A3144 Digital Hall sensor (or similar)
 * - A1302 Analog Hall sensor (alternative)
 * - 10kΩ pull-up resistor (for digital sensor)
 * - Breadboard and jumper wires
 * - Neodymium magnet for testing
 * 
 * GPIO ASSIGNMENTS:
 * =================
 * GPIO2  - External digital Hall sensor input (with pull-up)
 * GPIO1  - External analog Hall sensor input (ADC1_CH0)
 * GPIO38 - Built-in RGB LED (WS2812)
 * 
 * TECHNICAL NOTES:
 * ================
 * 1. ESP32-S3 removed several features from original ESP32:
 *    - Built-in Hall effect sensor (hallRead() function not available)
 *    - DAC (Digital-to-Analog Converter)
 *    - Ultra Low Power (ULP) coprocessor
 * 
 * 2. Alternative approaches:
 *    - Use external digital Hall sensors (A3144, A3141, etc.)
 *    - Use external analog Hall sensors (A1302, SS495A, etc.)
 *    - Use magnetometer IC (HMC5883L, QMC5883L)
 * 
 * 3. ADC Configuration:
 *    - ESP32-S3 has 12-bit ADC resolution (0-4095)
 *    - ADC1: GPIO1-GPIO10 (recommended for WiFi compatibility)
 *    - ADC2: GPIO11-GPIO20 (may conflict with WiFi)
 * 
 * IDE REQUIREMENTS:
 * =================
 * - Arduino IDE 2.0+
 * - ESP32 Arduino Core v3.0+
 * - Board: "ESP32S3 Dev Module"
 * - USB CDC On Boot: "Enabled" (for native USB serial)
 * 
 * Created: March 26 2019 by Peter Dalmaris (original ESP32 version)
 * Updated: December 2024 for ESP32-S3 compatibility and educational enhancement
 */

// ============================================================================
// LIBRARY INCLUDES
// ============================================================================
// No external libraries required for this demonstration

// ============================================================================
// CONFIGURATION CONSTANTS
// ============================================================================
const uint8_t DIGITAL_HALL_PIN = 2;        // GPIO2 for digital Hall sensor
const uint8_t ANALOG_HALL_PIN = 1;         // GPIO1 for analog Hall sensor (ADC1_CH0)
const uint8_t RGB_LED_PIN = 38;             // Built-in RGB LED (ESP32-S3-DevKitC-1 v1.1)

const uint32_t SERIAL_BAUD_RATE = 115200;  // Higher baud rate for better performance
const uint32_t SAMPLING_INTERVAL_MS = 100; // 100ms between readings
const uint16_t ADC_RESOLUTION = 12;         // 12-bit ADC resolution
const uint16_t ADC_MAX_VALUE = 4095;        // Maximum ADC value (2^12 - 1)

// Analog Hall sensor calibration (adjust based on your specific sensor)
const uint16_t ANALOG_HALL_BASELINE = 2048; // Typical baseline for 3.3V operation
const uint16_t ANALOG_HALL_THRESHOLD = 100; // Sensitivity threshold

// ============================================================================
// GLOBAL VARIABLES
// ============================================================================
uint32_t lastReadingTime = 0;               // Timestamp of last sensor reading
bool digitalHallState = false;              // Current digital Hall sensor state
bool lastDigitalHallState = false;          // Previous digital Hall sensor state
uint16_t analogHallValue = 0;               // Current analog Hall sensor value
uint32_t readingCounter = 0;                // Counter for number of readings taken

// ============================================================================
// SETUP FUNCTION
// ============================================================================
void setup() {
    // Initialize serial communication with higher baud rate
    Serial.begin(SERIAL_BAUD_RATE);
    
    // Wait for serial connection (important for native USB)
    while (!Serial && millis() < 3000) {
        delay(100);
    }
    
    // Print startup banner and hardware information
    printStartupBanner();
    
    // Configure ADC resolution for precise readings
    analogReadResolution(ADC_RESOLUTION);
    
    // Configure GPIO pins
    pinMode(DIGITAL_HALL_PIN, INPUT_PULLUP);  // Digital Hall sensor with pull-up
    pinMode(RGB_LED_PIN, OUTPUT);             // RGB LED pin
    
    // Initialize RGB LED (turn off)
    digitalWrite(RGB_LED_PIN, LOW);
    
    // Print configuration information
    printConfigurationInfo();
    
    // Demonstrate the Hall sensor limitation
    demonstrateHallSensorLimitation();
    
    Serial.println(F("=== Starting sensor monitoring ==="));
    Serial.println(F("Bring a magnet near the external Hall sensor (if connected)"));
    Serial.println();
}

// ============================================================================
// MAIN LOOP FUNCTION
// ============================================================================
void loop() {
    // Check if it's time for the next reading
    if (millis() - lastReadingTime >= SAMPLING_INTERVAL_MS) {
        lastReadingTime = millis();
        readingCounter++;
        
        // Read both digital and analog Hall sensors (if connected)
        readDigitalHallSensor();
        readAnalogHallSensor();
        
        // Print readings in a formatted manner
        printSensorReadings();
        
        // Update RGB LED based on sensor state
        updateStatusLED();
    }
    
    // Small delay to prevent excessive CPU usage
    delay(1);
}

// ============================================================================
// SENSOR READING FUNCTIONS
// ============================================================================

/**
 * Read digital Hall sensor state
 * Digital Hall sensors typically output LOW when magnet is detected
 */
void readDigitalHallSensor() {
    lastDigitalHallState = digitalHallState;
    digitalHallState = !digitalRead(DIGITAL_HALL_PIN); // Invert because sensor is active LOW
    
    // Detect state changes
    if (digitalHallState != lastDigitalHallState) {
        if (digitalHallState) {
            Serial.println(F("🧲 Digital Hall: MAGNET DETECTED!"));
        } else {
            Serial.println(F("   Digital Hall: Magnet removed"));
        }
    }
}

/**
 * Read analog Hall sensor value
 * Analog sensors provide proportional output based on magnetic field strength
 */
void readAnalogHallSensor() {
    analogHallValue = analogRead(ANALOG_HALL_PIN);
}

/**
 * Print formatted sensor readings to serial monitor
 */
void printSensorReadings() {
    // Print reading counter and timestamp
    Serial.printf("Reading #%lu (t=%lu ms): ", readingCounter, millis());
    
    // Print digital Hall sensor state
    Serial.printf("Digital=%s ", digitalHallState ? "DETECTED" : "none");
    
    // Print analog Hall sensor value and calculated field strength
    int16_t fieldStrength = analogHallValue - ANALOG_HALL_BASELINE;
    Serial.printf("Analog=%d (field=%+d) ", analogHallValue, fieldStrength);
    
    // Indicate significant magnetic field detection
    if (abs(fieldStrength) > ANALOG_HALL_THRESHOLD) {
        Serial.printf("🧲 STRONG FIELD! ");
    }
    
    Serial.println();
}

/**
 * Update the RGB LED based on Hall sensor state
 */
void updateStatusLED() {
    // Simple binary indication: LED on when magnet detected
    if (digitalHallState || (abs(analogHallValue - ANALOG_HALL_BASELINE) > ANALOG_HALL_THRESHOLD)) {
        digitalWrite(RGB_LED_PIN, HIGH);  // Turn on LED when magnet detected
    } else {
        digitalWrite(RGB_LED_PIN, LOW);   // Turn off LED when no magnet
    }
}

// ============================================================================
// UTILITY AND EDUCATIONAL FUNCTIONS
// ============================================================================

/**
 * Print startup banner with course and hardware information
 */
void printStartupBanner() {
    Serial.println(F("\n" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "="));
    Serial.println(F("  ESP32-S3 Hall Effect Sensor Implementation"));
    Serial.println(F("  Course: IoT Development with ESP32-S3"));
    Serial.println(F("  Lesson: 05.010 - Hall Effect Sensors"));
    Serial.println(F("=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "="));
    Serial.printf("Hardware: %s\n", "ESP32-S3-DevKitC-1 v1.1");
    Serial.printf("Module: %s\n", "ESP32-S3-WROOM-1-N16R16V");
    Serial.printf("CPU Frequency: %d MHz\n", getCpuFrequencyMhz());
    Serial.printf("Flash Size: %d MB\n", ESP.getFlashChipSize() / (1024 * 1024));
    Serial.printf("Free Heap: %d bytes\n", ESP.getFreeHeap());
    Serial.println();
}

/**
 * Print configuration and pin assignment information
 */
void printConfigurationInfo() {
    Serial.println(F("Configuration:"));
    Serial.printf("  Serial Baud Rate: %lu bps\n", SERIAL_BAUD_RATE);
    Serial.printf("  Sampling Interval: %lu ms\n", SAMPLING_INTERVAL_MS);
    Serial.printf("  ADC Resolution: %d bits (%d max value)\n", ADC_RESOLUTION, ADC_MAX_VALUE);
    Serial.println();
    
    Serial.println(F("GPIO Assignments:"));
    Serial.printf("  Digital Hall Sensor: GPIO%d (with pull-up)\n", DIGITAL_HALL_PIN);
    Serial.printf("  Analog Hall Sensor:  GPIO%d (ADC1_CH0)\n", ANALOG_HALL_PIN);
    Serial.printf("  RGB LED:             GPIO%d\n", RGB_LED_PIN);
    Serial.println();
}

/**
 * Demonstrate and explain the Hall sensor limitation on ESP32-S3
 */
void demonstrateHallSensorLimitation() {
    Serial.println(F("⚠️  IMPORTANT HARDWARE LIMITATION NOTICE ⚠️"));
    Serial.println(F("=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "="));
    Serial.println(F("The ESP32-S3 does NOT have a built-in Hall effect sensor."));
    Serial.println(F(""));
    Serial.println(F("Original ESP32 features removed in ESP32-S3:"));
    Serial.println(F("  ❌ Built-in Hall effect sensor (hallRead() function)"));
    Serial.println(F("  ❌ Digital-to-Analog Converter (DAC)"));
    Serial.println(F("  ❌ Ultra Low Power (ULP) coprocessor"));
    Serial.println(F(""));
    Serial.println(F("Reasons for removal:"));
    Serial.println(F("  • Cost optimization for mass production"));
    Serial.println(F("  • Die size reduction"));
    Serial.println(F("  • Focus on core WiFi/Bluetooth performance"));
    Serial.println(F("  • External sensors provide better accuracy"));
    Serial.println(F(""));
    Serial.println(F("Alternative solutions:"));
    Serial.println(F("  ✅ External digital Hall sensors (A3144, A3141)"));
    Serial.println(F("  ✅ External analog Hall sensors (A1302, SS495A)"));
    Serial.println(F("  ✅ Digital magnetometers (HMC5883L, QMC5883L)"));
    Serial.println(F("  ✅ Better sensitivity and accuracy"));
    Serial.println(F("=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "="));
    Serial.println();
    
    // Attempt to call hallRead() to demonstrate the compilation error
    Serial.println(F("Demonstration: Attempting to call hallRead()..."));
    Serial.println(F("Result: This would cause a compilation error because"));
    Serial.println(F("        hallRead() function does not exist on ESP32-S3."));
    Serial.println();
    
    // Show what the original code would have looked like
    Serial.println(F("Original ESP32 code would have been:"));
    Serial.println(F("  int hallValue = hallRead();"));
    Serial.println(F("  Serial.println(hallValue);"));
    Serial.println();
}

// ============================================================================
// EDUCATIONAL HELPER FUNCTIONS
// ============================================================================

/**
 * Convert ADC reading to approximate voltage
 * Useful for understanding analog sensor behavior
 */
float adcToVoltage(uint16_t adcValue) {
    return (float)adcValue * 3.3f / (float)ADC_MAX_VALUE;
}

/**
 * Calculate magnetic field strength indication
 * This is a simplified calculation for educational purposes
 */
int16_t calculateFieldStrength(uint16_t adcValue) {
    return adcValue - ANALOG_HALL_BASELINE;
}

/**
 * Print troubleshooting information
 * Call this function if sensors are not working as expected
 */
void printTroubleshootingInfo() {
    Serial.println(F("TROUBLESHOOTING GUIDE:"));
    Serial.println(F("====================="));
    Serial.println(F(""));
    Serial.println(F("No external sensors connected:"));
    Serial.println(F("  • This is normal - the demo shows the limitation"));
    Serial.println(F("  • Digital readings will show 'none' (pull-up)"));
    Serial.println(F("  • Analog readings will show ADC noise"));
    Serial.println(F(""));
    Serial.println(F("Digital Hall sensor not working:"));
    Serial.println(F("  • Check VCC connection (3.3V)"));
    Serial.println(F("  • Check GND connection"));
    Serial.println(F("  • Verify signal pin connection (GPIO2)"));
    Serial.println(F("  • Ensure magnet polarity is correct"));
    Serial.println(F(""));
    Serial.println(F("Analog Hall sensor not working:"));
    Serial.println(F("  • Check power supply connections"));
    Serial.println(F("  • Verify ADC pin connection (GPIO1)"));
    Serial.println(F("  • Adjust ANALOG_HALL_BASELINE constant"));
    Serial.println(F("  • Try different magnet strengths"));
    Serial.println(F(""));
    Serial.println(F("General tips:"));
    Serial.println(F("  • Use strong neodymium magnets"));
    Serial.println(F("  • Bring magnet very close to sensor"));
    Serial.println(F("  • Try both north and south poles"));
    Serial.println(F("  • Check sensor datasheet for specifications"));
    Serial.println();
}
