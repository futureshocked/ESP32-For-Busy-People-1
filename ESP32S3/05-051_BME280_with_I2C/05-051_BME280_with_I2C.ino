/*  05.051 - BME280 Environmental Sensor with I2C for ESP32-S3
 * 
 * Course: IoT Development with ESP32-S3
 * Target Hardware: ESP32-S3-DevKitC-1 v1.1
 * Module: ESP32-S3-WROOM-1-N16R16V
 * 
 * This comprehensive sketch demonstrates how to read temperature, humidity, 
 * barometric pressure, and calculated altitude from the BME280 environmental 
 * sensor using I2C communication. Features include error handling, non-blocking 
 * timing, sensor health monitoring, and data validation.
 * 
 * LEARNING OBJECTIVES:
 * ====================
 * - Understand I2C communication protocol implementation
 * - Learn environmental sensor data acquisition and processing
 * - Implement robust error handling for sensor communication
 * - Practice non-blocking programming techniques
 * - Apply data validation and filtering methods
 * - Understand altitude calculation principles
 * 
 * HARDWARE INFORMATION:
 * =====================
 * ESP32-S3-DevKitC-1 v1.1:
 * - Dual-core Xtensa LX7 CPU up to 240 MHz
 * - 16 MB Flash Memory, 16 MB PSRAM
 * - Native USB support via USB-C connector
 * - 45 GPIO pins with multiple peripheral functions
 * - Hardware I2C controllers (Wire library support)
 * - 12-bit ADC resolution (0-4095)
 * 
 * BME280 Environmental Sensor:
 * - Temperature range: -40°C to +85°C (±1.0°C accuracy)
 * - Humidity range: 0-100% RH (±3% accuracy)
 * - Pressure range: 300-1100 hPa (±1 hPa accuracy)
 * - I2C addresses: 0x76 or 0x77 (configurable via SDO pin)
 * - Operating voltage: 1.8V - 3.6V
 * - Low power consumption: 3.4μA @ 1Hz sampling
 * 
 * CIRCUIT DIAGRAM:
 * ================
 *                   ESP32-S3-DevKitC-1                    BME280 Breakout
 *                   ==================                    ================
 *                          3.3V  ●─────────────────────────● VIN/VCC
 *                           GND  ●─────────────────────────● GND
 *                    GPIO21/SDA  ●─────────────────────────● SDA
 *                    GPIO22/SCL  ●─────────────────────────● SCL
 *                                                          ● CSB (leave unconnected for I2C)
 *                                                          ● SDO (GND=0x76, VCC=0x77)
 * 
 * COMPONENTS:
 * ===========
 * 1x ESP32-S3-DevKitC-1 v1.1 development board
 * 1x BME280 environmental sensor breakout board
 * 4x Male-to-female jumper wires
 * 1x USB-C cable for programming and power
 * 1x Breadboard (optional, for organized connections)
 * 
 * GPIO ASSIGNMENTS:
 * =================
 * GPIO21 (SDA) - I2C Serial Data Line to BME280
 * GPIO22 (SCL) - I2C Serial Clock Line to BME280
 * GPIO38 - Onboard RGB LED (WS2812) for status indication
 * 
 * TECHNICAL NOTES:
 * ================
 * - BME280 uses I2C standard mode (100kHz) or fast mode (400kHz)
 * - Default I2C address is 0x76; use 0x77 if SDO pin is pulled high
 * - Sea level pressure constant may need adjustment for your location
 * - Altitude calculation uses barometric formula with standard atmosphere model
 * - Sensor readings are filtered using simple moving average for stability
 * - Non-blocking timing prevents system freezing during sensor operations
 * 
 * SAFETY CONSIDERATIONS:
 * ======================
 * - Verify 3.3V power supply to prevent sensor damage
 * - Ensure proper I2C pull-up resistors (usually integrated on breakout boards)
 * - Check connections before powering to prevent short circuits
 * - Handle sensor breakout board carefully to avoid damage to small components
 * 
 * IDE REQUIREMENTS:
 * =================
 * - Arduino IDE 2.0+
 * - ESP32 Arduino Core v3.0+
 * - Board: "ESP32S3 Dev Module"
 * - Libraries: Adafruit_Sensor, Adafruit_BME280
 * 
 * Created: March 28, 2019 by Peter Dalmaris
 * Updated: January 26, 2025 for ESP32-S3 compatibility and modern practices
 */

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

// ================================================================================
// CONFIGURATION CONSTANTS
// ================================================================================

// Sensor Configuration
const uint8_t BME280_I2C_ADDRESS = 0x76;           // Primary I2C address (0x77 if SDO high)
const float SEALEVEL_PRESSURE_HPA = 1013.25f;      // Standard sea level pressure
const uint16_t SENSOR_READ_INTERVAL_MS = 2000;     // Reading interval in milliseconds
const uint8_t MOVING_AVERAGE_SAMPLES = 5;          // Number of samples for averaging

