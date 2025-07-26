/*  05.041 - ESP32-S3 Analog Input with Potentiometer (Comprehensive Version)
 * 
 * Course: IoT Development with ESP32-S3
 * Target Hardware: ESP32-S3-DevKitC-1 v1.1
 * Module: ESP32-S3-WROOM-1-N16R16V
 * 
 * This comprehensive sketch demonstrates how to read analog values from a potentiometer
 * using the ESP32-S3's 12-bit ADC. The program provides multiple output formats,
 * statistical analysis, and educational features to help students understand analog
 * input processing on embedded systems.
 * 
 * HARDWARE INFORMATION:
 * =====================
 * ESP32-S3-DevKitC-1 v1.1 Specifications:
 * - Dual-core Xtensa LX7 processor (240 MHz)
 * - 16 MB Flash memory, 16 MB PSRAM
 * - 12-bit SAR ADC with multiple channels
 * - ADC1: GPIO1-GPIO10 (recommended for analog input)
 * - ADC2: GPIO11-GPIO20 (shared with WiFi, avoid when WiFi active)
 * - USB-C connector for programming and power
 * 
 * CIRCUIT DIAGRAM:
 * ================
 *                     ESP32-S3-DevKitC-1
 *                    ┌─────────────────────┐
 *                    │                     │
 *               ┌────┤ 3.3V            GND ├────┐
 *               │    │                     │    │
 *               │    │                     │    │
 *               │    │ GPIO1 (ADC1_CH0)    │    │
 *               │    │                     │    │
 *               │    └─────────────────────┘    │
 *               │             │                  │
 *               │             │                  │
 *           ┌───┴───┐        │              ┌───┴───┐
 *           │   1   │        │              │   3   │
 *           │       │        │              │       │
 *           │   P   │◄───────┼──────────────┤   G   │
 *           │   O   │        │              │   N   │
 *           │   T   │        │              │   D   │
 *           │       │◄───────┘              │       │
 *           │   2   │                       │       │
 *           └───────┘                       └───────┘
 *          10kΩ Potentiometer              Ground Rail
 * 
 * COMPONENTS:
 * ===========
 * 1x ESP32-S3-DevKitC-1 v1.1 Development Board
 * 1x 10kΩ Linear Potentiometer (3-pin)
 * 1x Breadboard (half-size or full-size)
 * 3x Male-to-Male Jumper Wires
 * 
 * Alternative Potentiometer Values: 1kΩ - 100kΩ (10kΩ recommended)
 * 
 * GPIO ASSIGNMENTS:
 * =================
 * GPIO1  (ADC1_CH0) - Potentiometer wiper (middle pin)
 * 3.3V              - Potentiometer pin 1 (high reference)
 * GND               - Potentiometer pin 3 (low reference)
 * 
 * TECHNICAL NOTES:
 * ================
 * - ESP32-S3 ADC is 12-bit (0-4095) compared to ESP32's variable resolution
 * - ADC1 channels are preferred as they don't conflict with WiFi
 * - GPIO35, GPIO36, GPIO37 are RESERVED for internal SPI flash/PSRAM
 * - ADC reference voltage is 3.3V (actual voltage may vary ±10%)
 * - ADC has built-in attenuation settings for different voltage ranges
 * - Default attenuation covers 0-3.3V range
 * 
 * IDE REQUIREMENTS:
 * =================
 * - Arduino IDE 2.0+
 * - ESP32 Arduino Core v3.0+
 * - Board: "ESP32S3 Dev Module" (CRITICAL: Must select ESP32-S3 board!)
 * - USB CDC On Boot: "Enabled" (for Serial communication)
 * 
 * IMPORTANT: Make sure you have selected "ESP32S3 Dev Module" as your board type
 * in Arduino IDE. Do NOT compile this for Arduino, SAMD, or other platforms.
 * 
 * Created: March 26 2019 by Peter Dalmaris (Original ESP32 version)
 * Updated: December 2024 for ESP32-S3 compatibility and educational enhancement
 */

