/*  05.061 - DHT22 Environment Sensor (Comprehensive Version) for ESP32-S3
 * 
 * Course: IoT Development with ESP32-S3
 * Target Hardware: ESP32-S3-DevKitC-1 v1.1
 * Module: ESP32-S3-WROOM-1-N16R16V
 * 
 * This comprehensive sketch demonstrates professional-grade implementation of DHT22 
 * temperature and humidity sensor interfacing with the ESP32-S3. It includes advanced
 * features like data validation, statistical analysis, calibration, and multiple
 * output formats suitable for IoT applications.
 * 
 * LEARNING OBJECTIVES:
 * ===================
 * - Understand digital sensor communication protocols
 * - Implement robust error handling and data validation
 * - Learn timing-critical sensor operations
 * - Apply statistical analysis to sensor data
 * - Develop professional IoT data logging practices
 * 
 * HARDWARE INFORMATION:
 * =====================
 * ESP32-S3-DevKitC-1 v1.1:
 * - Dual-core Xtensa LX7 @ 240MHz
 * - 16MB Flash, 16MB PSRAM
 * - 45 programmable GPIOs
 * - 12-bit ADC resolution
 * - Native USB support
 * 
 * DHT22 (AM2302) Sensor:
 * - Temperature range: -40°C to 80°C (±0.5°C accuracy)
 * - Humidity range: 0-99.9% RH (±2-5% accuracy)
 * - Single-wire digital interface
 * - 0.5Hz maximum sampling rate
 * - 3.3V-5V operating voltage
 * 
 * CIRCUIT DIAGRAM:
 * ================
 *                    ESP32-S3-DevKitC-1
 *                    ┌─────────────────┐
 *                    │              3V3├─────┬──────────┐
 *                    │                 │     │          │
 *                    │             GPIO2├─────┼────┐     │
 *                    │                 │     │    │     │
 *                    │              GND├─────┼────┼──┐  │
 *                    └─────────────────┘     │    │  │  │
 *                                            │    │  │  │
 *                     DHT22 (Front View)     │    │  │  │
 *                     ┌─────────────────┐    │    │  │  │
 *                     │  ┌───────────┐  │    │    │  │  │
 *                     │  │    :::    │  │    │    │  │  │
 *                     │  │    :::    │  │    │    │  │  │
 *                     │  │    :::    │  │    │    │  │  │
 *                     │  └───────────┘  │    │    │  │  │
 *                     └─────────────────┘    │    │  │  │
 *                      │ │ │ │               │    │  │  │
 *                     VCC│ │GND              │    │  │  │
 *                      │DATA│               │    │  │  │
 *                      │ │ │ │               │    │  │  │
 *                      1 2 3 4              │    │  │  │
 *                      │ │   │               │    │  │  │
 *                      │ └───┼───────────────┘    │  │  │
 *                      │     └─────────────────────┘  │  │
 *                      └─────────────────────────────┘  │
 *                                                       │
 *                     10kΩ Pull-up Resistor             │
 *                     ┌─────────VVV─────────────────────┘
 *                     │
 *                     └──── Connected between DATA and VCC
 * 
 * COMPONENTS:
 * ===========
 * 1x ESP32-S3-DevKitC-1 v1.1 development board
 * 1x DHT22 (AM2302) temperature and humidity sensor
 * 1x 10kΩ resistor (for pull-up, brown-black-orange-gold)
 * 1x Breadboard (half-size minimum)
 * 4x Jumper wires (male-to-male)
 * 1x USB-C cable for programming and power
 * 
 * GPIO ASSIGNMENTS:
 * =================
 * GPIO2  - DHT22 Data Pin (with 10kΩ pull-up)
 * 3V3    - DHT22 VCC (Power supply)
 * GND    - DHT22 GND (Ground)
 * Note: Pin 3 of DHT22 is not connected (NC)
 * 
 * TECHNICAL NOTES:
 * ================
 * - DHT22 requires 10kΩ pull-up resistor on data line
 * - Minimum 2 seconds between readings for stable operation
 * - Sensor needs 1-2 seconds to stabilize after power-on
 * - Communication uses proprietary 1-wire protocol (not Dallas 1-Wire)
 * - ESP32-S3 GPIO2 is safe for general I/O (avoid GPIO35-37)
 * - Data validation includes checksum verification
 * - Statistical smoothing reduces noise in measurements
 * 
 * IDE REQUIREMENTS:
 * =================
 * - Arduino IDE 2.0+
 * - ESP32 Arduino Core v3.0+
 * - Board: "ESP32S3 Dev Module"
 * - Libraries: DHTesp (install via Library Manager)
 * 
 * CALIBRATION & ACCURACY:
 * =======================
 * - Factory calibration is typically sufficient
 * - For higher accuracy, compare with reference sensor
 * - Adjust TEMP_OFFSET and HUMIDITY_OFFSET constants if needed
 * - Consider altitude correction for humidity readings
 * 
 * TROUBLESHOOTING:
 * ================
 * 1. Check all connections match circuit diagram
 * 2. Verify 10kΩ pull-up resistor is installed
 * 3. Ensure stable 3.3V power supply
 * 4. Check for loose connections on breadboard
 * 5. Verify DHT22 orientation (notch indicates pin 1)
 * 6. Allow 2+ seconds between readings
 * 7. Check serial monitor baud rate (115200)
 * 
 * Created: March 28, 2019 by Peter Dalmaris
 * Updated: July 27, 2025 for ESP32-S3 compatibility and educational enhancement
 */

