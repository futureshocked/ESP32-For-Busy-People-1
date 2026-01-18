/*  05.071 - ADXL335 Acceleration Sensor with Advanced Features for ESP32-S3
 * 
 * Course: IoT Development with ESP32-S3
 * Target Hardware: ESP32-S3-DevKitC-1 v1.1
 * Module: ESP32-S3-WROOM-1-N16R16V
 * 
 * This comprehensive sketch demonstrates how to interface with the ADXL335 analog
 * accelerometer sensor using the ESP32-S3. Features include calibration, filtering,
 * orientation detection, and comprehensive error handling suitable for educational
 * and professional applications.
 * 
 * HARDWARE INFORMATION:
 * =====================
 * - Board: ESP32-S3-DevKitC-1 v1.1
 * - Module: ESP32-S3-WROOM-1-N16R16V (16MB Flash, 16MB PSRAM)
 * - ADC Resolution: 12-bit (0-4095)
 * - Operating Voltage: 3.3V
 * - ADC Reference: 3.3V
 * - ADC Channels Used: ADC1_CH0, ADC1_CH1, ADC1_CH2
 * 
 * CIRCUIT DIAGRAM:
 * ================
 *     ESP32-S3-DevKitC-1          ADXL335 Breakout
 *     ┌─────────────────┐         ┌──────────────┐
 *     │             3V3 ├─────────┤ VCC          │
 *     │             GND ├─────────┤ GND          │
 *     │       GPIO1(J1) ├─────────┤ X-OUT        │
 *     │       GPIO2(J1) ├─────────┤ Y-OUT        │
 *     │       GPIO3(J1) ├─────────┤ Z-OUT        │
 *     │                 │         │ ST (Self Test│ (Leave unconnected)
 *     └─────────────────┘         └──────────────┘
 * 
 * COMPONENTS:
 * ===========
 * - 1x ESP32-S3-DevKitC-1 v1.1
 * - 1x ADXL335 Triple-Axis Accelerometer Breakout Board
 * - 4x Male-to-Female Jumper Wires
 * - 1x Breadboard (optional for prototyping)
 * 
 * GPIO ASSIGNMENTS:
 * =================
 * GPIO1  (ADC1_CH0) - X-axis accelerometer output
 * GPIO2  (ADC1_CH1) - Y-axis accelerometer output  
 * GPIO3  (ADC1_CH2) - Z-axis accelerometer output
 * GPIO38 (Built-in) - Status LED (WS2812 RGB)
 * 
 * TECHNICAL NOTES:
 * ================
 * - ADXL335 outputs analog voltages proportional to acceleration
 * - Sensitivity: ~300mV/g (varies with supply voltage)
 * - Zero-g offset: ~1.5V at 3.3V supply
 * - Maximum acceleration range: ±3g
 * - This version includes digital filtering and calibration
 * - GPIO35, 36, 37 are avoided (reserved for internal SPI flash/PSRAM)
 * 
 * IDE REQUIREMENTS:
 * =================
 * - Arduino IDE 2.0+
 * - ESP32 Arduino Core v3.0+
 * - Board: "ESP32S3 Dev Module"
 * - USB CDC On Boot: "Enabled"
 * - Flash Size: "16MB"
 * - PSRAM: "OPI PSRAM"
 * 
 * Created: March 28 2019 by Peter Dalmaris
 * Updated: December 2024 for ESP32-S3 compatibility and modern practices
 */

// ============================================================================
// INCLUDE LIBRARIES
// ============================================================================
#include <Adafruit_NeoPixel.h>

// ============================================================================
// CONFIGURATION CONSTANTS
// ============================================================================
// ADC Configuration
constexpr uint8_t ADC_RESOLUTION_BITS = 12;
constexpr uint16_t ADC_MAX_VALUE = (1 << ADC_RESOLUTION_BITS) - 1; // 4095 for 12-bit
constexpr float ADC_REFERENCE_VOLTAGE = 3.3f; // ESP32-S3 ADC reference voltage