// ===========================================================================================
//                                    CONFIGURATION CONSTANTS
// ===========================================================================================

// Hardware Configuration
const uint8_t POT_GPIO = 1;                    // GPIO1 (ADC1_CH0) - Potentiometer input
const uint8_t ADC_BITS = 12;                   // 12-bit ADC resolution (0-4095)
const uint16_t ADC_MAX_VALUE = 4095;           // Maximum ADC reading (2^12 - 1)
const float REFERENCE_VOLTAGE = 3.3;          // ESP32-S3 reference voltage

// Serial Communication Configuration
const uint32_t SERIAL_BAUD_RATE = 115200;     // Higher baud rate for faster data transfer
const uint16_t SERIAL_TIMEOUT_MS = 5000;      // Serial initialization timeout

// Measurement Configuration
const uint16_t SAMPLING_DELAY_MS = 100;       // Delay between measurements (100ms = 10Hz)
const uint8_t SAMPLES_FOR_AVERAGE = 10;       // Number of samples for moving average
const uint16_t NOISE_THRESHOLD = 5;           // Minimum change to report (reduces noise)

// Display Configuration
const unsigned int VOLTAGE_PRECISION = 3;     // Decimal places for voltage display
const unsigned int PERCENTAGE_PRECISION = 1;  // Decimal places for percentage display

// ===========================================================================================
//                                    GLOBAL VARIABLES
// ===========================================================================================

// Measurement Variables
uint16_t currentReading = 0;                  // Current ADC reading
uint16_t previousReading = 0;                 // Previous reading for change detection
uint16_t movingAverageBuffer[SAMPLES_FOR_AVERAGE]; // Buffer for moving average calculation
uint8_t bufferIndex = 0;                      // Current buffer position
uint32_t measurementCount = 0;                // Total number of measurements taken

// Statistical Variables
uint16_t minimumReading = ADC_MAX_VALUE;      // Minimum reading recorded
uint16_t maximumReading = 0;                  // Maximum reading recorded
uint32_t readingSum = 0;                      // Sum for average calculation

// Timing Variables
uint32_t lastMeasurementTime = 0;             // Timestamp of last measurement
uint32_t startTime = 0;                       // System start timestamp

// System Status
bool adcInitialized = false;                  // ADC initialization status
bool serialInitialized = false;               // Serial initialization status

// ===========================================================================================
//                                    SETUP FUNCTION
// ===========================================================================================

void setup() {
    // Record system start time
    startTime = millis();
    
    // Initialize Serial Communication
    initializeSerial();
    
    // Initialize ADC
    initializeADC();
    
    // Initialize measurement buffer
    initializeBuffer();
    
    // Display startup information
    displayStartupInfo();
    
    // Perform initial system check
    performSystemCheck();
    
    Serial.println("\n" + createSeparator('=', 60));
    Serial.println("SYSTEM READY - Starting measurements...");
    Serial.println(createSeparator('=', 60) + "\n");
    
    // Display column headers
    displayDataHeaders();
}

// ===========================================================================================
//                                    MAIN LOOP FUNCTION
// ===========================================================================================

void loop() {
    // Check if it's time for a new measurement
    if (millis() - lastMeasurementTime >= SAMPLING_DELAY_MS) {
        // Take ADC measurement
        takeMeasurement();
        
        // Update statistics
        updateStatistics();
        
        // Check for significant change (reduces noise in output)
        if (hasSignificantChange()) {
            // Display measurements in multiple formats
            displayMeasurements();
            
            // Update previous reading
            previousReading = currentReading;
        }
        
        // Update timestamp
        lastMeasurementTime = millis();
        
        // Increment measurement counter
        measurementCount++;
        
        // Display periodic statistics (every 100 measurements)
        if (measurementCount % 100 == 0) {
            displayStatistics();
        }
    }
    
    // Small delay to prevent excessive CPU usage
    delay(1);
}

// ===========================================================================================
//                                    INITIALIZATION FUNCTIONS
// ===========================================================================================

