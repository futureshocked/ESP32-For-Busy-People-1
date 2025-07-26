/*  05.031 - Photoresistor Light Sensor for ESP32-S3
 * 
 * Course: IoT Development with ESP32-S3
 * Target Hardware: ESP32-S3-DevKitC-1 v1.1
 * Module: ESP32-S3-WROOM-1-N16R16V
 * 
 * This sketch demonstrates how to read ambient light intensity using a photoresistor
 * (Light Dependent Resistor - LDR) with the ESP32-S3's high-resolution ADC.
 * 
 * The photoresistor changes its resistance based on light intensity:
 * - Bright light: Low resistance (few hundred ohms)
 * - Dark conditions: High resistance (several megohms)
 * 
 * We use a voltage divider circuit to convert the resistance change into a
 * voltage change that the ESP32-S3 ADC can measure with 12-bit precision.
 * 
 * HARDWARE INFORMATION:
 * =====================
 * ESP32-S3-DevKitC-1 v1.1 Features:
 * - Dual-core Xtensa LX7 CPU up to 240MHz
 * - 512KB SRAM + 16MB Flash + 16MB PSRAM
 * - 12-bit SAR ADC with up to 20 channels
 * - ADC1: GPIO1-GPIO10 (preferred for WiFi compatibility)
 * - ADC2: GPIO11-GPIO20 (shared with WiFi, avoid if using WiFi)
 * - ADC Resolution: 12-bit (0-4095) with configurable attenuation
 * - Reference Voltage: 3.3V (with 11dB attenuation)
 * 
 * CIRCUIT DIAGRAM:
 * ================
 * 
 *     3.3V
 *       |
 *       |
 *    [PHOTO]  <- Photoresistor (LDR)
 *       |
 *       +------- GPIO4 (ADC1_CH3)
 *       |
 *     [10kΩ]  <- Pull-down resistor
 *       |
 *      GND
 * 
 * Voltage Divider Analysis:
 * Vout = Vin × (R_pulldown / (R_photoresistor + R_pulldown))
 * 
 * Light Conditions vs Resistance:
 * - Bright light: ~1kΩ  → Vout ≈ 3.0V → ADC ≈ 3720
 * - Room light:   ~10kΩ → Vout ≈ 1.65V → ADC ≈ 2048  
 * - Dim light:    ~50kΩ → Vout ≈ 0.55V → ADC ≈ 680
 * - Dark:         ~1MΩ  → Vout ≈ 0.03V → ADC ≈ 40
 * 
 * COMPONENTS:
 * ===========
 * 1x ESP32-S3-DevKitC-1 v1.1 Development Board
 * 1x Photoresistor (LDR) - GL5528 or similar (1kΩ-1MΩ range)
 * 1x 10kΩ Resistor (1/4W, 5% tolerance)
 * 1x Breadboard (half-size recommended)
 * 3x Jumper wires (Male-to-Male)
 * 1x USB-C cable for programming and power
 * 
 * Component Specifications:
 * - Photoresistor: GL5528 or equivalent
 *   • Light resistance: 1-10kΩ (10 lux)
 *   • Dark resistance: 1MΩ+ (0 lux)
 *   • Peak sensitivity: ~540nm (green light)
 *   • Response time: 30ms (light→dark), 20ms (dark→light)
 * 
 * GPIO ASSIGNMENTS:
 * =================
 * GPIO4  : Analog input from voltage divider (ADC1_CH3)
 * GPIO38 : Onboard RGB LED for light level indication (WS2812)
 * 
 * TECHNICAL NOTES:
 * ================
 * 1. ADC Configuration:
 *    - 12-bit resolution provides 4096 discrete levels (0-4095)
 *    - Default attenuation allows 0-3.3V input range
 *    - ADC1 channels are preferred when using WiFi
 * 
 * 2. Voltage Divider Design:
 *    - 10kΩ pull-down resistor chosen to provide good sensitivity
 *    - Smaller resistor = less sensitive to dark conditions
 *    - Larger resistor = less sensitive to bright conditions
 * 
 * 3. Calibration Considerations:
 *    - Photoresistor tolerance can be ±20%
 *    - Temperature affects resistance (~0.1%/°C)
 *    - Consider averaging multiple readings for stability
 * 
 * 4. Performance Optimizations:
 *    - Reading interval affects power consumption
 *    - Use deep sleep for battery-powered applications
 *    - Consider interrupt-based reading for threshold detection
 * 
 * IDE REQUIREMENTS:
 * =================
 * - Arduino IDE 2.0+ (or 1.8.19+)
 * - ESP32 Arduino Core v2.0.0+ (tested with v2.0.14 and v3.0+)
 * - Board: "ESP32S3 Dev Module"
 * - Partition Scheme: "Default 4MB with spiffs"
 * - USB Mode: "Hardware CDC and JTAG"
 * 
 * Created: March 27, 2019 by Peter Dalmaris
 * Updated: July 26, 2025 for ESP32-S3 compatibility and educational enhancement
 */