#include "DHTesp.h"

// ================================================================================================
// CONFIGURATION CONSTANTS
// ================================================================================================

// Hardware Configuration
const uint8_t DHT_PIN = 2;                    // GPIO2 for DHT22 data line
const uint32_t SERIAL_BAUD_RATE = 115200;     // Serial communication speed
const uint32_t READING_INTERVAL_MS = 2500;    // Minimum interval between readings (ms)

// Calibration Constants (adjust based on reference measurements)
const float TEMP_OFFSET = 0.0;                // Temperature calibration offset (°C)
const float HUMIDITY_OFFSET = 0.0;            // Humidity calibration offset (%RH)

// Data Validation Thresholds
const float MIN_TEMP = -40.0;                 // Minimum valid temperature (°C)
const float MAX_TEMP = 80.0;                  // Maximum valid temperature (°C)
const float MIN_HUMIDITY = 0.0;               // Minimum valid humidity (%RH)
const float MAX_HUMIDITY = 100.0;             // Maximum valid humidity (%RH)

// Statistical Analysis Configuration
const uint8_t SAMPLE_BUFFER_SIZE = 10;        // Number of samples for moving average
const float OUTLIER_THRESHOLD = 3.0;          // Standard deviations for outlier detection

// Output Format Options
const bool ENABLE_JSON_OUTPUT = true;         // Enable JSON formatted output
const bool ENABLE_CSV_OUTPUT = false;         // Enable CSV formatted output
const bool ENABLE_STATISTICS = true;          // Enable statistical analysis

// ================================================================================================
// GLOBAL VARIABLES
// ================================================================================================

DHTesp dhtSensor;                              // DHT sensor object
ComfortState comfortStatus;                   // Comfort state analysis

// Statistical Analysis Arrays
float temperatureBuffer[SAMPLE_BUFFER_SIZE];   // Temperature sample buffer
float humidityBuffer[SAMPLE_BUFFER_SIZE];      // Humidity sample buffer
uint8_t bufferIndex = 0;                      // Current buffer position
uint8_t validSamples = 0;                     // Number of valid samples collected

// Performance Monitoring
uint32_t totalReadings = 0;                   // Total attempted readings
uint32_t successfulReadings = 0;              // Successful readings
uint32_t lastReadingTime = 0;                 // Timestamp of last reading

// Status Indicators
bool sensorInitialized = false;               // Sensor initialization status
uint8_t consecutiveErrors = 0;                // Count of consecutive read errors

// ================================================================================================
// SETUP FUNCTION
// ================================================================================================

// ================================================================================================
// STRING UTILITY FUNCTIONS
// ================================================================================================

String repeatChar(char character, int count) {
    String result = "";
    for (int i = 0; i < count; i++) {
        result += character;
    }
    return result;
}

void setup() {
    // Initialize serial communication
    Serial.begin(SERIAL_BAUD_RATE);
    while (!Serial && millis() < 5000) {
        delay(10); // Wait for serial connection up to 5 seconds
    }
    
    // Display startup information
    printStartupBanner();
    
    // Initialize DHT sensor
    Serial.println("[INIT] Initializing DHT22 sensor...");
    dhtSensor.setup(DHT_PIN, DHTesp::DHT22);
    
    // Verify sensor initialization
    delay(2000); // Allow sensor to stabilize
    if (performInitialSensorTest()) {
        sensorInitialized = true;
        Serial.println("[SUCCESS] DHT22 sensor initialized successfully");
        printSensorSpecifications();
    } else {
        Serial.println("[ERROR] DHT22 sensor initialization failed!");
        Serial.println("[INFO] Check connections and restart the device");
    }
    
    // Initialize data buffers
    initializeDataBuffers();
    
    Serial.println("\n" + repeatChar('=', 80));
    Serial.println("STARTING CONTINUOUS MONITORING");
    Serial.println(repeatChar('=', 80));
    
    if (ENABLE_CSV_OUTPUT) {
        printCSVHeader();
    }
}