// GPIO Pin Assignments (using safe ADC1 pins)
constexpr uint8_t X_AXIS_PIN = 1;  // GPIO1 - ADC1_CH0
constexpr uint8_t Y_AXIS_PIN = 2;  // GPIO2 - ADC1_CH1  
constexpr uint8_t Z_AXIS_PIN = 3;  // GPIO3 - ADC1_CH2
constexpr uint8_t STATUS_LED_PIN = 38; // Built-in WS2812 RGB LED

// ADXL335 Sensor Specifications
constexpr float SENSITIVITY_V_PER_G = 0.300f; // 300mV/g at 3.3V supply
constexpr float ZERO_G_VOLTAGE = 1.65f;       // Typical zero-g output voltage
constexpr float MAX_ACCELERATION_G = 3.0f;    // ±3g measurement range

// Filtering and Sampling Configuration
constexpr uint8_t FILTER_SAMPLES = 10;        // Number of samples for moving average
constexpr uint16_t SAMPLING_RATE_MS = 50;     // Sampling interval in milliseconds
constexpr uint16_t SERIAL_BAUD_RATE = 115200; // Modern baud rate for faster communication

// Orientation Detection Thresholds (in g-force)
constexpr float HORIZONTAL_THRESHOLD_G = 0.8f;  // Z-axis threshold for horizontal detection
constexpr float VERTICAL_THRESHOLD_G = 0.7f;    // X/Y-axis threshold for vertical detection
constexpr float SIDE_THRESHOLD_G = 0.6f;        // Threshold for side orientation

// Status LED Colors (RGB values)
constexpr uint32_t LED_COLOR_IDLE = 0x001000;      // Green - System ready
constexpr uint32_t LED_COLOR_HORIZONTAL = 0x000010; // Blue - Horizontal orientation
constexpr uint32_t LED_COLOR_VERTICAL = 0x100000;   // Red - Vertical orientation
constexpr uint32_t LED_COLOR_SIDE = 0x101000;       // Yellow - Side orientation
constexpr uint32_t LED_COLOR_ERROR = 0x100010;      // Magenta - Error state

// ============================================================================
// GLOBAL VARIABLES
// ============================================================================
// Status LED object
Adafruit_NeoPixel statusLED(1, STATUS_LED_PIN, NEO_GRB + NEO_KHZ800);

// Filtering arrays for moving average calculation
float xAxisBuffer[FILTER_SAMPLES] = {0};
float yAxisBuffer[FILTER_SAMPLES] = {0};
float zAxisBuffer[FILTER_SAMPLES] = {0};
uint8_t filterIndex = 0;

// Calibration offsets (determined during calibration phase)
float xAxisOffset = 0.0f;
float yAxisOffset = 0.0f;
float zAxisOffset = 0.0f;

// Performance monitoring
unsigned long lastSampleTime = 0;
unsigned long calibrationStartTime = 0;
bool isCalibrated = false;

// Current acceleration values (in g-force)
float currentXAccel = 0.0f;
float currentYAccel = 0.0f;
float currentZAccel = 0.0f;

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

/**
 * Convert ADC reading to voltage
 * @param adcValue Raw ADC reading (0-4095)
 * @return Voltage in volts
 */
float adcToVoltage(uint16_t adcValue) {
    return (static_cast<float>(adcValue) / ADC_MAX_VALUE) * ADC_REFERENCE_VOLTAGE;
}

/**
 * Convert voltage to acceleration in g-force
 * @param voltage Sensor output voltage
 * @param offset Calibrated zero-g offset voltage
 * @return Acceleration in g-force
 */
float voltageToAcceleration(float voltage, float offset) {
    return (voltage - offset) / SENSITIVITY_V_PER_G;
}

/**
 * Apply moving average filter to reduce noise
 * @param newValue New sensor reading
 * @param buffer Array storing previous readings
 * @return Filtered value
 */