/**
 * Initialize Serial communication with error handling
 */
void initializeSerial() {
    Serial.begin(SERIAL_BAUD_RATE);
    
    // Wait for Serial to initialize with timeout
    uint32_t serialStartTime = millis();
    while (!Serial && (millis() - serialStartTime < SERIAL_TIMEOUT_MS)) {
        delay(10);
    }
    
    if (Serial) {
        serialInitialized = true;
        Serial.println("\n" + createSeparator('=', 60));
        Serial.println("ESP32-S3 ANALOG INPUT DEMONSTRATION");
        Serial.println("Course: IoT Development with ESP32-S3");
        Serial.println("Lesson: 05.041 - Potentiometer Reading");
        Serial.println(createSeparator('=', 60));
    } else {
        // Serial failed to initialize - system will continue but without output
        serialInitialized = false;
    }
}

/**
 * Initialize ADC with proper configuration for ESP32-S3
 */
void initializeADC() {
    // Set ADC resolution to 12-bit (0-4095)
    analogReadResolution(ADC_BITS);
    
    // The ESP32-S3 automatically configures attenuation
    // Default attenuation (0dB) provides 0-1V range
    // For 0-3.3V range, we rely on the default configuration
    
    // Test ADC functionality
    uint16_t testReading = analogRead(POT_GPIO);
    
    if (testReading >= 0 && testReading <= ADC_MAX_VALUE) {
        adcInitialized = true;
        if (serialInitialized) {
            Serial.println("✓ ADC initialized successfully");
            Serial.println("  - Resolution: " + String(ADC_BITS) + " bits");
            Serial.println("  - Range: 0-" + String(ADC_MAX_VALUE));
            Serial.println("  - Reference: " + String(REFERENCE_VOLTAGE, 1) + "V");
            Serial.println("  - Input Pin: GPIO" + String(POT_GPIO) + " (ADC1_CH0)");
        }
    } else {
        adcInitialized = false;
        if (serialInitialized) {
            Serial.println("✗ ADC initialization failed!");
            Serial.println("  Check wiring and component connections.");
        }
    }
}

/**
 * Initialize the moving average buffer with current readings
 */
void initializeBuffer() {
    if (adcInitialized) {
        uint16_t initialReading = analogRead(POT_GPIO);
        
        // Fill buffer with initial reading
        for (uint8_t i = 0; i < SAMPLES_FOR_AVERAGE; i++) {
            movingAverageBuffer[i] = initialReading;
        }
        
        currentReading = initialReading;
        previousReading = initialReading;
        
        if (serialInitialized) {
            Serial.println("✓ Moving average buffer initialized");
            Serial.println("  - Buffer size: " + String(SAMPLES_FOR_AVERAGE) + " samples");
            Serial.println("  - Initial value: " + String(initialReading));
        }
    }
}

/**
 * Display comprehensive startup information
 */
void displayStartupInfo() {
    if (!serialInitialized) return;
    
    Serial.println("\nHARDWARE INFORMATION:");
    Serial.println("┌─────────────────────────────────────────┐");
    Serial.println("│ ESP32-S3-DevKitC-1 v1.1                │");
    Serial.println("│ Module: ESP32-S3-WROOM-1-N16R16V        │");
    Serial.println("│ Flash: 16MB, PSRAM: 16MB                │");
    Serial.println("│ ADC: 12-bit SAR, Dual Core LX7         │");
    Serial.println("└─────────────────────────────────────────┘");
    
    Serial.println("\nCIRCUITRY:");
    Serial.println("┌─────────────────────────────────────────┐");
    Serial.println("│ Potentiometer → GPIO1 (ADC1_CH0)       │");
    Serial.println("│ 3.3V → Pot Pin 1, GND → Pot Pin 3      │");
    Serial.println("│ Expected Range: 0V - 3.3V              │");
    Serial.println("└─────────────────────────────────────────┘");
    
    Serial.println("\nMEASUREMENT SETTINGS:");
    Serial.println("• Sampling Rate: " + String(1000/SAMPLING_DELAY_MS) + " Hz");
    Serial.println("• Moving Average: " + String(SAMPLES_FOR_AVERAGE) + " samples");
    Serial.println("• Noise Threshold: " + String(NOISE_THRESHOLD) + " ADC units");
    Serial.println("• Voltage Precision: " + String(VOLTAGE_PRECISION) + " decimal places");
}

