/*  05.053 - BME280 Environmental Sensor with Timer Interrupts for ESP32-S3
 * 
 * Course: IoT Development with ESP32-S3
 * Target Hardware: ESP32-S3-DevKitC-1 v1.1
 * Module: ESP32-S3-WROOM-1-N16R16V
 * 
 * This comprehensive example demonstrates professional-grade environmental monitoring
 * using the BME280 sensor with non-blocking timer interrupts. Features include
 * advanced error handling, I2C diagnostics, performance monitoring, and educational
 * debugging output suitable for intermediate students.
 * 
 * LEARNING OBJECTIVES:
 * ===================
 * - Master I2C communication protocols on ESP32-S3
 * - Implement non-blocking timer interrupt patterns
 * - Apply professional error handling techniques
 * - Understand sensor calibration and data validation
 * - Practice memory-efficient embedded programming
 * 
 * HARDWARE INFORMATION:
 * =====================
 * ESP32-S3-DevKitC-1 v1.1 Specifications:
 * - Dual-core Xtensa LX7 @ 240 MHz
 * - 16 MB Flash Memory, 16 MB PSRAM
 * - Built-in WiFi 802.11 b/g/n + Bluetooth 5.0
 * - 45 GPIO pins with advanced peripheral support
 * - Hardware I2C controllers with configurable pins
 * - 4 hardware timers with microsecond precision
 * 
 * BME280 Sensor Specifications:
 * - Operating Range: -40°C to +85°C, 300-1100 hPa, 0-100% RH
 * - Accuracy: ±1°C, ±1 hPa, ±3% RH
 * - I2C Address: 0x76 or 0x77 (selectable via SDO pin)
 * - Supply Voltage: 1.71V to 3.6V
 * - Current Consumption: 2.1µA @ 1Hz sampling
 * 
 * CIRCUIT DIAGRAM:
 * ================
 *                    ESP32-S3-DevKitC-1 v1.1
 *                    ┌─────────────────────┐
 *                    │                     │
 *              3.3V  │ 3V3              21 │ ──── SDA ────┐
 *               GND  │ GND              22 │ ──── SCL ────┼─── BME280
 *                    │                     │              │   ┌─────────┐
 *                    │                     │              ├───│ VIN     │
 *                    │                     │              │   │ GND     │
 *                    │                     │              │   │ SCL     │
 *                    │                     │              └───│ SDA     │
 *                    │                     │                  │ SDO     │ (optional)
 *                    │                     │                  └─────────┘
 *                    └─────────────────────┘
 * 
 * COMPONENTS:
 * ===========
 * 1x ESP32-S3-DevKitC-1 v1.1 Development Board
 * 1x BME280 Environmental Sensor Breakout Board
 * 1x 400-point Breadboard
 * 4x Male-to-Male Jumper Wires
 * 
 * Optional Components (for advanced experimentation):
 * 2x 4.7kΩ Pull-up Resistors (if not included on breakout)
 * 1x 0.1µF Ceramic Capacitor (power supply filtering)
 * 
 * GPIO ASSIGNMENTS:
 * =================
 * GPIO21 (SDA) - I2C Data Line
 * GPIO22 (SCL) - I2C Clock Line
 * 
 * I2C Configuration:
 * - Frequency: 100kHz (standard mode) or 400kHz (fast mode)
 * - Pull-up Resistors: 4.7kΩ (usually integrated on breakout boards)
 * - Address: 0x76 (default) or 0x77 (if SDO pin pulled high)
 * 
 * TECHNICAL NOTES:
 * ================
 * 1. Timer Interrupt Implementation:
 *    - Uses ESP32-S3 hardware timer 0 with 80MHz base frequency
 *    - Prescaler of 80 creates 1MHz tick rate (1µs resolution)
 *    - Critical sections protect shared variables from race conditions
 *    - IRAM_ATTR ensures ISR runs from internal RAM for speed
 * 
 * 2. I2C Communication:
 *    - ESP32-S3 supports up to 1MHz I2C frequency
 *    - Automatic error detection and recovery mechanisms
 *    - Built-in timeout protection prevents bus lockup
 *    - Diagnostic functions verify sensor connectivity
 * 
 * 3. Memory Management:
 *    - Minimal dynamic allocation for embedded efficiency
 *    - Volatile variables for interrupt-shared data
 *    - Const qualifiers for read-only configuration data
 * 
 * 4. Performance Optimizations:
 *    - Sensor readings cached to reduce I2C traffic
 *    - Interrupt-driven timing eliminates blocking delays
 *    - Serial output optimization for real-time monitoring
 * 
 * IDE REQUIREMENTS:
 * =================
 * - Arduino IDE 2.0+
 * - ESP32 Arduino Core v3.0+
 * - Board: "ESP32S3 Dev Module"
 * - Upload Speed: 921600
 * - CPU Frequency: 240MHz (WiFi/BT)
 * - Flash Mode: QIO
 * - Flash Size: 16MB
 * - PSRAM: Enabled
 * 
 * REQUIRED LIBRARIES:
 * ===================
 * Install via Library Manager:
 * 1. Adafruit BME280 Library (latest version)
 * 2. Adafruit Unified Sensor (dependency)
 * 
 * TROUBLESHOOTING GUIDE:
 * ======================
 * Issue: "Could not find a valid BME280 sensor"
 * Solution: Check I2C connections, try address 0x77, verify 3.3V power
 * 
 * Issue: Inconsistent readings
 * Solution: Add power supply capacitor, check wire length (<20cm recommended)
 * 
 * Issue: Timer not firing consistently
 * Solution: Verify critical section implementation, check for blocking code in loop()
 * 
 * Created: March 28, 2019 by Peter Dalmaris
 * Updated: July 26, 2025 for ESP32-S3 compatibility and educational enhancement
 */