// ================================================================================================
// MAIN LOOP FUNCTION
// ================================================================================================

void loop() {
    // Check if enough time has passed since last reading
    if (millis() - lastReadingTime < READING_INTERVAL_MS) {
        return;
    }
    
    // Perform sensor reading
    performSensorReading();
    
    // Update timing
    lastReadingTime = millis();
    
    // Small delay to prevent tight looping
    delay(100);
}

// ================================================================================================
// SENSOR READING FUNCTIONS
// ================================================================================================

void performSensorReading() {
    Serial.println("\n" + repeatChar('-', 50));
    Serial.println("DHT22 SENSOR READING #" + String(totalReadings + 1));
    Serial.println(repeatChar('-', 50));
    
    totalReadings++;
    
    // Read sensor data
    TempAndHumidity data = dhtSensor.getTempAndHumidity();
    
    // Check for reading errors
    if (dhtSensor.getStatus() != 0) {
        handleReadingError();
        return;
    }
    
    // Apply calibration offsets
    float calibratedTemp = data.temperature + TEMP_OFFSET;
    float calibratedHumidity = data.humidity + HUMIDITY_OFFSET;
    
    // Validate sensor data
    if (!validateSensorData(calibratedTemp, calibratedHumidity)) {
        Serial.println("[WARNING] Invalid sensor data detected - skipping reading");
        consecutiveErrors++;
        return;
    }
    
    // Reset error counter on successful reading
    consecutiveErrors = 0;
    successfulReadings++;
    
    // Store data for statistical analysis
    if (ENABLE_STATISTICS) {
        storeSampleData(calibratedTemp, calibratedHumidity);
    }
    
    // Display readings in various formats
    displaySensorData(calibratedTemp, calibratedHumidity);
    
    // Calculate and display comfort index
    calculateComfortIndex(calibratedTemp, calibratedHumidity);
    
    // Display statistics if enabled
    if (ENABLE_STATISTICS && validSamples >= 3) {
        displayStatisticalAnalysis();
    }
    
    // Display performance metrics
    displayPerformanceMetrics();
}

bool validateSensorData(float temperature, float humidity) {
    // Check temperature range
    if (temperature < MIN_TEMP || temperature > MAX_TEMP) {
        Serial.println("[ERROR] Temperature out of valid range: " + String(temperature) + "°C");
        return false;
    }
    
    // Check humidity range
    if (humidity < MIN_HUMIDITY || humidity > MAX_HUMIDITY) {
        Serial.println("[ERROR] Humidity out of valid range: " + String(humidity) + "%RH");
        return false;
    }
    
    // Check for NaN values
    if (isnan(temperature) || isnan(humidity)) {
        Serial.println("[ERROR] NaN values detected in sensor data");
        return false;
    }
    
    return true;
}

void handleReadingError() {
    consecutiveErrors++;
    
    Serial.println("[ERROR] Failed to read from DHT22 sensor");
    Serial.println("[INFO] Error code: " + String(dhtSensor.getStatus()));
    Serial.println("[INFO] Error description: " + String(dhtSensor.getStatusString()));
    Serial.println("[INFO] Consecutive errors: " + String(consecutiveErrors));
    
    // Provide troubleshooting advice based on error count
    if (consecutiveErrors == 1) {
        Serial.println("[ADVICE] This might be a temporary glitch - will retry");
    } else if (consecutiveErrors == 3) {
        Serial.println("[ADVICE] Check sensor connections and pull-up resistor");
    } else if (consecutiveErrors == 5) {
        Serial.println("[ADVICE] Consider restarting the device");
        Serial.println("[ADVICE] Verify sensor power supply stability");
    } else if (consecutiveErrors >= 10) {
        Serial.println("[CRITICAL] Multiple consecutive failures detected!");
        Serial.println("[CRITICAL] Hardware issue likely - check all connections");
    }
}

// ================================================================================================
// DATA STORAGE AND STATISTICAL ANALYSIS
// ================================================================================================

