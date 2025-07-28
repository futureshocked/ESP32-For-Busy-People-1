/*  05.071 - ADXL335 Acceleration Sensor (Minimal Version) for ESP32-S3
 * 
 * Course: IoT Development with ESP32-S3
 * Target Hardware: ESP32-S3-DevKitC-1 v1.1
 * Module: ESP32-S3-WROOM-1-N16R16V
 * 
 * This minimal sketch shows the basic operation of the ADXL335 analog accelerometer
 * with the ESP32-S3. Perfect for beginners learning sensor interfacing fundamentals.
 * 
 * HARDWARE INFORMATION:
 * =====================
 * - Board: ESP32-S3-DevKitC-1 v1.1
 * - Module: ESP32-S3-WROOM-1-N16R16V
 * - ADC Resolution: 12-bit (0-4095)
 * - Operating Voltage: 3.3V
 * 
 * CIRCUIT DIAGRAM:
 * ================
 *     ESP32-S3             ADXL335
 *     ┌─────────┐         ┌─────────┐
 *     │     3V3 ├─────────┤ VCC     │
 *     │     GND ├─────────┤ GND     │
 *     │   GPIO1 ├─────────┤ X-OUT   │
 *     │   GPIO2 ├─────────┤ Y-OUT   │
 *     │   GPIO3 ├─────────┤ Z-OUT   │
 *     └─────────┘         └─────────┘
 * 
 * COMPONENTS:
 * ===========
 * - 1x ESP32-S3-DevKitC-1
 * - 1x ADXL335 Accelerometer Breakout
 * - 4x Jumper Wires
 * 
 * GPIO ASSIGNMENTS:
 * =================
 * GPIO1 - X-axis (ADC1_CH0)
 * GPIO2 - Y-axis (ADC1_CH1)  
 * GPIO3 - Z-axis (ADC1_CH2)
 * 
 * TECHNICAL NOTES:
 * ================
 * - GPIO35, 36, 37 avoided (reserved for internal flash/PSRAM)
 * - 12-bit ADC resolution provides values 0-4095
 * - ADXL335 outputs ~1.65V at 0g, ±0.3V/g sensitivity
 * 
 * IDE REQUIREMENTS:
 * =================
 * - Arduino IDE 2.0+
 * - ESP32 Arduino Core v3.0+
 * - Board: "ESP32S3 Dev Module"
 * 
 * Created: March 28 2019 by Peter Dalmaris
 * Updated: December 2024 for ESP32-S3 compatibility
 */

// ============================================================================
// CONFIGURATION
// ============================================================================
// GPIO pins for accelerometer axes (using safe ADC1 pins)
const uint8_t X_AXIS_PIN = 1;  // GPIO1 - X-axis output
const uint8_t Y_AXIS_PIN = 2;  // GPIO2 - Y-axis output  
const uint8_t Z_AXIS_PIN = 3;  // GPIO3 - Z-axis output

// ADC and sensor settings
const uint8_t ADC_BITS = 12;           // 12-bit ADC resolution
const uint16_t ADC_MAX = 4095;         // Maximum ADC value (2^12 - 1)
const float SUPPLY_VOLTAGE = 3.3f;     // ESP32-S3 supply voltage

// Orientation thresholds (experimentally determined for 12-bit ADC)
const uint16_t HORIZONTAL_THRESHOLD = 2800;  // Z-axis value when horizontal
const uint16_t VERTICAL_X_THRESHOLD = 1500;  // X-axis value when tilted
const uint16_t VERTICAL_Y_THRESHOLD = 1500;  // Y-axis value when tilted
const uint16_t SIDE_THRESHOLD_LOW = 1400;    // Lower bound for side detection
const uint16_t SIDE_THRESHOLD_HIGH = 2600;   // Upper bound for side detection

// Timing
const uint16_t SAMPLE_DELAY_MS = 100;  // Delay between readings
const uint32_t BAUD_RATE = 115200;     // Serial communication speed

// ============================================================================
// SETUP FUNCTION
// ============================================================================
void setup() {
    // Initialize serial communication
    Serial.begin(BAUD_RATE);
    while (!Serial && millis() < 3000) {
        delay(10); // Wait up to 3 seconds for serial connection
    }
    
    // Set ADC resolution to 12 bits for better precision
    analogReadResolution(ADC_BITS);
    
    // Print startup information
    Serial.println("\n" + String('=', 50));
    Serial.println("   ESP32-S3 ADXL335 Accelerometer (Simple)");
    Serial.println(String('=', 50));
    Serial.printf("ADC Resolution: %d bits (0-%d)\n", ADC_BITS, ADC_MAX);
    Serial.printf("GPIO Pins - X:%d, Y:%d, Z:%d\n", X_AXIS_PIN, Y_AXIS_PIN, Z_AXIS_PIN);
    Serial.println("Format: X-Value  Y-Value  Z-Value  Orientation");
    Serial.println(String('-', 50));
    
    delay(2000); // Give time to read startup info
}

// ============================================================================
// MAIN LOOP
// ============================================================================
void loop() {
    // Read raw ADC values from all three axes
    uint16_t xValue = analogRead(X_AXIS_PIN);
    uint16_t yValue = analogRead(Y_AXIS_PIN);
    uint16_t zValue = analogRead(Z_AXIS_PIN);
    
    // Display the raw sensor values with tabs for alignment
    Serial.print(xValue);
    Serial.print("\t\t");
    Serial.print(yValue);
    Serial.print("\t\t");
    Serial.print(zValue);
    
    // Determine orientation based on sensor readings
    // Note: These thresholds are for 12-bit ADC and may need adjustment
    
    // Check if device is horizontal (Z-axis pointing up)
    if (zValue > HORIZONTAL_THRESHOLD) {
        Serial.print("\t\tHorizontal (face up)");
    }
    
    // Check if device is tilted forward/backward (X-axis)
    if (xValue < VERTICAL_X_THRESHOLD) {
        Serial.print("\t\tTilted forward");
    }
    
    // Check if device is on its side (Y-axis)
    if ((yValue > SIDE_THRESHOLD_LOW && yValue < SIDE_THRESHOLD_HIGH)) {
        Serial.print("\t\tOn side");
    }
    
    // Move to next line
    Serial.println();
    
    // Wait before next reading
    delay(SAMPLE_DELAY_MS);
}