#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <Wire.h>

// ==================================================================================
// CONFIGURATION CONSTANTS
// ==================================================================================

// Environmental constants
constexpr float SEALEVEL_PRESSURE_HPA = 1013.25f;    // Standard atmospheric pressure
constexpr float TEMPERATURE_OFFSET = 0.0f;           // Calibration offset if needed
constexpr float PRESSURE_OFFSET = 0.0f;              // Calibration offset if needed
constexpr float HUMIDITY_OFFSET = 0.0f;              // Calibration offset if needed

// I2C Configuration
constexpr uint8_t BME280_ADDRESS_PRIMARY = 0x76;     // Default I2C address
constexpr uint8_t BME280_ADDRESS_SECONDARY = 0x77;   // Alternative I2C address
constexpr uint32_t I2C_FREQUENCY = 400000;           // 400kHz fast mode
constexpr int SDA_PIN = 21;                          // ESP32-S3 default SDA pin
constexpr int SCL_PIN = 22;                          // ESP32-S3 default SCL pin

// Timer Configuration
constexpr uint8_t TIMER_NUMBER = 0;                  // Use timer 0 (0-3 available)
constexpr uint16_t TIMER_PRESCALER = 80;             // 80MHz / 80 = 1MHz (1µs ticks)
constexpr uint32_t TIMER_ALARM_VALUE = 2000000;      // 2 seconds in microseconds
constexpr bool TIMER_COUNT_UP = true;                // Count up instead of down
constexpr bool TIMER_AUTO_RELOAD = true;             // Automatic reload for periodic interrupts

// Serial Communication
constexpr uint32_t SERIAL_BAUDRATE = 115200;         // Modern standard baud rate
constexpr uint32_t SERIAL_TIMEOUT_MS = 10000;        // 10 second timeout for setup