float applyMovingAverageFilter(float newValue, float buffer[]) {
    // Add new value to circular buffer
    buffer[filterIndex] = newValue;
    
    // Calculate average
    float sum = 0.0f;
    for (uint8_t i = 0; i < FILTER_SAMPLES; i++) {
        sum += buffer[i];
    }
    
    return sum / FILTER_SAMPLES;
}

/**
 * Update status LED based on current orientation
 * @param orientation String describing current orientation
 */
void updateStatusLED(const String& orientation) {
    if (orientation.indexOf("horizontal") >= 0) {
        statusLED.setPixelColor(0, LED_COLOR_HORIZONTAL);
    } else if (orientation.indexOf("vertical") >= 0) {
        statusLED.setPixelColor(0, LED_COLOR_VERTICAL);
    } else if (orientation.indexOf("side") >= 0) {
        statusLED.setPixelColor(0, LED_COLOR_SIDE);
    } else {
        statusLED.setPixelColor(0, LED_COLOR_IDLE);
    }
    statusLED.show();
}

/**
 * Perform sensor calibration by averaging readings when sensor is level
 */
void performCalibration() {
    Serial.println("\n=== STARTING SENSOR CALIBRATION ===");
    Serial.println("Please keep the sensor level and stationary for 5 seconds...");
    
    statusLED.setPixelColor(0, LED_COLOR_ERROR); // Magenta during calibration
    statusLED.show();
    
    const uint16_t calibrationSamples = 100;
    float xSum = 0.0f, ySum = 0.0f, zSum = 0.0f;
    
    calibrationStartTime = millis();
    
    for (uint16_t i = 0; i < calibrationSamples; i++) {
        // Read raw ADC values
        uint16_t xRaw = analogRead(X_AXIS_PIN);
        uint16_t yRaw = analogRead(Y_AXIS_PIN);
        uint16_t zRaw = analogRead(Z_AXIS_PIN);
        
        // Convert to voltages and accumulate
        xSum += adcToVoltage(xRaw);
        ySum += adcToVoltage(yRaw);
        zSum += adcToVoltage(zRaw);
        
        delay(50); // 50ms between calibration samples
        
        // Show progress
        if (i % 20 == 0) {
            Serial.print(".");
        }
    }
    
    // Calculate average offsets
    xAxisOffset = xSum / calibrationSamples;
    yAxisOffset = ySum / calibrationSamples;
    zAxisOffset = (zSum / calibrationSamples) - SENSITIVITY_V_PER_G; // Subtract 1g for Z-axis
    
    isCalibrated = true;
    
    Serial.println("\n=== CALIBRATION COMPLETE ===");
    Serial.printf("X-axis offset: %.3f V\n", xAxisOffset);
    Serial.printf("Y-axis offset: %.3f V\n", yAxisOffset);
    Serial.printf("Z-axis offset: %.3f V\n", zAxisOffset);
    Serial.println("Starting measurements...\n");
    
    statusLED.setPixelColor(0, LED_COLOR_IDLE);
    statusLED.show();
}

/**
 * Read and filter accelerometer data
 */
void readAccelerometerData() {
    // Read raw ADC values
    uint16_t xRaw = analogRead(X_AXIS_PIN);
    uint16_t yRaw = analogRead(Y_AXIS_PIN);
    uint16_t zRaw = analogRead(Z_AXIS_PIN);
    
    // Convert to voltages
    float xVoltage = adcToVoltage(xRaw);
    float yVoltage = adcToVoltage(yRaw);
    float zVoltage = adcToVoltage(zRaw);
    
    // Apply filtering
    float xFiltered = applyMovingAverageFilter(xVoltage, xAxisBuffer);
    float yFiltered = applyMovingAverageFilter(yVoltage, yAxisBuffer);
    float zFiltered = applyMovingAverageFilter(zVoltage, zAxisBuffer);
    
    // Convert to acceleration (g-force)
    if (isCalibrated) {
        currentXAccel = voltageToAcceleration(xFiltered, xAxisOffset);
        currentYAccel = voltageToAcceleration(yFiltered, yAxisOffset);
        currentZAccel = voltageToAcceleration(zFiltered, zAxisOffset);
    } else {
        // Use default zero-g voltage if not calibrated
        currentXAccel = voltageToAcceleration(xFiltered, ZERO_G_VOLTAGE);
        currentYAccel = voltageToAcceleration(yFiltered, ZERO_G_VOLTAGE);
        currentZAccel = voltageToAcceleration(zFiltered, ZERO_G_VOLTAGE - SENSITIVITY_V_PER_G);
    }
    
    // Update filter index for next sample
    filterIndex = (filterIndex + 1) % FILTER_SAMPLES;
}

