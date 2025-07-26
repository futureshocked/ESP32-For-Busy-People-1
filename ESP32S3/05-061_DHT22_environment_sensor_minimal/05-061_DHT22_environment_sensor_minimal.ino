/*  05.061 - DHT22 Environment Sensor (Minimal Version) for ESP32-S3
 * 
 * Course: IoT Development with ESP32-S3
 * Target Hardware: ESP32-S3-DevKitC-1 v1.1
 * Module: ESP32-S3-WROOM-1-N16R16V
 * 
 * This minimal sketch demonstrates basic DHT22 temperature and humidity sensor
 * reading with the ESP32-S3. Perfect for beginners learning sensor interfacing
 * and students who need a quick working prototype.
 * 
 * LEARNING OBJECTIVES:
 * ===================
 * - Read digital temperature and humidity data
 * - Understand basic sensor timing requirements
 * - Display sensor data on serial monitor
 * - Handle basic sensor communication errors
 * 
 * HARDWARE INFORMATION:
 * =====================
 * ESP32-S3-DevKitC-1 v1.1:
 * - Dual-core processor running at 240MHz
 * - 16MB Flash memory, 16MB PSRAM
 * - 45 programmable GPIO pins
 * - Native USB support via USB-C
 * 
 * DHT22 (AM2302) Sensor:
 * - Measures temperature: -40°C to 80°C
 * - Measures humidity: 0% to 100% RH
 * - Digital output (no analog conversion needed)
 * - Requires 2 seconds between readings
 * 
 * CIRCUIT DIAGRAM:
 * ================
 *     ESP32-S3-DevKitC-1              DHT22 Sensor
 *     ┌─────────────┐                ┌─────────────┐
 *     │         3V3 ├────────────────┤ VCC (Pin 1) │
 *     │             │                │             │
 *     │      GPIO2  ├────────────────┤ DATA(Pin 2) │
 *     │             │           │    │             │
 *     │             │          ┌┴┐   │ NC  (Pin 3) │ (Not Connected)
 *     │             │     10kΩ │ │   │             │
 *     │             │          │ │   │ GND (Pin 4) │
 *     │             │          └┬┘   │             │
 *     │         GND ├───────────┼────┤             │
 *     └─────────────┘           │    └─────────────┘
 *                               │
 *                    Pull-up Resistor
 *                   (connects DATA to VCC)
 * 
 * COMPONENTS:
 * ===========
 * - 1x ESP32-S3-DevKitC-1 v1.1 board
 * - 1x DHT22 (AM2302) sensor
 * - 1x 10kΩ resistor (brown-black-orange-gold bands)
 * - 1x Breadboard
 * - 4x Jumper wires
 * - 1x USB-C cable
 * 
 * CONNECTIONS:
 * ============
 * ESP32-S3    DHT22    Description
 * --------    -----    -----------
 * 3V3         Pin 1    Power supply (3.3V)
 * GPIO2       Pin 2    Data line (with 10kΩ pull-up to 3V3)
 * (none)      Pin 3    Not connected
 * GND         Pin 4    Ground
 * 
 * IMPORTANT NOTES:
 * ================
 * - The 10kΩ pull-up resistor is REQUIRED on the data line
 * - Wait at least 2 seconds between sensor readings
 * - GPIO2 is safe to use on ESP32-S3 (avoid GPIO35-37)
 * - DHT22 pin 1 has a small notch or dot marking
 * 
 * IDE REQUIREMENTS:
 * =================
 * - Arduino IDE 2.0+
 * - ESP32 Arduino Core v3.0+
 * - Board setting: "ESP32S3 Dev Module"
 * - Install library: "DHTesp" by beegee_tokyo
 * 
 * TROUBLESHOOTING:
 * ================
 * Problem: "Failed to read" errors
 * Solution: Check the 10kΩ pull-up resistor is connected
 * 
 * Problem: Always reads 0 or strange values
 * Solution: Verify DHT22 wiring and orientation
 * 
 * Problem: No serial output
 * Solution: Check serial monitor baud rate is 115200
 * 
 * Created: March 28, 2019 by Peter Dalmaris
 * Updated: July 27, 2025 for ESP32-S3 compatibility and beginner focus
 */

#include "DHTesp.h"

// ================================================================================================
// CONFIGURATION
// ================================================================================================

const int DHT_PIN = 2;              // GPIO pin connected to DHT22 data line
const int SERIAL_SPEED = 115200;    // Serial monitor communication speed
const int READ_DELAY = 2500;       // Delay between readings (milliseconds)

// ================================================================================================
// GLOBAL VARIABLES
// ================================================================================================