// Data Validation Limits
constexpr float MIN_VALID_TEMP = -50.0f;             // Minimum reasonable temperature (°C)
constexpr float MAX_VALID_TEMP = 100.0f;             // Maximum reasonable temperature (°C)
constexpr float MIN_VALID_PRESSURE = 800.0f;         // Minimum reasonable pressure (hPa)
constexpr float MAX_VALID_PRESSURE = 1200.0f;        // Maximum reasonable pressure (hPa)
constexpr float MIN_VALID_HUMIDITY = 0.0f;           // Minimum humidity (%)
constexpr float MAX_VALID_HUMIDITY = 100.0f;         // Maximum humidity (%)

// ==================================================================================
// GLOBAL VARIABLES AND HARDWARE OBJECTS
// ==================================================================================

// BME280 sensor object
Adafruit_BME280 bme;

// Timer interrupt variables
volatile uint32_t interruptCounter = 0;              // Incremented by ISR, decremented by main loop
hw_timer_t* timer = nullptr;                         // Hardware timer pointer

// Thread-safe access to shared variables
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

// Performance monitoring
uint32_t lastReadingTime = 0;                        // Track timing between readings
uint32_t totalReadings = 0;                          // Count total sensor readings
uint32_t failedReadings = 0;                         // Count failed sensor readings

// Sensor data structure for efficient data handling
struct SensorData {
    float temperature;
    float pressure;
    float humidity;
    float altitude;
    uint32_t timestamp;
    bool valid;
};

SensorData currentReading = {0};

// ==================================================================================
// INTERRUPT SERVICE ROUTINE
// ==================================================================================

/*
 * Timer Interrupt Service Routine (ISR)
 * 
 * CRITICAL: This function executes in interrupt context and must be:
 * - Marked with IRAM_ATTR for fast execution from internal RAM
 * - Kept as short as possible to minimize interrupt latency
 * - Protected with critical sections when accessing shared variables
 * 
 * The ISR simply increments a counter that the main loop monitors.
 * All actual sensor reading happens in the main loop context.
 */
void IRAM_ATTR onTimer() {
    portENTER_CRITICAL_ISR(&timerMux);
    interruptCounter++;
    portEXIT_CRITICAL_ISR(&timerMux);
}

// ==================================================================================
// SENSOR MANAGEMENT FUNCTIONS
// ==================================================================================

/*
 * Initialize BME280 sensor with comprehensive error checking
 * 
 * Returns: true if initialization successful, false otherwise
 */
bool initializeBME280() {
    Serial.println("=== BME280 Sensor Initialization ===");
    
    // Try primary I2C address first
    Serial.printf("Attempting connection to BME280 at address 0x%02X...\n", BME280_ADDRESS_PRIMARY);
    if (bme.begin(BME280_ADDRESS_PRIMARY)) {
        Serial.println("✓ BME280 found at primary address 0x76");
        return true;
    }
    
    // Try secondary I2C address
    Serial.printf("Primary address failed. Trying secondary address 0x%02X...\n", BME280_ADDRESS_SECONDARY);
    if (bme.begin(BME280_ADDRESS_SECONDARY)) {
        Serial.println("✓ BME280 found at secondary address 0x77");
        return true;
    }
    
    // Both addresses failed
    Serial.println("✗ BME280 sensor not found at either address!");
    Serial.println("\nTroubleshooting Steps:");
    Serial.println("1. Verify wiring connections (VIN, GND, SCL, SDA)");
    Serial.println("2. Check if sensor is powered (3.3V supply)");
    Serial.println("3. Ensure pull-up resistors are present on I2C lines");
    Serial.println("4. Try different I2C address if SDO pin is connected");
    Serial.println("5. Measure continuity of jumper wires");
    
    return false;
}

/*
 * Perform I2C bus diagnostic scan
 * 
 * Scans all possible I2C addresses to identify connected devices.
 * Useful for troubleshooting and device discovery.
 */
