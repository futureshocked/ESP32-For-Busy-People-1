/*  05.031 - Simple Photoresistor Light Sensor for ESP32-S3
 * 
 * Course: IoT Development with ESP32-S3
 * Target Hardware: ESP32-S3-DevKitC-1 v1.1
 * Module: ESP32-S3-WROOM-1-N16R16V
 * 
 * This sketch shows you how to read ambient light intensity with a photoresistor.
 * After uploading the sketch to your ESP32-S3, open the serial monitor to see the
 * ADC value and calculated voltage from the photoresistor.
 * 
 * WHAT YOU'LL LEARN:
 * ==================
 * - How to read analog sensors with ESP32-S3's ADC
 * - Understanding voltage divider circuits
 * - Converting ADC readings to real voltage values
 * - Basic sensor data interpretation
 * 
 * CIRCUIT DIAGRAM:
 * ================
 * 
 *     3.3V
 *       |
 *    [PHOTO]  <- Photoresistor (LDR)
 *       |
 *       +------- GPIO4 (connect to ESP32-S3)
 *       |
 *     [10kΩ]  <- Regular resistor
 *       |
 *      GND
 * 
 * HOW IT WORKS:
 * =============
 * - Bright light = Low photoresistor resistance = Higher voltage at GPIO4
 * - Dark conditions = High photoresistor resistance = Lower voltage at GPIO4
 * - ESP32-S3 ADC converts voltage (0-3.3V) to numbers (0-4095)
 * 
 * COMPONENTS NEEDED:
 * ==================
 * 1x ESP32-S3-DevKitC-1 board
 * 1x Photoresistor (any common LDR like GL5528)
 * 1x 10kΩ resistor (brown-black-orange stripes)
 * 1x Breadboard
 * 3x Jumper wires
 * 1x USB-C cable
 * 
 * CONNECTIONS:
 * ============
 * Photoresistor pin 1 → 3.3V on ESP32-S3
 * Photoresistor pin 2 → GPIO4 on ESP32-S3
 * Photoresistor pin 2 → 10kΩ resistor
 * 10kΩ resistor → GND on ESP32-S3
 * 
 * ARDUINO IDE SETUP:
 * ===================
 * - Install ESP32 Arduino Core (v2.0.0 or newer)
 * - Board: "ESP32S3 Dev Module" 
 * - USB Mode: "Hardware CDC and JTAG"
 * - Upload Speed: 921600
 * 
 * Created: March 27, 2019 by Peter Dalmaris
 * Simplified: July 26, 2025 for ESP32-S3 beginner students
 */

// ========================================================================================
// CONFIGURATION - Change these if needed
// ========================================================================================

const int PHOTORESISTOR_PIN = 4;        // GPIO4 - Connect photoresistor here
const float REFERENCE_VOLTAGE = 3.3;    // ESP32-S3 voltage reference

// ========================================================================================
// SETUP FUNCTION - Runs once when board starts
// ========================================================================================

void setup() {
    // Start serial communication so we can see results
    Serial.begin(115200);
    delay(1000);  // Wait for serial to be ready
    
    // Configure the ADC for 12-bit resolution (0-4095)
    analogReadResolution(12);
    
    // Configure the photoresistor pin as input
    pinMode(PHOTORESISTOR_PIN, INPUT);
    
    // Print startup message
    Serial.println("ESP32-S3 Photoresistor Demo");
    Serial.println("=============================");
    Serial.println("Cover the photoresistor to see values change!");
    Serial.println();
    Serial.println("ADC Value\tVoltage\t\tLight Level");
    Serial.println("=========\t=======\t\t===========");
}

// ========================================================================================
// MAIN LOOP - Runs continuously
// ========================================================================================

void loop() {
    // Read the photoresistor value (0-4095)
    int adcValue = analogRead(PHOTORESISTOR_PIN);
    
    // Convert ADC reading to actual voltage (0.0-3.3V)
    float voltage = (adcValue * REFERENCE_VOLTAGE) / 4095.0;
    
    // Determine light level based on ADC value
    String lightLevel;
    if (adcValue > 3000) {
        lightLevel = "BRIGHT";
    } else if (adcValue > 2000) {
        lightLevel = "NORMAL";
    } else if (adcValue > 1000) {
        lightLevel = "DIM";
    } else {
        lightLevel = "DARK";
    }
    
    // Display the results in a neat table format
    Serial.print(adcValue);
    Serial.print("\t\t");
    Serial.print(voltage, 2);  // Show 2 decimal places
    Serial.print("V\t\t");
    Serial.println(lightLevel);
    
    // Wait half a second before next reading
    delay(500);
}