// I2C Configuration
const uint8_t I2C_SDA_PIN = 21;                     // I2C Serial Data Line
const uint8_t I2C_SCL_PIN = 22;                     // I2C Serial Clock Line
const uint32_t I2C_FREQUENCY = 400000;              // I2C frequency (400kHz fast mode)

// Serial Communication
const uint32_t SERIAL_BAUD_RATE = 115200;          // Serial communication speed
const uint16_t SERIAL_TIMEOUT_MS = 1000;           // Serial timeout

// Status LED Configuration
const uint8_t STATUS_LED_PIN = 38;                  // Onboard RGB LED (WS2812)

// Data Validation Thresholds
const float TEMP_MIN_VALID = -40.0f;                // Minimum valid temperature (°C)
const float TEMP_MAX_VALID = 85.0f;                 // Maximum valid temperature (°C)
const float HUMIDITY_MIN_VALID = 0.0f;              // Minimum valid humidity (%)
const float HUMIDITY_MAX_VALID = 100.0f;            // Maximum valid humidity (%)
const float PRESSURE_MIN_VALID = 300.0f;            // Minimum valid pressure (hPa)
const float PRESSURE_MAX_VALID = 1100.0f;           // Maximum valid pressure (hPa)

// ================================================================================
// GLOBAL VARIABLES
// ================================================================================

// Sensor object
Adafruit_BME280 bme;

// Timing variables
unsigned long lastSensorRead = 0;
unsigned long systemStartTime = 0;

// Sensor data structure
struct SensorData {
    float temperature;      // Temperature in Celsius
    float humidity;         // Relative humidity in %
    float pressure;         // Pressure in hPa
    float altitude;         // Calculated altitude in meters
    bool valid;             // Data validity flag
    unsigned long timestamp; // Reading timestamp
};

// Moving average arrays
float tempReadings[MOVING_AVERAGE_SAMPLES];
float humidityReadings[MOVING_AVERAGE_SAMPLES];
float pressureReadings[MOVING_AVERAGE_SAMPLES];
uint8_t readingIndex = 0;
bool averageArrayFilled = false;

// System status variables
bool sensorInitialized = false;
uint32_t totalReadings = 0;
uint32_t validReadings = 0;
uint32_t errorCount = 0;

// ================================================================================
// SETUP FUNCTION
// ================================================================================

void setup() {
    // Record system start time
    systemStartTime = millis();
    
    // Initialize serial communication
    Serial.begin(SERIAL_BAUD_RATE);
    Serial.setTimeout(SERIAL_TIMEOUT_MS);
    
    // Wait for serial connection (useful for debugging)
    unsigned long serialStart = millis();
    while (!Serial && (millis() - serialStart < 3000)) {
        delay(10);
    }
    
    // Print startup banner
    printStartupBanner();
    
    // Initialize I2C with custom pins
    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
    Wire.setClock(I2C_FREQUENCY);
    Serial.printf("I2C initialized: SDA=%d, SCL=%d, Frequency=%d Hz\n", 
                  I2C_SDA_PIN, I2C_SCL_PIN, I2C_FREQUENCY);
    
    // Initialize BME280 sensor
    initializeBME280();
    
    // Initialize moving average arrays
    initializeAverageArrays();
    
    // Print system information
    printSystemInfo();
    
    Serial.println("Setup completed successfully!");
    Serial.println("Starting sensor readings...\n");
}

// ================================================================================
// MAIN LOOP FUNCTION
// ================================================================================

void loop() {
    // Non-blocking sensor reading
    if (millis() - lastSensorRead >= SENSOR_READ_INTERVAL_MS) {
        lastSensorRead = millis();
        
        // Read and process sensor data
        SensorData currentReading = readSensorData();
        
        if (currentReading.valid) {
            // Apply moving average filtering
            SensorData filteredReading = applyMovingAverage(currentReading);
            
            // Display sensor readings
            displaySensorData(filteredReading);
            
            // Update statistics
            validReadings++;
        } else {
            Serial.println("❌ Invalid sensor reading detected!");
            errorCount++;
        }
        
        totalReadings++;
        
        // Print statistics every 10 readings
        if (totalReadings % 10 == 0) {
            printStatistics();
        }
    }
    
    // Handle other non-blocking tasks here
    delay(10); // Small delay to prevent watchdog issues
}

// ================================================================================
// SENSOR INITIALIZATION FUNCTIONS
// ================================================================================