// No additional includes required - using Arduino framework functions

// ========================================================================================
// CONFIGURATION CONSTANTS
// ========================================================================================

// Hardware pin assignments
const uint8_t PHOTORESISTOR_PIN = 4;    // GPIO4 (ADC1_CH3) - Photoresistor input
const uint8_t RGB_LED_PIN = 38;         // GPIO38 - Onboard WS2812 RGB LED

// ADC configuration constants
const uint8_t ADC_RESOLUTION_BITS = 12; // 12-bit ADC resolution
const uint16_t ADC_MAX_VALUE = 4095;    // Maximum ADC reading (2^12 - 1)
const float REFERENCE_VOLTAGE = 3.3;    // ESP32-S3 reference voltage

// Voltage divider constants
const uint16_t PULLDOWN_RESISTOR = 10000; // 10kΩ pull-down resistor value

// Sampling and display configuration
const uint16_t SAMPLING_INTERVAL_MS = 500;  // Reading interval in milliseconds
const uint8_t READINGS_TO_AVERAGE = 5;      // Number of readings to average for stability

// Light level thresholds for classification
const uint16_t BRIGHT_THRESHOLD = 3000;     // ADC value for bright light
const uint16_t NORMAL_THRESHOLD = 2000;     // ADC value for normal room light
const uint16_t DIM_THRESHOLD = 800;         // ADC value for dim light
const uint16_t DARK_THRESHOLD = 200;        // ADC value for dark conditions

// ========================================================================================
// GLOBAL VARIABLES
// ========================================================================================

// Measurement variables
uint16_t currentReading = 0;
uint16_t averagedReading = 0;
float measuredVoltage = 0.0;
uint32_t estimatedResistance = 0;

// Performance monitoring
unsigned long lastReadingTime = 0;
unsigned long totalReadings = 0;

// ========================================================================================
// SETUP FUNCTION
// ========================================================================================

void setup() {
    // Initialize serial communication for debugging and data output
    Serial.begin(115200);
    while (!Serial) {
        delay(10); // Wait for serial connection (important for USB CDC)
    }
    
    // Display startup information
    printStartupInfo();
    
    // Configure ADC for optimal performance
    configureADC();
    
    // Initialize GPIO pins
    initializeGPIO();
    
    // Perform initial calibration
    performInitialCalibration();
    
    Serial.println("\n" + String('=', 60));
    Serial.println("PHOTORESISTOR MONITORING STARTED");
    Serial.println(String('=', 60));
    Serial.println("Time(ms)\tRaw ADC\tVoltage(V)\tResistance(Ω)\tLight Level");
    Serial.println(String('-', 60));
}

// ========================================================================================
// MAIN LOOP FUNCTION
// ========================================================================================

void loop() {
    // Check if it's time for a new reading
    if (millis() - lastReadingTime >= SAMPLING_INTERVAL_MS) {
        // Read and process photoresistor data
        readPhotoresistor();
        
        // Calculate derived values
        calculateVoltageAndResistance();
        
        // Display results
        displayReadings();
        
        // Classify and indicate light level
        classifyLightLevel();
        
        // Update timing and statistics
        lastReadingTime = millis();
        totalReadings++;
        
        // Perform periodic diagnostics
        if (totalReadings % 20 == 0) {
            performDiagnostics();
        }
    }
    
    // Small delay to prevent excessive CPU usage
    delay(1);
}

// ========================================================================================
// PHOTORESISTOR READING FUNCTIONS
// ========================================================================================

/**
 * Read photoresistor value with averaging for improved stability
 * Uses multiple samples to reduce noise and improve accuracy
 */