void storeSampleData(float temperature, float humidity) {
    // Store data in circular buffer
    temperatureBuffer[bufferIndex] = temperature;
    humidityBuffer[bufferIndex] = humidity;
    
    // Update buffer index
    bufferIndex = (bufferIndex + 1) % SAMPLE_BUFFER_SIZE;
    
    // Update valid sample count
    if (validSamples < SAMPLE_BUFFER_SIZE) {
        validSamples++;
    }
}

void displayStatisticalAnalysis() {
    // Calculate averages
    float avgTemp = calculateAverage(temperatureBuffer, validSamples);
    float avgHumidity = calculateAverage(humidityBuffer, validSamples);
    
    // Calculate standard deviations
    float stdDevTemp = calculateStandardDeviation(temperatureBuffer, validSamples, avgTemp);
    float stdDevHumidity = calculateStandardDeviation(humidityBuffer, validSamples, avgHumidity);
    
    // Find min/max values
    float minTemp, maxTemp, minHumidity, maxHumidity;
    findMinMax(temperatureBuffer, validSamples, &minTemp, &maxTemp);
    findMinMax(humidityBuffer, validSamples, &minHumidity, &maxHumidity);
    
    Serial.println("\n📊 STATISTICAL ANALYSIS (Last " + String(validSamples) + " readings):");
    Serial.println("Temperature - Avg: " + String(avgTemp, 2) + "°C, StdDev: " + String(stdDevTemp, 2) + 
                   "°C, Range: " + String(minTemp, 1) + "-" + String(maxTemp, 1) + "°C");
    Serial.println("Humidity    - Avg: " + String(avgHumidity, 2) + "%RH, StdDev: " + String(stdDevHumidity, 2) + 
                   "%RH, Range: " + String(minHumidity, 1) + "-" + String(maxHumidity, 1) + "%RH");
}

// ================================================================================================
// DISPLAY AND OUTPUT FUNCTIONS
// ================================================================================================

void displaySensorData(float temperature, float humidity) {
    // Standard readable format
    Serial.println("🌡️  Temperature: " + String(temperature, 2) + " °C (" + String(celsiusToFahrenheit(temperature), 1) + " °F)");
    Serial.println("💧 Humidity: " + String(humidity, 2) + " %RH");
    
    // Additional calculated values
    float dewPoint = calculateDewPoint(temperature, humidity);
    float heatIndex = calculateHeatIndex(temperature, humidity);
    
    Serial.println("🌫️  Dew Point: " + String(dewPoint, 2) + " °C");
    Serial.println("🔥 Heat Index: " + String(heatIndex, 2) + " °C");
    
    // JSON format output
    if (ENABLE_JSON_OUTPUT) {
        displayJSONOutput(temperature, humidity, dewPoint, heatIndex);
    }
    
    // CSV format output
    if (ENABLE_CSV_OUTPUT) {
        displayCSVOutput(temperature, humidity, dewPoint, heatIndex);
    }
}

void displayJSONOutput(float temp, float humidity, float dewPoint, float heatIndex) {
    Serial.println("\n📋 JSON Output:");
    Serial.println("{");
    Serial.println("  \"timestamp\": " + String(millis()) + ",");
    Serial.println("  \"temperature_celsius\": " + String(temp, 2) + ",");
    Serial.println("  \"temperature_fahrenheit\": " + String(celsiusToFahrenheit(temp), 2) + ",");
    Serial.println("  \"humidity_percent\": " + String(humidity, 2) + ",");
    Serial.println("  \"dew_point_celsius\": " + String(dewPoint, 2) + ",");
    Serial.println("  \"heat_index_celsius\": " + String(heatIndex, 2) + ",");
    Serial.println("  \"reading_number\": " + String(successfulReadings) + ",");
    Serial.println("  \"sensor_status\": \"OK\"");
    Serial.println("}");
}