/**
 * Perform system self-check and display results
 */
void performSystemCheck() {
    if (!serialInitialized) return;
    
    Serial.println("\nSYSTEM CHECK:");
    Serial.println("─────────────");
    
    // Check Serial communication
    Serial.println("Serial Communication: " + String(serialInitialized ? "✓ OK" : "✗ FAILED"));
    
    // Check ADC initialization
    Serial.println("ADC Configuration:    " + String(adcInitialized ? "✓ OK" : "✗ FAILED"));
    
    // Check GPIO configuration
    bool gpioOK = (POT_GPIO >= 1 && POT_GPIO <= 10); // Valid ADC1 range
    Serial.println("GPIO Assignment:      " + String(gpioOK ? "✓ OK" : "✗ FAILED"));
    
    // Test measurement
    if (adcInitialized) {
        uint16_t testValue = analogRead(POT_GPIO);
        bool measurementOK = (testValue >= 0 && testValue <= ADC_MAX_VALUE);
        Serial.println("Test Measurement:     " + String(measurementOK ? "✓ OK" : "✗ FAILED"));
        Serial.println("  Initial Reading: " + String(testValue) + " (" + 
                      String(adcToVoltage(testValue), VOLTAGE_PRECISION) + "V)");
    }
    
    // Overall system status
    bool systemOK = serialInitialized && adcInitialized;
    Serial.println("Overall Status:       " + String(systemOK ? "✓ READY" : "✗ ERROR"));
    
    if (!systemOK) {
        Serial.println("\n⚠ WARNING: System errors detected!");
        Serial.println("Please check connections and restart if necessary.");
    }
}

// ===========================================================================================
//                                    MEASUREMENT FUNCTIONS
// ===========================================================================================

/**
 * Take a new ADC measurement and update moving average
 */
void takeMeasurement() {
    if (!adcInitialized) return;
    
    // Read raw ADC value
    uint16_t rawReading = analogRead(POT_GPIO);
    
    // Ensure reading is within valid range
    rawReading = constrain(rawReading, 0, ADC_MAX_VALUE);
    
    // Update moving average buffer
    movingAverageBuffer[bufferIndex] = rawReading;
    bufferIndex = (bufferIndex + 1) % SAMPLES_FOR_AVERAGE;
    
    // Calculate moving average
    uint32_t sum = 0;
    for (uint8_t i = 0; i < SAMPLES_FOR_AVERAGE; i++) {
        sum += movingAverageBuffer[i];
    }
    
    currentReading = sum / SAMPLES_FOR_AVERAGE;
}

/**
 * Update statistical tracking variables
 */
void updateStatistics() {
    // Update min/max values
    if (currentReading < minimumReading) {
        minimumReading = currentReading;
    }
    if (currentReading > maximumReading) {
        maximumReading = currentReading;
    }
    
    // Update running sum for overall average
    readingSum += currentReading;
}

/**
 * Check if the current reading represents a significant change
 * @return true if change exceeds noise threshold
 */
bool hasSignificantChange() {
    return abs((int)currentReading - (int)previousReading) >= NOISE_THRESHOLD;
}

// ===========================================================================================
//                                    DISPLAY FUNCTIONS
// ===========================================================================================

/**
 * Display column headers for measurement data
 */
void displayDataHeaders() {
    if (!serialInitialized) return;
    
    Serial.println("Time(s) | ADC Raw | Voltage(V) | Percent(%) | Status");
    Serial.println("─────────────────────────────────────────────────────");
}

/**
 * Display current measurements in formatted columns
 */