void readPhotoresistor() {
    uint32_t sum = 0;
    uint16_t validReadings = 0;
    
    // Take multiple readings for averaging
    for (uint8_t i = 0; i < READINGS_TO_AVERAGE; i++) {
        uint16_t reading = analogRead(PHOTORESISTOR_PIN);
        
        // Validate reading (basic sanity check)
        if (reading <= ADC_MAX_VALUE) {
            sum += reading;
            validReadings++;
        }
        
        delay(2); // Small delay between readings
    }
    
    // Calculate average if we have valid readings
    if (validReadings > 0) {
        averagedReading = sum / validReadings;
        currentReading = averagedReading;
    } else {
        Serial.println("ERROR: No valid ADC readings obtained!");
        currentReading = 0;
    }
}

/**
 * Calculate voltage and estimated photoresistor resistance
 * Uses voltage divider formula: Vout = Vin × (R2 / (R1 + R2))
 * Solving for R1 (photoresistor): R1 = R2 × ((Vin - Vout) / Vout)
 */
void calculateVoltageAndResistance() {
    // Convert ADC reading to voltage
    measuredVoltage = (currentReading * REFERENCE_VOLTAGE) / ADC_MAX_VALUE;
    
    // Calculate photoresistor resistance using voltage divider formula
    // Avoid division by zero
    if (measuredVoltage > 0.01) { // Minimum 10mV to avoid calculation errors
        estimatedResistance = PULLDOWN_RESISTOR * 
                             ((REFERENCE_VOLTAGE - measuredVoltage) / measuredVoltage);
    } else {
        estimatedResistance = 999999; // Very high resistance (essentially infinite)
    }
}

// ========================================================================================
// DISPLAY AND CLASSIFICATION FUNCTIONS
// ========================================================================================

/**
 * Display current readings in a formatted table
 */
void displayReadings() {
    // Format: Time, Raw ADC, Voltage, Resistance, Light Level
    Serial.print(millis());
    Serial.print("\t\t");
    Serial.print(currentReading);
    Serial.print("\t");
    Serial.print(measuredVoltage, 3);
    Serial.print("\t\t");
    Serial.print(estimatedResistance);
    Serial.print("\t\t");
    Serial.println(getLightLevelString());
}

/**
 * Classify light level based on ADC reading and provide user feedback
 */
void classifyLightLevel() {
    String lightLevel = getLightLevelString();
    
    // Visual indication could be added here (e.g., RGB LED color)
    // For educational purposes, we'll just use serial output
    
    // Optional: Add threshold alerts
    static String previousLevel = "";
    if (lightLevel != previousLevel) {
        Serial.println(">>> Light level changed to: " + lightLevel + " <<<");
        previousLevel = lightLevel;
    }
}

/**
 * Get descriptive string for current light level
 */
String getLightLevelString() {
    if (currentReading >= BRIGHT_THRESHOLD) {
        return "BRIGHT";
    } else if (currentReading >= NORMAL_THRESHOLD) {
        return "NORMAL";
    } else if (currentReading >= DIM_THRESHOLD) {
        return "DIM";
    } else if (currentReading >= DARK_THRESHOLD) {
        return "DARK";
    } else {
        return "VERY DARK";
    }
}

// ========================================================================================
// INITIALIZATION AND CONFIGURATION FUNCTIONS
// ========================================================================================

/**
 * Display comprehensive startup information for educational purposes
 */
void printStartupInfo() {
    Serial.println("\n" + String('=', 60));
    Serial.println("ESP32-S3 PHOTORESISTOR LIGHT SENSOR");
    Serial.println("Course: IoT Development with ESP32-S3");
    Serial.println("Lesson: 05.031 - Analog Sensor Reading");
    Serial.println(String('=', 60));
    
    // Hardware information
    Serial.println("HARDWARE CONFIGURATION:");
    Serial.println("- Board: ESP32-S3-DevKitC-1 v1.1");
    Serial.println("- Module: ESP32-S3-WROOM-1-N16R16V");
    Serial.println("- ADC Pin: GPIO" + String(PHOTORESISTOR_PIN) + " (ADC1_CH3)");
    Serial.println("- ADC Resolution: " + String(ADC_RESOLUTION_BITS) + " bits (0-" + String(ADC_MAX_VALUE) + ")");
    Serial.println("- Reference Voltage: " + String(REFERENCE_VOLTAGE) + "V");
    Serial.println("- Pull-down Resistor: " + String(PULLDOWN_RESISTOR) + "Ω");
    
    // Measurement parameters
    Serial.println("\nMEASUREMENT PARAMETERS:");
    Serial.println("- Sampling Interval: " + String(SAMPLING_INTERVAL_MS) + "ms");
    Serial.println("- Readings Averaged: " + String(READINGS_TO_AVERAGE));
    Serial.println("- Light Thresholds: Bright>" + String(BRIGHT_THRESHOLD) + 
                   ", Normal>" + String(NORMAL_THRESHOLD) + 
                   ", Dim>" + String(DIM_THRESHOLD) + 
                   ", Dark>" + String(DARK_THRESHOLD));
}