DHTesp dhtSensor;                   // Create DHT sensor object
int readingCount = 0;               // Count how many readings we've taken

// ================================================================================================
// UTILITY FUNCTIONS
// ================================================================================================

/*
 * Helper function to create repeated characters (Arduino String doesn't have repeat)
 */
String repeatChar(char character, int count) {
    String result = "";
    for (int i = 0; i < count; i++) {
        result += character;
    }
    return result;
}

// ================================================================================================
// SETUP FUNCTION (runs once at startup)
// ================================================================================================

void setup() {
    // Start serial communication for displaying results
    Serial.begin(SERIAL_SPEED);
    
    // Wait a moment for serial connection
    delay(1000);
    
    // Print startup message
    Serial.println("=====================================");
    Serial.println("    DHT22 Sensor - ESP32-S3");
    Serial.println("    Simple Temperature & Humidity");
    Serial.println("=====================================");
    Serial.println();
    
    // Initialize the DHT22 sensor
    Serial.println("Initializing DHT22 sensor...");
    dhtSensor.setup(DHT_PIN, DHTesp::DHT22);
    
    // Give sensor time to stabilize
    Serial.println("Waiting for sensor to stabilize...");
    delay(2000);
    
    Serial.println("Sensor ready! Starting measurements...");
    Serial.println();
}

// ================================================================================================
// MAIN LOOP (runs continuously)
// ================================================================================================

void loop() {
    // Increment reading counter
    readingCount++;
    
    // Print reading header
    Serial.println(repeatChar('=', 42));
    Serial.print("Reading #");
    Serial.println(readingCount);
    Serial.println(repeatChar('=', 42));
    
    // Read temperature and humidity from sensor
    TempAndHumidity data = dhtSensor.getTempAndHumidity();
    
    // Check if reading was successful
    if (dhtSensor.getStatus() != 0) {
        // Reading failed - show error message
        Serial.println("❌ ERROR: Failed to read from DHT22 sensor");
        Serial.print("Error code: ");
        Serial.println(dhtSensor.getStatus());
        Serial.println();
        Serial.println("💡 Check these things:");
        Serial.println("   - Is the 10kΩ pull-up resistor connected?");
        Serial.println("   - Are all wires connected properly?");
        Serial.println("   - Is the sensor getting 3.3V power?");
        Serial.println();
    } else {
        // Reading successful - display the data
        Serial.println("✅ Sensor reading successful!");
        Serial.println();
        
        // Display temperature
        Serial.print("🌡️  Temperature: ");
        Serial.print(data.temperature, 1);  // Show 1 decimal place
        Serial.print(" °C");
        
        // Also show in Fahrenheit
        float fahrenheit = (data.temperature * 9.0/5.0) + 32.0;
        Serial.print(" (");
        Serial.print(fahrenheit, 1);
        Serial.println(" °F)");
        
        // Display humidity
        Serial.print("💧 Humidity: ");
        Serial.print(data.humidity, 1);     // Show 1 decimal place
        Serial.println(" %");
        
        Serial.println();
        
        // Simple comfort assessment
        checkComfort(data.temperature, data.humidity);
    }
    
    // Show when next reading will occur
    Serial.print("⏱️  Next reading in ");
    Serial.print(READ_DELAY / 1000);
    Serial.println(" seconds...");
    Serial.println();
    
    // Wait before taking next reading
    // DHT22 needs at least 2 seconds between readings
    delay(READ_DELAY);
}

// ================================================================================================
// HELPER FUNCTIONS
// ================================================================================================

/*
 * Simple comfort assessment function
 * Tells user if temperature and humidity are comfortable
 */
void checkComfort(float temperature, float humidity) {
    Serial.println("🏠 Comfort Assessment:");
    
    // Check temperature comfort (ideal range: 20-26°C)
    if (temperature < 18) {
        Serial.println("   Temperature: Too Cold ❄️");
    } else if (temperature > 28) {
        Serial.println("   Temperature: Too Hot 🔥");
    } else {
        Serial.println("   Temperature: Comfortable ✅");
    }
    
    // Check humidity comfort (ideal range: 40-60%)
    if (humidity < 30) {
        Serial.println("   Humidity: Too Dry 🏜️");
    } else if (humidity > 70) {
        Serial.println("   Humidity: Too Humid 💦");
    } else {
        Serial.println("   Humidity: Comfortable ✅");
    }
    
    // Overall comfort
    if (temperature >= 18 && temperature <= 28 && humidity >= 30 && humidity <= 70) {
        Serial.println("   Overall: Perfect conditions! 😊");
    } else {
        Serial.println("   Overall: Could be more comfortable 🤔");
    }
    
    Serial.println();
}