void displayMeasurements() {
    if (!serialInitialized) return;
    
    // Calculate derived values
    float voltage = adcToVoltage(currentReading);
    float percentage = adcToPercentage(currentReading);
    float elapsedTime = (millis() - startTime) / 1000.0;
    
    // Format and display data
    String timeStr = String(elapsedTime, 1);
    String adcStr = String(currentReading);
    String voltageStr = String(voltage, VOLTAGE_PRECISION);
    String percentStr = String(percentage, PERCENTAGE_PRECISION);
    
    // Pad strings for alignment
    timeStr = padString(timeStr, 7);
    adcStr = padString(adcStr, 7);
    voltageStr = padString(voltageStr, 10);
    percentStr = padString(percentStr, 10);
    
    // Determine status
    String status = getReadingStatus(percentage);
    
    Serial.println(timeStr + " | " + adcStr + " | " + voltageStr + " | " + 
                   percentStr + " | " + status);
}

/**
 * Display comprehensive statistics
 */
void displayStatistics() {
    if (!serialInitialized || measurementCount == 0) return;
    
    Serial.println("\n" + createSeparator('-', 60));
    Serial.println("MEASUREMENT STATISTICS (Count: " + String(measurementCount) + ")");
    Serial.println(createSeparator('-', 60));
    
    // Calculate overall average
    float overallAverage = (float)readingSum / measurementCount;
    
    // Display statistics
    Serial.println("Minimum Reading:  " + String(minimumReading) + 
                   " (" + String(adcToVoltage(minimumReading), VOLTAGE_PRECISION) + "V)");
    Serial.println("Maximum Reading:  " + String(maximumReading) + 
                   " (" + String(adcToVoltage(maximumReading), VOLTAGE_PRECISION) + "V)");
    Serial.println("Current Reading:  " + String(currentReading) + 
                   " (" + String(adcToVoltage(currentReading), VOLTAGE_PRECISION) + "V)");
    Serial.println("Average Reading:  " + String(overallAverage, 1) + 
                   " (" + String(adcToVoltage(overallAverage), VOLTAGE_PRECISION) + "V)");
    
    // Display range information
    uint16_t range = maximumReading - minimumReading;
    Serial.println("Measurement Range: " + String(range) + " ADC units");
    
    // Display uptime
    float uptime = (millis() - startTime) / 1000.0;
    Serial.println("System Uptime:    " + String(uptime, 1) + " seconds");
    
    Serial.println(createSeparator('-', 60) + "\n");
}

// ===========================================================================================
//                                    UTILITY FUNCTIONS
// ===========================================================================================

/**
 * Convert ADC reading to voltage
 * @param adcValue Raw ADC reading (0-4095)
 * @return Voltage value (0.0-3.3V)
 */
float adcToVoltage(uint16_t adcValue) {
    return ((float)adcValue / ADC_MAX_VALUE) * REFERENCE_VOLTAGE;
}

/**
 * Convert ADC reading to percentage
 * @param adcValue Raw ADC reading (0-4095)
 * @return Percentage value (0.0-100.0%)
 */
float adcToPercentage(uint16_t adcValue) {
    return ((float)adcValue / ADC_MAX_VALUE) * 100.0;
}

/**
 * Get status description based on reading percentage
 * @param percentage Reading as percentage (0-100)
 * @return Status string
 */
String getReadingStatus(float percentage) {
    if (percentage < 10) return "Very Low";
    else if (percentage < 25) return "Low";
    else if (percentage < 75) return "Medium";
    else if (percentage < 90) return "High";
    else return "Very High";
}

/**
 * Pad string to specified width for alignment
 * @param str Input string
 * @param width Desired width
 * @return Padded string
 */
String padString(String str, uint8_t width) {
    while (str.length() < width) {
        str = " " + str;
    }
    return str;
}

/**
 * Create separator line for display formatting
 * @param character Character to repeat
 * @param count Number of repetitions
 * @return Separator string
 */
String createSeparator(char character, uint8_t count) {
    String result = "";
    for (uint8_t i = 0; i < count; i++) {
        result += character;
    }
    return result;
}