void calculateComfortIndex(float temperature, float humidity) {
    // Use the correct function signature for getComfortRatio
    float comfortRatio = dhtSensor.getComfortRatio(comfortStatus, temperature, humidity, false);
    
    Serial.println("\n🏠 COMFORT ANALYSIS:");
    
    String comfortText;
    switch (comfortStatus) {
        case Comfort_OK:
            comfortText = "✅ Comfortable";
            break;
        case Comfort_TooHot:
            comfortText = "🔥 Too Hot";
            break;
        case Comfort_TooCold:
            comfortText = "❄️ Too Cold";
            break;
        case Comfort_TooDry:
            comfortText = "🏜️ Too Dry";
            break;
        case Comfort_TooHumid:
            comfortText = "💦 Too Humid";
            break;
        case Comfort_HotAndHumid:
            comfortText = "🌡️💦 Hot and Humid";
            break;
        case Comfort_HotAndDry:
            comfortText = "🌡️🏜️ Hot and Dry";
            break;
        case Comfort_ColdAndHumid:
            comfortText = "❄️💦 Cold and Humid";
            break;
        case Comfort_ColdAndDry:
            comfortText = "❄️🏜️ Cold and Dry";
            break;
        default:
            comfortText = "❓ Unknown";
            break;
    }
    
    Serial.println("Status: " + comfortText);
    Serial.println("Comfort Ratio: " + String(comfortRatio, 2));
    
    // Provide comfort recommendations
    provideComfortRecommendations(comfortStatus);
}

// ================================================================================================
// UTILITY AND CALCULATION FUNCTIONS
// ================================================================================================

float celsiusToFahrenheit(float celsius) {
    return (celsius * 9.0 / 5.0) + 32.0;
}

float calculateDewPoint(float temperature, float humidity) {
    float a = 17.27;
    float b = 237.7;
    float alpha = ((a * temperature) / (b + temperature)) + log(humidity / 100.0);
    return (b * alpha) / (a - alpha);
}

float calculateHeatIndex(float temperature, float humidity) {
    // Simplified heat index calculation for Celsius
    if (temperature < 27.0) {
        return temperature; // Heat index not applicable below 27°C
    }
    
    float T = celsiusToFahrenheit(temperature);
    float R = humidity;
    
    float HI = -42.379 + 2.04901523 * T + 10.14333127 * R - 0.22475541 * T * R 
               - 6.83783e-3 * T * T - 5.481717e-2 * R * R + 1.22874e-3 * T * T * R 
               + 8.5282e-4 * T * R * R - 1.99e-6 * T * T * R * R;
    
    return (HI - 32.0) * 5.0 / 9.0; // Convert back to Celsius
}

float calculateAverage(float* buffer, uint8_t count) {
    float sum = 0.0;
    for (uint8_t i = 0; i < count; i++) {
        sum += buffer[i];
    }
    return sum / count;
}

float calculateStandardDeviation(float* buffer, uint8_t count, float average) {
    if (count <= 1) return 0.0;
    
    float sumSquaredDiff = 0.0;
    for (uint8_t i = 0; i < count; i++) {
        float diff = buffer[i] - average;
        sumSquaredDiff += diff * diff;
    }
    return sqrt(sumSquaredDiff / (count - 1));
}

void findMinMax(float* buffer, uint8_t count, float* minVal, float* maxVal) {
    if (count == 0) {
        *minVal = *maxVal = 0.0;
        return;
    }
    
    *minVal = *maxVal = buffer[0];
    for (uint8_t i = 1; i < count; i++) {
        if (buffer[i] < *minVal) *minVal = buffer[i];
        if (buffer[i] > *maxVal) *maxVal = buffer[i];
    }
}

// ================================================================================================
// INITIALIZATION AND UTILITY FUNCTIONS
// ================================================================================================

void printStartupBanner() {
    Serial.println("\n" + repeatChar('=', 80));
    Serial.println("          DHT22 ENVIRONMENTAL SENSOR - COMPREHENSIVE VERSION");
    Serial.println("                      ESP32-S3-DevKitC-1 v1.1");
    Serial.println(repeatChar('=', 80));
    Serial.println("Course: IoT Development with ESP32-S3");
    Serial.println("Lesson: 05.061 - Temperature and Humidity Monitoring");
    Serial.println("Target: Professional IoT sensor implementation");
    Serial.println("");
    Serial.println("Hardware: ESP32-S3-WROOM-1-N16R16V");
    Serial.println("Sensor: DHT22 (AM2302) Digital Temperature & Humidity");
    Serial.println("GPIO Pin: " + String(DHT_PIN) + " (with 10kΩ pull-up resistor)");
    Serial.println("Baud Rate: " + String(SERIAL_BAUD_RATE));
    Serial.println(repeatChar('=', 80));
}

void printSensorSpecifications() {
    Serial.println("\n📋 DHT22 SENSOR SPECIFICATIONS:");
    Serial.println("• Temperature Range: -40°C to +80°C (±0.5°C accuracy)");
    Serial.println("• Humidity Range: 0-100% RH (±2-5% accuracy)");
    Serial.println("• Resolution: Temperature 0.1°C, Humidity 0.1%RH");
    Serial.println("• Sampling Rate: 0.5Hz (once every 2 seconds)");
    Serial.println("• Operating Voltage: 3.3V-5.5V DC");
    Serial.println("• Current: 1-1.5mA measuring, 40-50μA standby");
}

