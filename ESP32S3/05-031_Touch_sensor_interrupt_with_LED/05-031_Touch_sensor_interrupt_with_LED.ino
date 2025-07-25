/*  05.031 - Touch Sensor with Interrupt and LED Control for ESP32-S3
 * 
 * Course: IoT Development with ESP32-S3
 * Target Hardware: ESP32-S3-DevKitC-1 v1.1
 * Module: ESP32-S3-WROOM-1-N16R16V
 * 
 * This sketch demonstrates how to control an LED using the ESP32-S3's integrated 
 * capacitive touch sensor with interrupt-driven detection. When GPIO13 is touched,
 * an interrupt service routine (ISR) is triggered, which sets a flag that the 
 * main loop monitors to control LED illumination.
 * 
 * Key Learning Objectives:
 * - Understanding capacitive touch sensing principles
 * - Implementing interrupt service routines (ISR) 
 * - Proper interrupt handling and flag-based communication
 * - GPIO configuration for ESP32-S3
 * - Serial debugging techniques
 * 
 * HARDWARE INFORMATION:
 * =====================
 * ESP32-S3-DevKitC-1 v1.1 Specifications:
 * - Dual-core Xtensa LX7 CPU (240MHz)
 * - 16MB Flash Memory, 16MB PSRAM
 * - Built-in capacitive touch sensors on GPIO1-GPIO14
 * - 12-bit ADC resolution (0-4095)
 * - Operating voltage: 3.3V
 * - USB-C connector for programming and power
 * - Onboard RGB LED (WS2812) on GPIO38
 * 
 * CIRCUIT DIAGRAM:
 * ================
 * 
 *     ESP32-S3-DevKitC-1 v1.1        External LED Circuit
 *     ┌─────────────────────────┐     ┌────────────────────┐
 *     │                         │     │                    │
 *     │  [USB-C]                │     │         LED        │
 *     │                         │     │        ┌─┴─┐       │
 *     │  GPIO13 ●────────────────────── Touch Wire         │
 *     │                         │     │        │ + │       │
 *     │  GPIO2  ●─────────────────────┼────────┤   │       │
 *     │                         │     │        │ - │       │
 *     │  GND    ●─────────────────────┼────────┴─┬─┘       │
 *     │                         │     │          │         │
 *     │  3V3    ●               │     │        [320Ω]      │
 *     │                         │     │          │         │
 *     │  GPIO38 (RGB LED)       │     │          │         │
 *     │                         │     │          │         │
 *     └─────────────────────────┘     └──────────┼─────────┘
 *                                                │
 *                                                ●
 *                                               GND
 * 
 * COMPONENTS:
 * ===========
 * Required Components:
 * - 1x ESP32-S3-DevKitC-1 v1.1 development board
 * - 1x LED (any color, 3mm or 5mm)
 * - 1x 320Ω resistor (current limiting for LED)
 * - 1x Jumper wire (for touch sensor connection)
 * - 1x Breadboard (optional, for neater connections)
 * - 2x Male-to-male jumper wires (for connections)
 * 
 * Component Specifications:
 * - LED: Forward voltage ~2.0V, Forward current ~10mA
 * - Resistor: 320Ω ±5% (orange-red-brown-gold bands)
 * - Wire: 22AWG solid core recommended for breadboard use
 * 
 * GPIO ASSIGNMENTS:
 * =================
 * GPIO Pin | Function      | Direction | Notes
 * ---------|---------------|-----------|---------------------------
 * GPIO2    | LED Control   | Output    | External LED via resistor
 * GPIO13   | Touch Sensor  | Input     | TOUCH13, capacitive touch
 * GPIO38   | RGB LED       | Output    | Onboard WS2812 (alternative)
 * 
 * Reserved GPIOs (DO NOT USE):
 * - GPIO35, GPIO36, GPIO37: Reserved for internal flash/PSRAM
 * - GPIO19, GPIO20: USB D-/D+ lines
 * 
 * TECHNICAL NOTES:
 * ================
 * 1. Touch Sensor Operation:
 *    - ESP32-S3 has 14 capacitive touch sensors (TOUCH1-TOUCH14)
 *    - GPIO13 corresponds to TOUCH13
 *    - Touch detection threshold is calibrated experimentally
 *    - Lower values indicate stronger touch detection
 * 
 * 2. Interrupt Service Routine (ISR):
 *    - ISR should be minimal and fast-executing
 *    - Use volatile variables for ISR communication
 *    - Avoid Serial.print() or delay() in ISR
 *    - Flag-based communication between ISR and main loop
 * 
 * 3. Hardware Protection:
 *    - 320Ω resistor limits LED current to ~10mA
 *    - GPIO2 can source up to 40mA (well within limits)
 *    - Touch sensor is protected by internal circuitry
 * 
 * 4. Power Considerations:
 *    - Total current draw: <50mA (including ESP32-S3 operation)
 *    - USB-C provides up to 500mA (more than sufficient)
 * 
 * IDE REQUIREMENTS:
 * =================
 * - Arduino IDE 2.0+
 * - ESP32 Arduino Core v3.0+
 * - Board: "ESP32S3 Dev Module"
 * - Flash Size: "16MB (128Mb)"
 * - PSRAM: "OPI PSRAM"
 * - USB Mode: "Hardware CDC and JTAG"
 * 
 * Library Dependencies: None (uses built-in ESP32-S3 libraries)
 * 
 * Created: March 27, 2019 by Peter Dalmaris (Original ESP32 version)
 * Updated: December 2024 for ESP32-S3 compatibility and educational enhancement
 * 
 * References:
 * - ESP32-S3 Technical Reference Manual
 * - ESP32-S3 Series Datasheet
 * - Arduino ESP32 Core Documentation
 */