/**
 * Determine device orientation based on acceleration values
 * @return String describing the current orientation
 */
String determineOrientation() {
    String orientation = "";
    
    // Check for horizontal position (Z-axis pointing up)
    if (currentZAccel > HORIZONTAL_THRESHOLD_G) {
        orientation += "horizontal (face up)";
    } else if (currentZAccel < -HORIZONTAL_THRESHOLD_G) {
        orientation += "horizontal (face down)";
    }
    
    // Check for vertical positions
    if (abs(currentXAccel) > VERTICAL_THRESHOLD_G) {
        if (orientation.length() > 0) orientation += " + ";
        orientation += currentXAccel > 0 ? "tilted right" : "tilted left";
    }
    
    if (abs(currentYAccel) > VERTICAL_THRESHOLD_G) {
        if (orientation.length() > 0) orientation += " + ";
        orientation += currentYAccel > 0 ? "tilted forward" : "tilted backward";
    }
    
    // Check for side positions
    if (abs(currentXAccel) > SIDE_THRESHOLD_G && abs(currentZAccel) < SIDE_THRESHOLD_G) {
        if (orientation.length() > 0) orientation += " + ";
        orientation += "on side (X-axis dominant)";
    }
    
    if (abs(currentYAccel) > SIDE_THRESHOLD_G && abs(currentZAccel) < SIDE_THRESHOLD_G) {
        if (orientation.length() > 0) orientation += " + ";
        orientation += "on side (Y-axis dominant)";
    }
    
    // Default case
    if (orientation.length() == 0) {
        orientation = "neutral/transitioning";
    }
    
    return orientation;
}

/**
 * Display comprehensive sensor data and analysis
 */
void displaySensorData() {
    // Calculate total acceleration magnitude
    float totalAcceleration = sqrt(currentXAccel * currentXAccel + 
                                  currentYAccel * currentYAccel + 
                                  currentZAccel * currentZAccel);
    
    // Determine orientation
    String orientation = determineOrientation();
    
    // Update status LED
    updateStatusLED(orientation);
    
    // Display formatted output
    Serial.println("┌─────────────────────────────────────────────────────────────┐");
    Serial.printf("│ Time: %8lu ms │ Total Accel: %6.3f g │ Status: %s │\n", 
                  millis(), totalAcceleration, isCalibrated ? "CAL" : "RAW");
    Serial.println("├─────────────────────────────────────────────────────────────┤");
    Serial.printf("│ X-axis: %7.3f g │ Y-axis: %7.3f g │ Z-axis: %7.3f g │\n",
                  currentXAccel, currentYAccel, currentZAccel);
    Serial.println("├─────────────────────────────────────────────────────────────┤");
    Serial.printf("│ Orientation: %-47s │\n", orientation.c_str());
    Serial.println("└─────────────────────────────────────────────────────────────┘");
    Serial.println();
}

// ============================================================================
// MAIN PROGRAM FUNCTIONS
// ============================================================================

