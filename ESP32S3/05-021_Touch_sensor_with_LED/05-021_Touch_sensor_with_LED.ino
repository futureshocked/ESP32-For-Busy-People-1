/*  05.021 - Advanced Touch Sensor with Multi-LED Control for ESP32-S3
 * 
 * Course: IoT Development with ESP32-S3
 * Target Hardware: ESP32-S3-DevKitC-1 v1.1
 * Module: ESP32-S3-WROOM-1-N16R16V
 * 
 * This sketch demonstrates advanced touch sensor capabilities of the ESP32-S3,
 * controlling both an external LED and the onboard RGB LED. The implementation
 * includes automatic threshold calibration, debouncing, and comprehensive
 * debugging output for educational purposes.
 * 
 * HARDWARE INFORMATION:
 * =====================
 * Target Board: ESP32-S3-DevKitC-1 v1.1
 * MCU: ESP32-S3-WROOM-1-N16R16V
 * Flash: 16 MB
 * PSRAM: 16 MB
 * ADC Resolution: 12-bit (0-4095)
 * Touch Sensors: 14 capacitive touch channels
 * Onboard RGB LED: WS2812 on GPIO38 (v1.1)
 * 
 * CIRCUIT DIAGRAM:
 * ================
 *                    ESP32-S3-DevKitC-1 v1.1
 *                    ┌─────────────────────┐
 *                    │                     │
 *                    │                GPIO2├─────┬─────── LED Anode
 *                    │                     │     │
 *                    │                GPIO4├─────┼─────── Touch Wire
 *                    │                     │     │
 *                    │               GPIO38├─────┼─────── RGB LED (onboard)
 *                    │                     │     │
 *                    │                 GND├─────┴─────── LED Cathode & GND
 *                    │                     │
 *                    └─────────────────────┘
 *                                             │
 *                                             ▼
 *                                        ┌─────────┐
 *                                        │   LED   │
 *                                        │  ┌───┐  │
 *                                        └──┤ R ├──┘
 *                                           └───┘
 *                                          330Ω
 * 
 * COMPONENTS:
 * ===========
 * 1x ESP32-S3-DevKitC-1 v1.1 Development Board
 * 1x Standard LED (any color, 3mm or 5mm)
 * 1x 330Ω Resistor (Orange-Orange-Brown-Gold)
 * 1x Jumper wire or touch probe for GPIO4
 * 1x Breadboard (half-size recommended)
 * 2x Male-to-Male jumper wires
 * 
 * GPIO ASSIGNMENTS:
 * =================
 * GPIO2  : External LED control (output)
 * GPIO4  : Touch sensor input (T4)
 * GPIO38 : Onboard RGB LED (WS2812)
 * 
 * TECHNICAL NOTES:
 * ================
 * - Touch threshold is auto-calibrated during setup
 * - Baseline touch value is measured without contact
 * - Touch detection uses percentage-based threshold (25% reduction)
 * - Debouncing prevents false triggering
 * - RGB LED demonstrates advanced peripheral control
 * - Serial output provides real-time debugging information
 * 
 * SAFETY CONSIDERATIONS:
 * =====================
 * - 330Ω resistor limits LED current to safe levels (~10mA)
 * - Touch sensor operates at 3.3V logic levels
 * - Avoid connecting touch wire to high voltage sources
 * - Use proper grounding to prevent interference
 * 
 * IDE REQUIREMENTS:
 * =================
 * - Arduino IDE 2.0+
 * - ESP32 Arduino Core v3.0+
 * - Board: "ESP32S3 Dev Module"
 * - USB CDC On Boot: "Enabled" (recommended)
 * - Flash Size: "16MB (128Mb)"
 * - PSRAM: "OPI PSRAM"
 * 
 * Created: March 27, 2019 by Peter Dalmaris
 * Updated: July 25, 2025 for ESP32-S3 compatibility and educational enhancement
 */

// ================================================================================
// CONFIGURATION CONSTANTS
// ================================================================================

// GPIO Pin Assignments
const uint8_t EXTERNAL_LED_PIN = 2;        // External LED control pin
const uint8_t TOUCH_SENSOR_PIN = 4;        // Touch sensor pin (T4)
const uint8_t RGB_LED_PIN = 38;            // Onboard RGB LED pin (v1.1)

// Touch Sensor Configuration
const uint16_t TOUCH_SAMPLES = 10;         // Number of samples for calibration
const uint8_t TOUCH_THRESHOLD_PERCENT = 25; // Percentage reduction for touch detection
const uint16_t DEBOUNCE_DELAY_MS = 50;     // Debounce delay in milliseconds
const uint16_t SERIAL_UPDATE_MS = 500;     // Serial output update interval

// RGB LED Configuration (for WS2812)
const uint8_t LED_BRIGHTNESS = 50;         // RGB LED brightness (0-255)

