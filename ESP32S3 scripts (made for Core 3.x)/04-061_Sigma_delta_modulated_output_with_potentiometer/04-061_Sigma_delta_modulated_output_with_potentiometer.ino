/*  04.060 - PWM LED Control with Potentiometer for ESP32-S3
 * 
 * This sketch demonstrates how to create a PWM-controlled analog output using a potentiometer
 * for smooth LED brightness control on the ESP32-S3 platform.
 * 
 * IMPORTANT CHANGES FROM ORIGINAL ESP32:
 * - Sigma-Delta modulation is NOT available on ESP32-S3
 * - Uses LEDC (LED Control) peripheral instead for hardware PWM
 * - GPIO pins updated for ESP32-S3 compatibility
 * - Enhanced with detailed hardware documentation
 * 
 * HARDWARE INFORMATION:
 * =====================
 * Target Board: ESP32-S3-DevKitC-1 v1.1
 * MCU: ESP32-S3-WROOM-1 (N8R8 or N16R16V)
 * Flash: 8MB or 16MB
 * PSRAM: 8MB or 16MB
 * 
 * FEATURES USED:
 * - ADC1 for analog input reading
 * - LEDC peripheral for PWM output
 * - 12-bit ADC resolution (0-4095)
 * - 8-bit PWM resolution (0-255)
 * 
 * CIRCUIT DIAGRAM:
 * ================
 * 
 *     ESP32-S3-DevKitC-1
 *         ┌─────────┐
 *         │ 3V3  GND│
 *         │ RST   TX│
 *         │ 4    RX │
 *         │ 5     1 │◄── Potentiometer middle pin
 *         │ 6     2 │◄── LED Anode (via 330Ω resistor)
 *         │ 7    42 │
 *         │ ...  ...│
 *         └─────────┘
 * 
 * POTENTIOMETER WIRING:
 * Pin 1 (CCW) ──► 3V3
 * Pin 2 (Wiper) ──► GPIO1 (ADC1_CH0)
 * Pin 3 (CW) ──► GND
 * 
 * LED WIRING:
 * GPIO2 ──► 330Ω Resistor ──► LED Anode
 * LED Cathode ──► GND
 * 
 * This sketch was originally written by Peter Dalmaris and updated for ESP32-S3
 * by incorporating LEDC PWM functionality and ESP32-S3 specific GPIO assignments.
 * 
 * COMPONENTS:
 * ===========
 * - ESP32-S3-DevKitC-1 development board
 * - Potentiometer, 10KΩ linear taper
 * - LED (any standard 3mm or 5mm LED)
 * - Resistor, 330Ω (for LED current limiting)
 * - Breadboard and jumper wires
 * 
 * IDE REQUIREMENTS:
 * =================
 * - Arduino IDE 2.0+ with ESP32 Arduino Core
 * - ESP32 Arduino Core v2.0.8 or newer
 * - Board: "ESP32S3 Dev Module" or "ESP32-S3-DevKitC-1"
 * 
 * LIBRARIES:
 * ==========
 * - No external libraries required (uses built-in ESP32 LEDC library)
 * 
 * GPIO ASSIGNMENTS:
 * =================
 * GPIO1 (ADC1_CH0): Potentiometer analog input
 * GPIO2: LED PWM output
 * 
 * TECHNICAL NOTES:
 * ================
 * 1. ESP32-S3 uses LEDC peripheral instead of Sigma-Delta for PWM generation
 * 2. LEDC provides better precision and lower noise than sigma-delta
 * 3. ADC resolution is 12-bit (0-4095) on ESP32-S3
 * 4. PWM frequency set to 1kHz for smooth LED dimming
 * 5. GPIO35, GPIO36, GPIO37 are reserved for internal SPI flash/PSRAM communication
 * 6. Uses newer LEDC API (Arduino Core v3.0+): ledcAttach() instead of ledcSetup()
 * 7. PWM channels are automatically managed by the newer API
 * 
 * REFERENCES:
 * ===========
 * 1. ESP32-S3 Datasheet: https://www.espressif.com/sites/default/files/documentation/esp32-s3_datasheet_en.pdf
 * 2. ESP32-S3 Technical Reference: https://www.espressif.com/sites/default/files/documentation/esp32-s3_technical_reference_manual_en.pdf
 * 3. LEDC Documentation: https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/api-reference/peripherals/ledc.html
 * 4. Arduino ESP32 v3.0+ LEDC API: https://docs.espressif.com/projects/arduino-esp32/en/latest/api/ledc.html
 * 
 * Created: March 26, 2019 by Peter Dalmaris
 * Updated: July 25, 2025 for ESP32-S3 compatibility with Arduino Core v3.0+lity
 * 
 */