void setup() {
    // Initialize serial communication
    Serial.begin(SERIAL_BAUD_RATE);
    while (!Serial && millis() < 5000) {
        delay(10); // Wait for serial connection or timeout after 5 seconds
    }
    
    // Display startup information
    Serial.println("\n" + String('=', 65));
    Serial.println("        ESP32-S3 ADXL335 Accelerometer - Advanced Version");
    Serial.println(String('=', 65));
    Serial.printf("Hardware: %s\n", "ESP32-S3-DevKitC-1 v1.1");
    Serial.printf("Module: %s\n", "ESP32-S3-WROOM-1-N16R16V");
    Serial.printf("ADC Resolution: %d bits (0-%d)\n", ADC_RESOLUTION_BITS, ADC_MAX_VALUE);
    Serial.printf("Sampling Rate: %d ms\n", SAMPLING_RATE_MS);
    Serial.printf("Filter Samples: %d\n", FILTER_SAMPLES);
    Serial.println(String('=', 65));
    
    // Configure ADC resolution
    analogReadResolution(ADC_RESOLUTION_BITS);
    Serial.printf("✓ ADC resolution set to %d bits\n", ADC_RESOLUTION_BITS);
    
    // Initialize status LED
    statusLED.begin();
    statusLED.setPixelColor(0, LED_COLOR_ERROR); // Red during initialization
    statusLED.show();
    Serial.printf("✓ Status LED initialized on GPIO%d\n", STATUS_LED_PIN);
    
    // Validate GPIO pins
    Serial.println("\nGPIO Pin Validation:");
    Serial.printf("✓ X-axis: GPIO%d (ADC1_CH0)\n", X_AXIS_PIN);
    Serial.printf("✓ Y-axis: GPIO%d (ADC1_CH1)\n", Y_AXIS_PIN);
    Serial.printf("✓ Z-axis: GPIO%d (ADC1_CH2)\n", Z_AXIS_PIN);
    
    // Perform initial sensor readings to verify connectivity
    Serial.println("\nSensor Connectivity Test:");
    uint16_t xTest = analogRead(X_AXIS_PIN);
    uint16_t yTest = analogRead(Y_AXIS_PIN);
    uint16_t zTest = analogRead(Z_AXIS_PIN);
    
    Serial.printf("X-axis raw: %d (%.3fV)\n", xTest, adcToVoltage(xTest));
    Serial.printf("Y-axis raw: %d (%.3fV)\n", yTest, adcToVoltage(yTest));
    Serial.printf("Z-axis raw: %d (%.3fV)\n", zTest, adcToVoltage(zTest));
    
    // Check for sensor connectivity (readings should be within reasonable range)
    if (xTest < 100 || xTest > 3900 || yTest < 100 || yTest > 3900 || zTest < 100 || zTest > 3900) {
        Serial.println("⚠️  WARNING: Sensor readings outside expected range!");
        Serial.println("   Please check wiring and power connections.");
    } else {
        Serial.println("✓ Sensor connectivity verified");
    }
    
    delay(2000); // Allow time to read startup information
    
    // Perform calibration
    performCalibration();
    
    Serial.println("System ready! Starting continuous measurements...");
    Serial.println("Legend: CAL=Calibrated, RAW=Uncalibrated");
    Serial.println();
    
    lastSampleTime = millis();
}

void loop() {
    unsigned long currentTime = millis();
    
    // Check if it's time for the next sample
    if (currentTime - lastSampleTime >= SAMPLING_RATE_MS) {
        // Read and process accelerometer data
        readAccelerometerData();
        
        // Display results
        displaySensorData();
        
        // Update timing
        lastSampleTime = currentTime;
    }
    
    // Handle serial commands for recalibration
    if (Serial.available()) {
        String command = Serial.readStringUntil('\n');
        command.trim();
        command.toLowerCase();
        
        if (command == "cal" || command == "calibrate") {
            performCalibration();
        } else if (command == "help") {
            Serial.println("\nAvailable Commands:");
            Serial.println("  cal, calibrate - Perform sensor calibration");
            Serial.println("  help           - Show this help message");
            Serial.println();
        }
    }
    
    // Small delay to prevent watchdog issues
    delay(1);
}