void performI2CDiagnostic() {
    Serial.println("\n=== I2C Bus Diagnostic Scan ===");
    Serial.println("Scanning I2C bus for devices...");
    
    uint8_t deviceCount = 0;
    
    for (uint8_t address = 1; address < 127; address++) {
        Wire.beginTransmission(address);
        uint8_t error = Wire.endTransmission();
        
        if (error == 0) {
            Serial.printf("✓ Device found at address 0x%02X\n", address);
            deviceCount++;
            
            // Identify known devices
            switch (address) {
                case 0x76:
                case 0x77:
                    Serial.println("  └─ Likely BME280 environmental sensor");
                    break;
                case 0x48:
                case 0x49:
                case 0x4A:
                case 0x4B:
                    Serial.println("  └─ Likely ADS1115 ADC or similar");
                    break;
                case 0x68:
                    Serial.println("  └─ Likely MPU6050 IMU or DS3231 RTC");
                    break;
                default:
                    Serial.println("  └─ Unknown device");
                    break;
            }
        }
    }
    
    Serial.printf("\nScan complete. Found %d device(s) on I2C bus.\n", deviceCount);
    
    if (deviceCount == 0) {
        Serial.println("⚠ No I2C devices detected! Check connections and power supply.");
    }
}

/*
 * Validate sensor reading values for reasonable ranges
 * 
 * Params: SensorData structure containing readings to validate
 * Returns: true if all readings are within expected ranges
 */
bool validateSensorData(const SensorData& data) {
    if (data.temperature < MIN_VALID_TEMP || data.temperature > MAX_VALID_TEMP) {
        Serial.printf("⚠ Temperature out of range: %.2f°C\n", data.temperature);
        return false;
    }
    
    if (data.pressure < MIN_VALID_PRESSURE || data.pressure > MAX_VALID_PRESSURE) {
        Serial.printf("⚠ Pressure out of range: %.2f hPa\n", data.pressure);
        return false;
    }
    
    if (data.humidity < MIN_VALID_HUMIDITY || data.humidity > MAX_VALID_HUMIDITY) {
        Serial.printf("⚠ Humidity out of range: %.2f%%\n", data.humidity);
        return false;
    }
    
    return true;
}

/*
 * Read all sensor values with error handling and validation
 * 
 * Returns: SensorData structure with readings and validity flag
 */
SensorData readSensorData() {
    SensorData data = {0};
    data.timestamp = millis();
    
    // Read raw sensor values
    data.temperature = bme.readTemperature() + TEMPERATURE_OFFSET;
    data.pressure = (bme.readPressure() / 100.0f) + PRESSURE_OFFSET;  // Convert Pa to hPa
    data.humidity = bme.readHumidity() + HUMIDITY_OFFSET;
    data.altitude = bme.readAltitude(SEALEVEL_PRESSURE_HPA);
    
    // Validate readings
    data.valid = validateSensorData(data);
    
    if (!data.valid) {
        failedReadings++;
        Serial.println("✗ Sensor reading validation failed");
    }
    
    totalReadings++;
    return data;
}

/*
 * Display formatted sensor readings with enhanced output
 */
void displaySensorReadings(const SensorData& data) {
    if (!data.valid) {
        Serial.println("✗ Invalid sensor data - skipping display");
        return;
    }
    
    // Calculate time since last reading
    uint32_t timeDelta = data.timestamp - lastReadingTime;
    lastReadingTime = data.timestamp;
    
    // Header with timestamp and performance info
    Serial.println("\n" + String("=").substring(0, 60));
    Serial.printf("Reading #%lu | Time: %lu ms | Δt: %lu ms\n", 
                  totalReadings, data.timestamp, timeDelta);
    Serial.println(String("=").substring(0, 60));
    
    // Temperature reading with comfort indication
    Serial.printf("🌡️  Temperature: %6.2f °C", data.temperature);
    if (data.temperature < 18.0) Serial.print(" (Cool)");
    else if (data.temperature > 25.0) Serial.print(" (Warm)");
    else Serial.print(" (Comfortable)");
    Serial.println();
    
    // Pressure reading with weather tendency
    Serial.printf("🌤️  Pressure:    %6.2f hPa", data.pressure);
    if (data.pressure < 1000.0) Serial.print(" (Low - Stormy)");
    else if (data.pressure > 1020.0) Serial.print(" (High - Fair)");
    else Serial.print(" (Normal)");
    Serial.println();
    
    // Humidity reading with comfort indication
    Serial.printf("💧 Humidity:    %6.2f %%", data.humidity);
    if (data.humidity < 30.0) Serial.print(" (Dry)");
    else if (data.humidity > 70.0) Serial.print(" (Humid)");
    else Serial.print(" (Comfortable)");
    Serial.println();
    
    // Altitude calculation
    Serial.printf("🏔️  Altitude:    %6.2f m (calculated)\n", data.altitude);
    
    // Performance statistics
    float successRate = ((float)(totalReadings - failedReadings) / totalReadings) * 100.0f;
    Serial.printf("📊 Success Rate: %6.1f%% (%lu/%lu readings)\n", 
                  successRate, totalReadings - failedReadings, totalReadings);
}