// ================================
// GPIO PIN DEFINITIONS
// ================================
const byte POT_GPIO = 1;    // GPIO1 - ADC1_CH0 for potentiometer input
const byte LED_GPIO = 2;    // GPIO2 - PWM output for LED control

// ================================
// PWM CONFIGURATION CONSTANTS
// ================================
const uint32_t PWM_FREQUENCY = 1000;  // PWM frequency in Hz (1kHz for smooth dimming)
const byte PWM_RESOLUTION = 8;   // PWM resolution in bits (8-bit = 0-255)

// ================================
// ADC CONFIGURATION CONSTANTS
// ================================
const uint16_t ADC_MAX_VALUE = 4095;  // 12-bit ADC maximum value
const byte PWM_MAX_VALUE = 255;       // 8-bit PWM maximum value

// ================================
// SETUP FUNCTION
// ================================
void setup() {
    // Initialize serial communication for debugging
    Serial.begin(115200);
    
    // Wait for serial port to initialize
    while (!Serial && millis() < 3000) {
        delay(10);
    }
    
    Serial.println("\n==================================================");
    Serial.println("ESP32-S3 PWM LED Control with Potentiometer");
    Serial.println("==================================================");
    Serial.println("Hardware: ESP32-S3-DevKitC-1");
    Serial.println("Potentiometer: GPIO" + String(POT_GPIO) + " (ADC1_CH0)");
    Serial.println("LED Output: GPIO" + String(LED_GPIO) + " (PWM)");
    Serial.println("PWM Frequency: " + String(PWM_FREQUENCY) + " Hz");
    Serial.println("PWM Resolution: " + String(PWM_RESOLUTION) + " bits");
    Serial.println("==================================================\n");
    
    // Configure ADC resolution (12-bit = 0-4095)
    analogReadResolution(12);
    
    // Setup LEDC PWM using newer API (ESP32 Arduino Core v3.0+)
    // Parameters: pin, frequency, resolution_bits
    if (!ledcAttach(LED_GPIO, PWM_FREQUENCY, PWM_RESOLUTION)) {
        Serial.println("ERROR: Failed to configure PWM on GPIO" + String(LED_GPIO));
        while(1) delay(1000); // Stop execution if PWM setup fails
    }
    
    // Initialize LED to OFF state
    ledcWrite(LED_GPIO, 0);
    
    Serial.println("Setup complete! Turn the potentiometer to control LED brightness.");
    Serial.println("Monitoring values...\n");
}

// ================================
// MAIN LOOP FUNCTION
// ================================
void loop() {
    // Read analog value from potentiometer (0-4095)
    uint16_t pot_raw_value = analogRead(POT_GPIO);
    
    // Map potentiometer value to PWM range (0-255)
    // map(value, fromLow, fromHigh, toLow, toHigh)
    uint8_t pwm_value = map(pot_raw_value, 0, ADC_MAX_VALUE, 0, PWM_MAX_VALUE);
    
    // Apply PWM value to LED using newer API
    ledcWrite(LED_GPIO, pwm_value);
    
    // Calculate brightness percentage for display
    float brightness_percent = (pwm_value / (float)PWM_MAX_VALUE) * 100.0;
    
    // Output debug information every 100ms
    static unsigned long last_print_time = 0;
    if (millis() - last_print_time >= 100) {
        Serial.printf("ADC: %4d | PWM: %3d | Brightness: %5.1f%%\n", 
                     pot_raw_value, pwm_value, brightness_percent);
        last_print_time = millis();
    }
    
    // Small delay to prevent excessive CPU usage
    delay(10);
}

// ================================
// UTILITY FUNCTIONS
// ================================

/*
 * Function: getPWMFrequency
 * Returns the current PWM frequency
 */
uint32_t getPWMFrequency() {
    return PWM_FREQUENCY;
}

/*
 * Function: setPWMFrequency
 * Allows runtime change of PWM frequency
 * Parameter: freq - new frequency in Hz
 */
void setPWMFrequency(uint32_t freq) {
    // Detach and reattach with new frequency
    ledcDetach(LED_GPIO);
    if (!ledcAttach(LED_GPIO, freq, PWM_RESOLUTION)) {
        Serial.println("ERROR: Failed to reconfigure PWM frequency");
        // Fallback to original frequency
        ledcAttach(LED_GPIO, PWM_FREQUENCY, PWM_RESOLUTION);
    }
}

/*
 * Function: getBrightnessPercent
 * Returns current LED brightness as percentage
 */
float getBrightnessPercent() {
    uint16_t current_adc = analogRead(POT_GPIO);
    return (map(current_adc, 0, ADC_MAX_VALUE, 0, PWM_MAX_VALUE) / (float)PWM_MAX_VALUE) * 100.0;
}