void initializeBME280() {
    Serial.print("Initializing BME280 sensor...");
    
    // Try primary address first
    if (bme.begin(BME280_I2C_ADDRESS)) {
        sensorInitialized = true;
        Serial.printf(" ✓ Found at address 0x%02X\n", BME280_I2C_ADDRESS);
    }
    // Try alternate address if primary fails
    else if (bme.begin(0x77)) {
        sensorInitialized = true;
        Serial.printf(" ✓ Found at address 0x77\n");
    }
    else {
        Serial.println(" ❌ FAILED!");
        Serial.println("BME280 sensor not found. Please check:");
        Serial.println("1. Wiring connections (SDA, SCL, VCC, GND)");
        Serial.println("2. I2C address (0x76 or 0x77)");
        Serial.println("3. Power supply (3.3V)");
        Serial.println("4. Pull-up resistors on I2C lines");
        
        // Attempt I2C scan for debugging
        scanI2CDevices();
        
        while (1) {
            delay(1000);
            Serial.println("System halted. Fix sensor connection and restart.");
        }
    }
    
    // Configure sensor settings for optimal performance
    configureSensorSettings();
}

void configureSensorSettings() {
    // Set sensor to normal mode with optimal settings
    // Temperature: x2 oversampling
    // Pressure: x16 oversampling  
    // Humidity: x1 oversampling
    // Filter: coefficient 16
    // Standby time: 62.5ms
    
    bme.setSampling(Adafruit_BME280::MODE_NORMAL,     // Operating mode
                    Adafruit_BME280::SAMPLING_X2,     // Temperature oversampling
                    Adafruit_BME280::SAMPLING_X16,    // Pressure oversampling
                    Adafruit_BME280::SAMPLING_X1,     // Humidity oversampling
                    Adafruit_BME280::FILTER_X16,      // IIR filter coefficient
                    Adafruit_BME280::STANDBY_MS_62_5);  // Standby time
    
    Serial.println("Sensor configured with optimal settings");
}

// ================================================================================
// SENSOR READING FUNCTIONS
// ================================================================================

SensorData readSensorData() {
    SensorData data;
    data.timestamp = millis();
    data.valid = false;
    
    if (!sensorInitialized) {
        Serial.println("❌ Sensor not initialized!");
        return data;
    }
    
    // Read raw sensor values
    data.temperature = bme.readTemperature();
    data.humidity = bme.readHumidity();
    data.pressure = bme.readPressure() / 100.0f; // Convert Pa to hPa
    data.altitude = bme.readAltitude(SEALEVEL_PRESSURE_HPA);
    
    // Validate sensor readings
    data.valid = validateSensorData(data);
    
    return data;
}

bool validateSensorData(const SensorData& data) {
    // Check for NaN values
    if (isnan(data.temperature) || isnan(data.humidity) || 
        isnan(data.pressure) || isnan(data.altitude)) {
        Serial.println("❌ NaN value detected in sensor data");
        return false;
    }
    
    // Check temperature range
    if (data.temperature < TEMP_MIN_VALID || data.temperature > TEMP_MAX_VALID) {
        Serial.printf("❌ Temperature out of range: %.2f°C\n", data.temperature);
        return false;
    }
    
    // Check humidity range
    if (data.humidity < HUMIDITY_MIN_VALID || data.humidity > HUMIDITY_MAX_VALID) {
        Serial.printf("❌ Humidity out of range: %.2f%%\n", data.humidity);
        return false;
    }
    
    // Check pressure range
    if (data.pressure < PRESSURE_MIN_VALID || data.pressure > PRESSURE_MAX_VALID) {
        Serial.printf("❌ Pressure out of range: %.2f hPa\n", data.pressure);
        return false;
    }
    
    return true;
}

// ================================================================================
// MOVING AVERAGE FILTERING FUNCTIONS
// ================================================================================

void initializeAverageArrays() {
    for (uint8_t i = 0; i < MOVING_AVERAGE_SAMPLES; i++) {
        tempReadings[i] = 0.0f;
        humidityReadings[i] = 0.0f;
        pressureReadings[i] = 0.0f;
    }
    readingIndex = 0;
    averageArrayFilled = false;
}

SensorData applyMovingAverage(const SensorData& newReading) {
    // Store new readings in circular buffer
    tempReadings[readingIndex] = newReading.temperature;
    humidityReadings[readingIndex] = newReading.humidity;
    pressureReadings[readingIndex] = newReading.pressure;
    
    // Advance index
    readingIndex = (readingIndex + 1) % MOVING_AVERAGE_SAMPLES;
    
    // Check if we've filled the array at least once
    if (!averageArrayFilled && readingIndex == 0) {
        averageArrayFilled = true;
    }
    
    // Calculate averages
    SensorData filteredData;
    filteredData.temperature = calculateAverage(tempReadings);
    filteredData.humidity = calculateAverage(humidityReadings);
    filteredData.pressure = calculateAverage(pressureReadings);
    filteredData.altitude = bme.readAltitude(SEALEVEL_PRESSURE_HPA); // Use current pressure for altitude
    filteredData.valid = true;
    filteredData.timestamp = newReading.timestamp;
    
    return filteredData;
}