// ================================================================================
// GLOBAL VARIABLES
// ================================================================================

// Touch sensor state management
uint16_t touchBaseline = 0;                // Baseline touch value (no touch)
uint16_t touchThreshold = 0;               // Calculated touch threshold
bool isLedOn = false;                      // Current LED state
bool lastTouchState = false;               // Previous touch state for debouncing
unsigned long lastTouchTime = 0;           // Last touch event timestamp
unsigned long lastSerialOutput = 0;        // Last serial output timestamp

// ================================================================================
// SETUP FUNCTION
// ================================================================================

void setup() {
    // Initialize serial communication
    Serial.begin(115200);
    delay(1000); // Allow time for serial monitor startup
    
    // Print startup banner
    printStartupBanner();
    
    // Initialize GPIO pins
    initializeHardware();
    
    // Calibrate touch sensor
    calibrateTouchSensor();
    
    // Setup complete
    Serial.println("Setup complete! Touch GPIO4 to control LEDs");
    Serial.println("================================================================================");
}

// ================================================================================
// MAIN LOOP FUNCTION
// ================================================================================

void loop() {
    // Read current touch value
    uint16_t currentTouchValue = touchRead(TOUCH_SENSOR_PIN);
    
    // Detect touch event with debouncing
    bool touchDetected = processTouchInput(currentTouchValue);
    
    // Handle touch event
    if (touchDetected && !lastTouchState) {
        toggleLEDs();
        lastTouchState = true;
        lastTouchTime = millis();
    } else if (!touchDetected && lastTouchState && 
               (millis() - lastTouchTime > DEBOUNCE_DELAY_MS)) {
        lastTouchState = false;
    }
    
    // Update serial output periodically
    updateSerialOutput(currentTouchValue);
    
    // Small delay to prevent excessive CPU usage
    delay(10);
}

// ================================================================================
// HARDWARE INITIALIZATION FUNCTIONS
// ================================================================================

void initializeHardware() {
    Serial.println("Initializing hardware...");
    
    // Configure external LED pin
    pinMode(EXTERNAL_LED_PIN, OUTPUT);
    digitalWrite(EXTERNAL_LED_PIN, LOW);
    Serial.println("  ✓ External LED pin configured (GPIO" + String(EXTERNAL_LED_PIN) + ")");
    
    // Configure RGB LED pin
    pinMode(RGB_LED_PIN, OUTPUT);
    digitalWrite(RGB_LED_PIN, LOW);
    Serial.println("  ✓ RGB LED pin configured (GPIO" + String(RGB_LED_PIN) + ")");
    
    // Touch sensor is automatically configured by touchRead()
    Serial.println("  ✓ Touch sensor ready (GPIO" + String(TOUCH_SENSOR_PIN) + ")");
    
    Serial.println("Hardware initialization complete!\n");
}

void calibrateTouchSensor() {
    Serial.println("Calibrating touch sensor...");
    Serial.println("DO NOT touch the sensor during calibration!");
    
    // Allow time for user to read message
    delay(2000);
    
    uint32_t totalReading = 0;
    uint16_t minReading = 65535;
    uint16_t maxReading = 0;
    
    // Take multiple baseline readings
    for (int i = 0; i < TOUCH_SAMPLES; i++) {
        uint16_t reading = touchRead(TOUCH_SENSOR_PIN);
        totalReading += reading;
        
        if (reading < minReading) minReading = reading;
        if (reading > maxReading) maxReading = reading;
        
        Serial.print("Calibration sample " + String(i + 1) + "/" + String(TOUCH_SAMPLES) + ": ");
        Serial.println(reading);
        
        delay(100);
    }
    
    // Calculate baseline and threshold
    touchBaseline = totalReading / TOUCH_SAMPLES;
    touchThreshold = touchBaseline - (touchBaseline * TOUCH_THRESHOLD_PERCENT / 100);
    
    // Display calibration results
    Serial.println("\nCalibration Results:");
    Serial.println("  Baseline (no touch): " + String(touchBaseline));
    Serial.println("  Min reading: " + String(minReading));
    Serial.println("  Max reading: " + String(maxReading));
    Serial.println("  Touch threshold: " + String(touchThreshold));
    Serial.println("  Variation: ±" + String(maxReading - minReading));
    
    // Validate calibration
    if ((maxReading - minReading) > (touchBaseline / 10)) {
        Serial.println("  ⚠ WARNING: High variation detected. Ensure stable environment.");
    } else {
        Serial.println("  ✓ Calibration successful!");
    }
    
    Serial.println();
}

// ================================================================================
// TOUCH PROCESSING FUNCTIONS
// ================================================================================