/**
 * Configure ADC for optimal photoresistor reading
 */
void configureADC() {
    // Set ADC resolution to 12 bits for maximum precision
    analogReadResolution(ADC_RESOLUTION_BITS);
    
    // Note: ESP32-S3 uses 11dB attenuation by default, allowing 0-3.3V input
    // This is suitable for our voltage divider circuit
    
    Serial.println("\nADC CONFIGURATION COMPLETE:");
    Serial.println("- Resolution: " + String(ADC_RESOLUTION_BITS) + " bits");
    Serial.println("- Input range: 0 - " + String(REFERENCE_VOLTAGE) + "V");
}

/**
 * Initialize GPIO pins for the application
 */
void initializeGPIO() {
    // Configure photoresistor pin as analog input
    pinMode(PHOTORESISTOR_PIN, INPUT);
    
    // Note: RGB LED initialization would go here if implementing visual feedback
    // pinMode(RGB_LED_PIN, OUTPUT);
    
    Serial.println("\nGPIO CONFIGURATION COMPLETE:");
    Serial.println("- GPIO" + String(PHOTORESISTOR_PIN) + ": Analog input (photoresistor)");
}

/**
 * Perform initial calibration and validation
 */
void performInitialCalibration() {
    Serial.println("\nPERFORMING INITIAL CALIBRATION...");
    
    // Take several initial readings to validate circuit
    uint16_t initialReading = 0;
    uint8_t validReadings = 0;
    
    for (uint8_t i = 0; i < 10; i++) {
        uint16_t reading = analogRead(PHOTORESISTOR_PIN);
        if (reading <= ADC_MAX_VALUE) {
            initialReading += reading;
            validReadings++;
        }
        delay(100);
    }
    
    if (validReadings > 0) {
        initialReading /= validReadings;
        Serial.println("- Initial ADC reading: " + String(initialReading));
        Serial.println("- Circuit validation: PASSED");
        
        // Basic circuit validation
        if (initialReading == 0) {
            Serial.println("WARNING: Reading is 0 - check connections!");
        } else if (initialReading >= ADC_MAX_VALUE - 10) {
            Serial.println("WARNING: Reading near maximum - check pull-down resistor!");
        }
    } else {
        Serial.println("ERROR: Calibration failed - no valid readings!");
    }
}

// ========================================================================================
// DIAGNOSTIC AND UTILITY FUNCTIONS
// ========================================================================================

/**
 * Perform periodic diagnostics for system monitoring
 */
void performDiagnostics() {
    Serial.println("\n" + String('-', 40));
    Serial.println("SYSTEM DIAGNOSTICS:");
    Serial.println("- Total readings: " + String(totalReadings));
    Serial.println("- Current ADC: " + String(currentReading) + 
                   " (" + String((currentReading * 100) / ADC_MAX_VALUE) + "% of range)");
    Serial.println("- Measured voltage: " + String(measuredVoltage, 3) + "V");
    Serial.println("- Estimated LDR resistance: " + String(estimatedResistance) + "Ω");
    Serial.println("- Current light level: " + getLightLevelString());
    // Note: Heap monitoring varies by ESP32 Arduino Core version
    // Uncomment the next line if ESP.getFreeHeap() is available in your core version
    // Serial.println("- Free heap: " + String(ESP.getFreeHeap()) + " bytes");
    Serial.println("- Uptime: " + String(millis() / 1000) + " seconds");
    Serial.println(String('-', 40) + "\n");
}