bool performInitialSensorTest() {
    Serial.println("[TEST] Performing initial sensor validation...");
    
    for (int attempt = 1; attempt <= 3; attempt++) {
        Serial.println("[TEST] Attempt " + String(attempt) + "/3");
        
        TempAndHumidity testData = dhtSensor.getTempAndHumidity();
        
        if (dhtSensor.getStatus() == 0) {
            Serial.println("[TEST] ✅ Sensor responding correctly");
            Serial.println("[TEST] Initial readings - Temp: " + String(testData.temperature, 1) + 
                           "°C, Humidity: " + String(testData.humidity, 1) + "%RH");
            return true;
        }
        
        Serial.println("[TEST] ❌ Attempt failed, status: " + String(dhtSensor.getStatus()));
        if (attempt < 3) delay(2000);
    }
    
    return false;
}

void initializeDataBuffers() {
    Serial.println("[INIT] Initializing statistical data buffers...");
    
    for (uint8_t i = 0; i < SAMPLE_BUFFER_SIZE; i++) {
        temperatureBuffer[i] = 0.0;
        humidityBuffer[i] = 0.0;
    }
    
    bufferIndex = 0;
    validSamples = 0;
    
    Serial.println("[INIT] ✅ Data buffers initialized");
}

void displayPerformanceMetrics() {
    float successRate = (totalReadings > 0) ? (float)successfulReadings / totalReadings * 100.0 : 0.0;
    
    Serial.println("\n📈 PERFORMANCE METRICS:");
    Serial.println("• Total Readings: " + String(totalReadings));
    Serial.println("• Successful: " + String(successfulReadings));
    Serial.println("• Success Rate: " + String(successRate, 1) + "%");
    Serial.println("• Consecutive Errors: " + String(consecutiveErrors));
    Serial.println("• Uptime: " + formatUptime(millis()));
    Serial.println("• Free Heap: " + String(ESP.getFreeHeap()) + " bytes");
}

void provideComfortRecommendations(ComfortState comfort) {
    Serial.println("💡 Recommendations:");
    
    switch (comfort) {
        case Comfort_TooHot:
            Serial.println("   - Increase ventilation or air conditioning");
            Serial.println("   - Close curtains/blinds to block sunlight");
            break;
        case Comfort_TooCold:
            Serial.println("   - Increase heating or insulation");
            Serial.println("   - Close windows and doors");
            break;
        case Comfort_TooDry:
            Serial.println("   - Use humidifier or place water containers");
            Serial.println("   - Add indoor plants");
            break;
        case Comfort_TooHumid:
            Serial.println("   - Increase ventilation or use dehumidifier");
            Serial.println("   - Check for moisture sources");
            break;
        case Comfort_HotAndHumid:
            Serial.println("   - Prioritize cooling and dehumidification");
            Serial.println("   - Ensure adequate air circulation");
            break;
        case Comfort_ColdAndDry:
            Serial.println("   - Add heating and humidification");
            Serial.println("   - Check insulation and draft sources");
            break;
        default:
            Serial.println("   - Current conditions are comfortable!");
            break;
    }
}

void printCSVHeader() {
    Serial.println("\n📊 CSV DATA FORMAT:");
    Serial.println("Timestamp,Temperature_C,Temperature_F,Humidity_RH,DewPoint_C,HeatIndex_C,Status");
}

void displayCSVOutput(float temp, float humidity, float dewPoint, float heatIndex) {
    Serial.print(millis());
    Serial.print(",");
    Serial.print(temp, 2);
    Serial.print(",");
    Serial.print(celsiusToFahrenheit(temp), 2);
    Serial.print(",");
    Serial.print(humidity, 2);
    Serial.print(",");
    Serial.print(dewPoint, 2);
    Serial.print(",");
    Serial.print(heatIndex, 2);
    Serial.print(",");
    Serial.println("OK");
}

String formatUptime(uint32_t milliseconds) {
    uint32_t seconds = milliseconds / 1000;
    uint32_t minutes = seconds / 60;
    uint32_t hours = minutes / 60;
    
    seconds %= 60;
    minutes %= 60;
    
    return String(hours) + "h " + String(minutes) + "m " + String(seconds) + "s";
}