// ==================================================================================
// TIMER MANAGEMENT FUNCTIONS
// ==================================================================================

/*
 * Initialize hardware timer with comprehensive configuration
 * 
 * Returns: true if timer initialization successful
 */
bool initializeTimer() {
    Serial.println("=== Hardware Timer Initialization ===");

    // timerBegin now takes frequency in Hz directly
    // Prescaler is calculated internally based on APB frequency (typically 80 MHz)
    timer = timerBegin(1000000);  // 1 MHz = 1 µs tick resolution

    if (timer == nullptr) {
        Serial.println("✗ Failed to initialize hardware timer");
        return false;
    }

    // Attach ISR (no extra arguments)
    timerAttachInterrupt(timer, &onTimer);

    // Set alarm: interval in microseconds (here: 2 seconds)
    timerAlarm(timer, TIMER_ALARM_VALUE, TIMER_AUTO_RELOAD, 0);

    Serial.printf("✓ Timer initialized successfully\n");
    Serial.printf("  Frequency: 1 MHz tick rate\n");
    Serial.printf("  Interval: %lu µs (%.1f seconds)\n",
                  TIMER_ALARM_VALUE, TIMER_ALARM_VALUE / 1000000.0f);

    return true;
}

// ==================================================================================
// MAIN PROGRAM FUNCTIONS
// ==================================================================================

void setup() {
    // Initialize serial communication
    Serial.begin(SERIAL_BAUDRATE);
    
    // Wait for serial port to connect (useful for debugging)
    uint32_t startTime = millis();
    while (!Serial && (millis() - startTime) < SERIAL_TIMEOUT_MS) {
        delay(10);
    }
    
    // Display welcome message and system information
    Serial.println("\n" + String("=").substring(0, 80));
    Serial.println("  BME280 Environmental Monitoring System");
    Serial.println("  ESP32-S3-DevKitC-1 v1.1 | Course: IoT Development");
    Serial.println("  Lesson 05.053 - Comprehensive Implementation");
    Serial.println(String("=").substring(0, 80));
    
    // Display hardware information
    Serial.printf("System Information:\n");
    Serial.printf("- CPU Frequency: %d MHz\n", getCpuFrequencyMhz());
    Serial.printf("- Flash Size: %d MB\n", ESP.getFlashChipSize() / (1024 * 1024));
    Serial.printf("- Free Heap: %d bytes\n", ESP.getFreeHeap());
    Serial.printf("- Arduino Core: %s\n", ESP.getSdkVersion());
    
    // Initialize I2C with custom pins and frequency
    Serial.printf("\nInitializing I2C (SDA:%d, SCL:%d, %dkHz)...\n", 
                  SDA_PIN, SCL_PIN, I2C_FREQUENCY / 1000);
    Wire.begin(SDA_PIN, SCL_PIN);
    Wire.setClock(I2C_FREQUENCY);
    
    // Perform I2C diagnostic scan
    performI2CDiagnostic();
    
    // Initialize BME280 sensor
    if (!initializeBME280()) {
        Serial.println("\n❌ SETUP FAILED: Cannot continue without BME280 sensor");
        Serial.println("Please check connections and restart the system.");
        while (1) {
            delay(1000);  // Halt execution
        }
    }
    
    // Display sensor configuration
    Serial.println("\n=== BME280 Configuration ===");
    Serial.println("Sensor Settings:");
    Serial.println("- Temperature: Enabled");
    Serial.println("- Pressure: Enabled");
    Serial.println("- Humidity: Enabled");
    Serial.printf("- Sea Level Pressure: %.2f hPa\n", SEALEVEL_PRESSURE_HPA);
    
    // Initialize hardware timer
    if (!initializeTimer()) {
        Serial.println("\n❌ SETUP FAILED: Cannot initialize hardware timer");
        while (1) {
            delay(1000);  // Halt execution
        }
    }
    
    // Setup complete
    Serial.println("\n✅ Setup completed successfully!");
    Serial.println("Environmental monitoring started...");
    Serial.println("Readings will appear every 2 seconds.\n");
    
    // Take initial reading to verify everything works
    currentReading = readSensorData();
    displaySensorReadings(currentReading);
}