float calculateAverage(const float* readings) {
    float sum = 0.0f;
    uint8_t count = averageArrayFilled ? MOVING_AVERAGE_SAMPLES : readingIndex;
    
    for (uint8_t i = 0; i < count; i++) {
        sum += readings[i];
    }
    
    return (count > 0) ? (sum / count) : 0.0f;
}

// ================================================================================
// DISPLAY FUNCTIONS
// ================================================================================

void displaySensorData(const SensorData& data) {
    Serial.println("================================================================================");
    Serial.printf("📊 Sensor Reading #%d (Runtime: %lu ms)\n", totalReadings + 1, millis() - systemStartTime);
    Serial.println("================================================================================");
    
    // Temperature display
    Serial.printf("🌡️  Temperature: %.2f°C (%.2f°F)\n", 
                  data.temperature, (data.temperature * 9.0f / 5.0f) + 32.0f);
    
    // Humidity display
    Serial.printf("💧 Humidity: %.2f%% RH\n", data.humidity);
    
    // Pressure display
    Serial.printf("📊 Pressure: %.2f hPa (%.2f inHg)\n", 
                  data.pressure, data.pressure * 0.02953f);
    
    // Altitude display
    Serial.printf("🏔️  Altitude: %.2f m (%.2f ft)\n", 
                  data.altitude, data.altitude * 3.28084f);
    
    // Comfort level assessment
    assessComfortLevel(data);
    
    Serial.printf("⏰ Reading time: %lu ms\n", data.timestamp);
    Serial.println();
}

void assessComfortLevel(const SensorData& data) {
    Serial.print("🏠 Comfort Level: ");
    
    // Simple comfort assessment based on temperature and humidity
    if (data.temperature >= 20.0f && data.temperature <= 26.0f && 
        data.humidity >= 40.0f && data.humidity <= 60.0f) {
        Serial.println("Comfortable ✓");
    } else if (data.temperature < 18.0f) {
        Serial.println("Too Cold ❄️");
    } else if (data.temperature > 28.0f) {
        Serial.println("Too Hot 🔥");
    } else if (data.humidity < 30.0f) {
        Serial.println("Too Dry 🏜️");
    } else if (data.humidity > 70.0f) {
        Serial.println("Too Humid 💦");
    } else {
        Serial.println("Moderate 🤔");
    }
}

// ================================================================================
// UTILITY FUNCTIONS
// ================================================================================

void printStartupBanner() {
    Serial.println("\n================================================================================");
    Serial.println("🌡️ BME280 Environmental Sensor - ESP32-S3 Edition");
    Serial.println("================================================================================");
    Serial.println("Course: IoT Development with ESP32-S3");
    Serial.println("Lesson: 05.051 - Environmental Sensing with I2C");
    Serial.println("Hardware: ESP32-S3-DevKitC-1 v1.1 + BME280 Sensor");
    Serial.println("================================================================================\n");
}

void printSystemInfo() {
    Serial.println("📋 System Information:");
    Serial.printf("   Chip Model: %s\n", ESP.getChipModel());
    Serial.printf("   Chip Revision: %d\n", ESP.getChipRevision());
    Serial.printf("   CPU Frequency: %d MHz\n", ESP.getCpuFreqMHz());
    Serial.printf("   Flash Size: %d MB\n", ESP.getFlashChipSize() / (1024 * 1024));
    Serial.printf("   PSRAM Size: %d MB\n", ESP.getPsramSize() / (1024 * 1024));
    Serial.printf("   Free Heap: %d bytes\n", ESP.getFreeHeap());
    Serial.println();
}

void printStatistics() {
    Serial.println("📈 Statistics:");
    Serial.printf("   Total Readings: %d\n", totalReadings);
    Serial.printf("   Valid Readings: %d (%.1f%%)\n", validReadings, 
                  (float)validReadings / totalReadings * 100.0f);
    Serial.printf("   Error Count: %d\n", errorCount);
    Serial.printf("   Runtime: %.2f minutes\n", (millis() - systemStartTime) / 60000.0f);
    Serial.println();
}

void scanI2CDevices() {
    Serial.println("🔍 Scanning I2C bus for devices...");
    uint8_t deviceCount = 0;
    
    for (uint8_t address = 1; address < 127; address++) {
        Wire.beginTransmission(address);
        if (Wire.endTransmission() == 0) {
            Serial.printf("   Device found at address 0x%02X\n", address);
            deviceCount++;
        }
    }
    
    if (deviceCount == 0) {
        Serial.println("   No I2C devices found!");
    } else {
        Serial.printf("   Found %d device(s)\n", deviceCount);
    }
    Serial.println();
}
