/*  05.051 - BME280 Environmental Sensor (Minimal Version) for ESP32-S3
 * 
 * Course: IoT Development with ESP32-S3
 * Target Hardware: ESP32-S3-DevKitC-1 v1.1
 * Module: ESP32-S3-WROOM-1-N16R16V
 * 
 * This simplified sketch shows how to read temperature, humidity, and barometric 
 * pressure from the BME280 sensor using I2C communication. Perfect for beginners 
 * learning environmental sensing basics.
 * 
 * LEARNING OBJECTIVES:
 * ====================
 * - Learn basic I2C sensor communication
 * - Read environmental data from BME280
 * - Display sensor readings on Serial Monitor
 * - Understand sensor initialization and error checking
 * 
 * HARDWARE INFORMATION:
 * =====================
 * ESP32-S3-DevKitC-1 v1.1:
 * - Dual-core processor with built-in WiFi/Bluetooth
 * - Native USB support via USB-C connector
 * - Multiple GPIO pins for sensor connections
 * 
 * BME280 Environmental Sensor:
 * - Measures temperature, humidity, and pressure
 * - Uses I2C communication (2 wires + power)
 * - Very accurate and reliable sensor
 * 
 * CIRCUIT DIAGRAM:
 * ================
 *                ESP32-S3-DevKitC-1              BME280 Breakout
 *                ==================              ================
 *                       3.3V  ●─────────────────● VIN
 *                        GND  ●─────────────────● GND  
 *                 GPIO21(SDA) ●─────────────────● SDA
 *                 GPIO22(SCL) ●─────────────────● SCL
 * 
 * COMPONENTS:
 * ===========
 * 1x ESP32-S3-DevKitC-1 v1.1 development board
 * 1x BME280 environmental sensor breakout board  
 * 4x Jumper wires (male-to-female)
 * 1x USB-C cable
 * 
 * GPIO ASSIGNMENTS:
 * =================
 * GPIO21 - I2C SDA (Serial Data)
 * GPIO22 - I2C SCL (Serial Clock)
 * 
 * TECHNICAL NOTES:
 * ================
 * - BME280 I2C address is usually 0x76 (sometimes 0x77)
 * - Use 3.3V power (NOT 5V) to avoid damaging the sensor
 * - GPIO21 and GPIO22 are the default I2C pins for ESP32-S3
 * - Sea level pressure is set to 1013.25 hPa (standard atmosphere)
 * 
 * IDE REQUIREMENTS:
 * =================
 * - Arduino IDE 2.0+
 * - ESP32 Arduino Core v3.0+
 * - Board: "ESP32S3 Dev Module"
 * - Libraries: Install "Adafruit BME280 Library" and "Adafruit Unified Sensor"
 * 
 * INSTALLATION STEPS:
 * ===================
 * 1. Open Arduino IDE
 * 2. Go to Tools > Manage Libraries
 * 3. Search for "Adafruit BME280" and install
 * 4. Search for "Adafruit Unified Sensor" and install
 * 5. Select Board: "ESP32S3 Dev Module"
 * 6. Upload this sketch
 * 
 * Created: March 28, 2019 by Peter Dalmaris  
 * Updated: January 26, 2025 for ESP32-S3 compatibility (Minimal Version)
 */

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

// ================================================================================
// CONFIGURATION
// ================================================================================

// Sensor settings
const uint8_t BME280_I2C_ADDRESS = 0x76;      // BME280 I2C address (try 0x77 if this doesn't work)
const float SEALEVEL_PRESSURE = 1013.25;      // Standard sea level pressure in hPa
const unsigned long READ_DELAY = 2000;        // Wait 2 seconds between readings

// Create BME280 sensor object
Adafruit_BME280 bme;

// ================================================================================
// SETUP FUNCTION - Runs once when ESP32 starts
// ================================================================================

void setup() {
    // Start serial communication at 115200 baud rate
    Serial.begin(115200);
    
    // Wait a moment for serial to initialize
    delay(1000);
    
    // Print welcome message
    Serial.println("\n=== BME280 Environmental Sensor Test ===");
    Serial.println("ESP32-S3 + BME280 Sensor");
    Serial.println("=========================================\n");
    
    // Initialize I2C communication on default pins
    // GPIO21 = SDA, GPIO22 = SCL
    Wire.begin();
    
    Serial.print("Initializing BME280 sensor...");
    
    // Try to initialize the BME280 sensor
    if (bme.begin(BME280_I2C_ADDRESS)) {
        Serial.println(" Success!");
        Serial.printf("Sensor found at I2C address: 0x%02X\n", BME280_I2C_ADDRESS);
    } else {
        // If sensor not found, try alternate address
        Serial.print(" trying alternate address...");
        if (bme.begin(0x77)) {
            Serial.println(" Success at 0x77!");
        } else {
            Serial.println(" FAILED!");
            Serial.println("\nERROR: Could not find BME280 sensor!");
            Serial.println("Please check:");
            Serial.println("1. Wiring connections");
            Serial.println("2. Power supply (3.3V)");
            Serial.println("3. I2C address (0x76 or 0x77)");
            
            // Stop the program here
            while(1) {
                delay(1000);
                Serial.println("Fix sensor connection and restart ESP32");
            }
        }
    }
    
    Serial.println("\nSensor initialized successfully!");
    Serial.println("Starting measurements...\n");
}

// ================================================================================
// MAIN LOOP - Runs continuously
// ================================================================================

void loop() {
    // Read all sensor values
    float temperature = bme.readTemperature();           // Temperature in Celsius
    float humidity = bme.readHumidity();                 // Humidity in %
    float pressure = bme.readPressure() / 100.0F;       // Pressure in hPa (convert from Pa)
    float altitude = bme.readAltitude(SEALEVEL_PRESSURE); // Altitude in meters
    
    // Display the readings
    Serial.println("--- Environmental Readings ---");
    
    // Temperature in Celsius and Fahrenheit
    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.print(" °C  (");
    Serial.print((temperature * 9.0/5.0) + 32);  // Convert to Fahrenheit
    Serial.println(" °F)");
    
    // Humidity percentage
    Serial.print("Humidity:    ");
    Serial.print(humidity);
    Serial.println(" %");
    
    // Pressure in hPa
    Serial.print("Pressure:    ");
    Serial.print(pressure);
    Serial.println(" hPa");
    
    // Calculated altitude
    Serial.print("Altitude:    ");
    Serial.print(altitude);
    Serial.println(" meters");
    
    Serial.println(); // Empty line for readability
    
    // Wait before next reading
    delay(READ_DELAY);
}