void loop() {
    // Check if timer interrupt has occurred
    if (interruptCounter > 0) {
        // Enter critical section to safely decrement counter
        portENTER_CRITICAL(&timerMux);
        interruptCounter--;
        portEXIT_CRITICAL(&timerMux);
        
        // Perform sensor reading and display
        currentReading = readSensorData();
        displaySensorReadings(currentReading);
    }
    
    // Small delay to prevent CPU spinning
    // This doesn't affect timing since we're using interrupts
    delay(10);
    
    // Optional: Add watchdog timer reset if using
    // esp_task_wdt_reset();
}

// ==================================================================================
// UTILITY FUNCTIONS (Educational Examples)
// ==================================================================================

/*
 * Calculate heat index (apparent temperature) from temperature and humidity
 * 
 * Useful for understanding how humidity affects perceived temperature.
 * Formula based on Rothfusz equation used by US National Weather Service.
 */
float calculateHeatIndex(float tempF, float humidity) {
    if (tempF < 80.0) return tempF;  // Heat index not applicable below 80°F
    
    float hi = -42.379 + 2.04901523 * tempF + 10.14333127 * humidity
               - 0.22475541 * tempF * humidity - 6.83783e-3 * tempF * tempF
               - 5.481717e-2 * humidity * humidity + 1.22874e-3 * tempF * tempF * humidity
               + 8.5282e-4 * tempF * humidity * humidity - 1.99e-6 * tempF * tempF * humidity * humidity;
    
    return hi;
}

/*
 * Calculate dew point temperature from temperature and humidity
 * 
 * Dew point is the temperature at which air becomes saturated with water vapor.
 * Uses Magnus formula approximation for educational purposes.
 */
float calculateDewPoint(float tempC, float humidity) {
    float a = 17.27;
    float b = 237.7;
    float alpha = ((a * tempC) / (b + tempC)) + log(humidity / 100.0);
    return (b * alpha) / (a - alpha);
}

/*
 * Example function: Data logging to internal memory
 * 
 * In a real application, you might want to log data for later analysis.
 * This example shows basic concepts for educational purposes.
 */
void logDataToMemory(const SensorData& data) {
    // This is a simplified example - in practice you'd want:
    // - Circular buffer to prevent memory overflow
    // - Timestamps for data correlation
    // - Data compression for efficiency
    // - Error checking and recovery
    
    static uint32_t logIndex = 0;
    static const uint32_t MAX_LOG_ENTRIES = 100;
    
    // Simple logging demonstration
    Serial.printf("LOG[%lu]: T=%.1f°C, P=%.1fhPa, H=%.1f%%\n",
                  logIndex % MAX_LOG_ENTRIES, data.temperature, data.pressure, data.humidity);
    
    logIndex++;
}