// ========================================================================
// CONFIGURATION CONSTANTS
// ========================================================================

// GPIO Pin Assignments
const uint8_t LED_GPIO = 2;           // External LED control pin
const uint8_t TOUCH_GPIO = 13;        // Touch sensor pin (TOUCH13)
const uint8_t ONBOARD_LED_GPIO = 38;  // Onboard RGB LED (alternative)

// Touch Sensor Configuration
const uint16_t TOUCH_THRESHOLD = 20;  // Touch detection threshold (lower = more sensitive)
                                      // Typical range: 10-40 (determined experimentally)

// Timing Configuration
const uint16_t LED_ON_DURATION_MS = 500;    // LED illumination time (milliseconds)
const uint16_t SERIAL_BAUD_RATE = 115200;   // Serial communication speed
const uint16_t SETUP_DELAY_MS = 1000;       // Startup delay for serial monitor

// ========================================================================
// GLOBAL VARIABLES
// ========================================================================

// Interrupt Communication Variables (must be volatile for ISR access)
volatile bool touchDetectedFlag = false;  // ISR sets this flag when touch occurs

// Performance Monitoring Variables
unsigned long lastTouchTime = 0;          // Timestamp of last touch event
unsigned long touchCount = 0;             // Total number of touch events
uint16_t lastTouchValue = 0;              // Last raw touch sensor reading

// ========================================================================
// INTERRUPT SERVICE ROUTINE (ISR)
// ========================================================================

/*
 * Touch Sensor Interrupt Service Routine
 * 
 * This function is called automatically when a touch event occurs.
 * IMPORTANT: Keep ISR functions minimal and fast-executing!
 * 
 * Best Practices for ISR:
 * - No Serial.print() calls
 * - No delay() functions  
 * - No complex calculations
 * - Use volatile variables for data sharing
 * - Set flags for main loop to process
 */
void IRAM_ATTR handleTouchInterrupt() {
    touchDetectedFlag = true;  // Set flag for main loop to process
    touchCount++;              // Increment touch counter
    lastTouchTime = millis();  // Record timestamp
}

// ========================================================================
// SETUP FUNCTION
// ========================================================================

void setup() {
    // Initialize Serial Communication
    Serial.begin(SERIAL_BAUD_RATE);
    delay(SETUP_DELAY_MS);  // Allow time for serial monitor to connect
    
    // Print startup information
    printStartupInfo();
    
    // Configure GPIO Pins
    pinMode(LED_GPIO, OUTPUT);
    digitalWrite(LED_GPIO, LOW);  // Ensure LED starts in OFF state
    
    // Initialize Touch Sensor
    initializeTouchSensor();
    
    // Perform initial system checks
    performSystemChecks();
    
    Serial.println("✓ Setup completed successfully!");
    Serial.println("Touch GPIO13 to trigger LED...\n");
}

// ========================================================================
// MAIN LOOP FUNCTION
// ========================================================================

void loop() {
    // Check for touch detection flag (set by ISR)
    if (touchDetectedFlag) {
        processTouchEvent();
        touchDetectedFlag = false;  // Clear flag after processing
    }
    
    // Periodic status reporting (every 10 seconds)
    static unsigned long lastStatusTime = 0;
    if (millis() - lastStatusTime > 10000) {
        printStatusReport();
        lastStatusTime = millis();
    }
    
    // Small delay to prevent excessive CPU usage
    delay(10);
}