bool processTouchInput(uint16_t touchValue) {
    // Touch is detected when value drops below threshold
    bool touchDetected = (touchValue < touchThreshold);
    
    // Additional validation: ensure significant drop from baseline
    if (touchDetected) {
        uint16_t touchDelta = touchBaseline - touchValue;
        uint8_t reductionPercent = (touchDelta * 100) / touchBaseline;
        
        // Require at least the configured percentage reduction
        if (reductionPercent < TOUCH_THRESHOLD_PERCENT) {
            touchDetected = false;
        }
    }
    
    return touchDetected;
}

void toggleLEDs() {
    // Toggle LED state
    isLedOn = !isLedOn;
    
    // Update external LED
    digitalWrite(EXTERNAL_LED_PIN, isLedOn ? HIGH : LOW);
    
    // Update RGB LED with simple color indication
    updateRGBLED();
    
    // Log state change
    Serial.println("Touch detected! LEDs turned " + String(isLedOn ? "ON" : "OFF"));
}

void updateRGBLED() {
    if (isLedOn) {
        // Simple RGB control - set to blue when on
        // Note: This is a basic implementation. For full WS2812 control,
        // consider using libraries like FastLED or Adafruit NeoPixel
        analogWrite(RGB_LED_PIN, LED_BRIGHTNESS);
    } else {
        analogWrite(RGB_LED_PIN, 0);
    }
}

// ================================================================================
// OUTPUT AND DEBUGGING FUNCTIONS
// ================================================================================

void updateSerialOutput(uint16_t currentTouchValue) {
    unsigned long currentTime = millis();
    
    if (currentTime - lastSerialOutput >= SERIAL_UPDATE_MS) {
        // Calculate touch statistics
        uint16_t touchDelta = (currentTouchValue < touchBaseline) ? 
                             (touchBaseline - currentTouchValue) : 0;
        uint8_t reductionPercent = (touchBaseline > 0) ? 
                                  (touchDelta * 100) / touchBaseline : 0;
        
        // Display current readings
        Serial.print("Touch: " + String(currentTouchValue));
        Serial.print(" | Baseline: " + String(touchBaseline));
        Serial.print(" | Threshold: " + String(touchThreshold));
        Serial.print(" | Delta: " + String(touchDelta));
        Serial.print(" | Reduction: " + String(reductionPercent) + "%");
        Serial.print(" | LED: " + String(isLedOn ? "ON " : "OFF"));
        Serial.print(" | Status: ");
        
        if (currentTouchValue < touchThreshold) {
            Serial.println("TOUCHING");
        } else {
            Serial.println("NOT TOUCHING");
        }
        
        lastSerialOutput = currentTime;
    }
}

void printStartupBanner() {
    Serial.println("================================================================================");
    Serial.println("           ESP32-S3 Advanced Touch Sensor with LED Control");
    Serial.println("================================================================================");
    Serial.println("Course: IoT Development with ESP32-S3");
    Serial.println("Hardware: ESP32-S3-DevKitC-1 v1.1");
    Serial.println("Module: ESP32-S3-WROOM-1-N16R16V");
    Serial.println();
    Serial.println("Features:");
    Serial.println("  • Automatic touch threshold calibration");
    Serial.println("  • Touch debouncing for reliable operation");
    Serial.println("  • External LED control (GPIO" + String(EXTERNAL_LED_PIN) + ")");
    Serial.println("  • Onboard RGB LED indication (GPIO" + String(RGB_LED_PIN) + ")");
    Serial.println("  • Real-time debugging output");
    Serial.println("  • Professional embedded programming practices");
    Serial.println();
    Serial.println("Touch sensor: GPIO" + String(TOUCH_SENSOR_PIN) + " (T" + String(TOUCH_SENSOR_PIN) + ")");
    Serial.println("================================================================================");
    Serial.println();
}

// ================================================================================
// UTILITY AND HELPER FUNCTIONS
// ================================================================================

void printTroubleshootingInfo() {
    Serial.println("\nTROUBLESHOOTING GUIDE:");
    Serial.println("======================");
    Serial.println("Problem: Touch not detected");
    Serial.println("  • Check jumper wire connection to GPIO4");
    Serial.println("  • Ensure proper grounding");
    Serial.println("  • Try recalibrating (reset the board)");
    Serial.println("  • Check for electromagnetic interference");
    Serial.println();
    Serial.println("Problem: LED not working");
    Serial.println("  • Verify LED polarity (long leg = anode)");
    Serial.println("  • Check resistor value (330Ω recommended)");
    Serial.println("  • Ensure proper power supply");
    Serial.println();
    Serial.println("Problem: Unstable readings");
    Serial.println("  • Use shorter touch wire");
    Serial.println("  • Move away from WiFi routers");
    Serial.println("  • Ensure stable power supply");
    Serial.println();
}