// ========================================================================
// HELPER FUNCTIONS
// ========================================================================

/*
 * Print comprehensive startup information
 */
void printStartupInfo() {
    Serial.println("========================================");
    Serial.println("ESP32-S3 Touch Sensor with Interrupt");
    Serial.println("========================================");
    Serial.print("Board: ESP32-S3-DevKitC-1 v1.1\n");
    Serial.print("Module: ESP32-S3-WROOM-1-N16R16V\n");
    Serial.print("Arduino Core: ESP32 v3.0+\n");
    Serial.printf("CPU Frequency: %d MHz\n", getCpuFrequencyMhz());
    Serial.printf("Flash Size: %d MB\n", ESP.getFlashChipSize() / (1024 * 1024));
    Serial.printf("Free Heap: %d bytes\n", ESP.getFreeHeap());
    Serial.println("========================================");
    Serial.printf("LED GPIO: %d\n", LED_GPIO);
    Serial.printf("Touch GPIO: %d (TOUCH13)\n", TOUCH_GPIO);
    Serial.printf("Touch Threshold: %d\n", TOUCH_THRESHOLD);
    Serial.println("========================================\n");
}

/*
 * Initialize and configure the touch sensor
 */
void initializeTouchSensor() {
    Serial.print("Initializing touch sensor... ");
    
    // Attach interrupt to touch sensor
    // Parameters: touch_num, function, threshold
    touchAttachInterrupt(T13, handleTouchInterrupt, TOUCH_THRESHOLD);
    
    Serial.println("✓ Done");
    
    // Calibrate touch sensor (take baseline reading)
    calibrateTouchSensor();
}

/*
 * Calibrate touch sensor and provide user feedback
 */
void calibrateTouchSensor() {
    Serial.print("Calibrating touch sensor (please don't touch)... ");
    
    const int calibrationSamples = 10;
    long totalValue = 0;
    
    for (int i = 0; i < calibrationSamples; i++) {
        totalValue += touchRead(T13);
        delay(100);
    }
    
    uint16_t baselineValue = totalValue / calibrationSamples;
    
    Serial.printf("✓ Done\n");
    Serial.printf("Baseline touch value: %d\n", baselineValue);
    Serial.printf("Touch threshold: %d\n", TOUCH_THRESHOLD);
    
    if (baselineValue < TOUCH_THRESHOLD + 10) {
        Serial.println("⚠ WARNING: Baseline value is close to threshold!");
        Serial.println("   Consider adjusting TOUCH_THRESHOLD or check wiring.");
    }
}

/*
 * Perform initial system checks and validation
 */
void performSystemChecks() {
    Serial.print("Performing system checks... ");
    
    // Test LED functionality
    digitalWrite(LED_GPIO, HIGH);
    delay(200);
    digitalWrite(LED_GPIO, LOW);
    
    // Validate touch sensor readings
    uint16_t currentTouchValue = touchRead(T13);
    if (currentTouchValue == 0 || currentTouchValue > 1000) {
        Serial.println("✗ WARNING: Touch sensor may not be functioning correctly!");
        Serial.printf("   Current reading: %d (expected: 50-200)\n", currentTouchValue);
    }
    
    Serial.println("✓ Done");
}

/*
 * Process touch event detected by ISR
 */
void processTouchEvent() {
    // Read current touch value for debugging
    lastTouchValue = touchRead(T13);
    
    // Turn on LED
    digitalWrite(LED_GPIO, HIGH);
    
    // Print touch detection information
    Serial.printf("Touch #%lu detected! ", touchCount);
    Serial.printf("Value: %d, ", lastTouchValue);
    Serial.printf("Time: %lu ms\n", lastTouchTime);
    
    // Keep LED on for specified duration
    delay(LED_ON_DURATION_MS);
    
    // Turn off LED
    digitalWrite(LED_GPIO, LOW);
    
    Serial.println("LED turned off");
}

/*
 * Print periodic status report
 */
void printStatusReport() {
    Serial.println("\n--- Status Report ---");
    Serial.printf("Total touches: %lu\n", touchCount);
    Serial.printf("Current touch value: %d\n", touchRead(T13));
    Serial.printf("Free heap: %d bytes\n", ESP.getFreeHeap());
    Serial.printf("Uptime: %lu seconds\n", millis() / 1000);
    
    if (touchCount > 0) {
        Serial.printf("Last touch: %lu ms ago\n", millis() - lastTouchTime);
    }
    Serial.println("--------------------\n");
}
