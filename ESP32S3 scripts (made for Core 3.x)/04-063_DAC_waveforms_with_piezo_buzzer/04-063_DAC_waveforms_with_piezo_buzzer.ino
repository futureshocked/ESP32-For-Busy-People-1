/*  04.063 - PWM Waveform Generator with Piezo Buzzer for ESP32-S3
 * 
 * Course: IoT Development with ESP32-S3
 * Target Hardware: ESP32-S3-DevKitC-1 v1.1
 * Module: ESP32-S3-WROOM-1-N16R16V
 * 
 * This sketch demonstrates how to generate various audio waveforms using PWM
 * (Pulse Width Modulation) and a piezo buzzer. Unlike the original ESP32,
 * ESP32-S3 does not have built-in DAC channels, so we use the LEDC peripheral
 * to create PWM signals that simulate analog output.
 * 
 * The script generates triangle, sine, and square waveforms that are converted
 * to audio by the piezo buzzer through PWM frequency modulation.
 * 
 * HARDWARE INFORMATION:
 * =====================
 * MCU: ESP32-S3 (Xtensa dual-core 32-bit LX7 @ 240MHz)
 * Flash: 16 MB (external)
 * PSRAM: 16 MB (external QSPI)
 * GPIO: 45 programmable GPIOs
 * PWM: 8 independent channels via LEDC peripheral
 * ADC: 12-bit resolution (0-4095 range)
 * Operating Voltage: 3.3V
 * 
 * CIRCUIT DIAGRAM:
 * ================
 * 
 *   ESP32-S3-DevKitC-1              Piezo Buzzer
 *   ┌─────────────────┐             ┌──────────┐
 *   │                 │             │          │
 *   │             21 ─┼─────────────┼─ (+)     │
 *   │                 │             │          │
 *   │            GND ─┼─────────────┼─ (-)     │
 *   │                 │             │          │
 *   └─────────────────┘             └──────────┘
 * 
 * COMPONENTS:
 * ===========
 * 1x ESP32-S3-DevKitC-1 v1.1 development board
 * 1x Piezo buzzer (3V-5V compatible)
 * 2x Male-to-male jumper wires
 * 1x Breadboard (optional, for organization)
 * 
 * Component Specifications:
 * - Piezo Buzzer: Operating voltage 3-24V, frequency range 100Hz-4KHz
 * - Recommended: Standard 12mm piezo buzzer with built-in oscillator
 * 
 * GPIO ASSIGNMENTS:
 * =================
 * GPIO21: PWM output to piezo buzzer positive terminal
 * GND:    Connected to piezo buzzer negative terminal
 * 
 * Note: GPIO21 is chosen because it's safe for general I/O on ESP32-S3
 * and doesn't conflict with internal flash/PSRAM communication.
 * 
 * TECHNICAL NOTES:
 * ================
 * 1. ESP32-S3 vs ESP32 DAC Difference:
 *    - Original ESP32: Has 2x 8-bit DAC channels (GPIO25, GPIO26)
 *    - ESP32-S3: No DAC - uses PWM via LEDC peripheral instead
 * 
 * 2. PWM Implementation:
 *    - LEDC peripheral provides 8 independent PWM channels
 *    - Using 12-bit resolution (0-4095 range) for smooth waveforms
 *    - Base frequency: 1000Hz for good audio reproduction
 * 
 * 3. Waveform Generation:
 *    - Triangle: Linear ramp up/down for harmonic-rich sound
 *    - Sine: Pure tone with fundamental frequency
 *    - Square: Rich harmonic content via Fourier series approximation
 * 
 * 4. Performance Considerations:
 *    - Floating-point math for sine calculations
 *    - Timing delays for audible waveform periods
 *    - Memory usage: ~2KB for waveform calculations
 * 
 * IDE REQUIREMENTS:
 * =================
 * - Arduino IDE 2.0+
 * - ESP32 Arduino Core v3.0+
 * - Board: "ESP32S3 Dev Module"
 * - USB CDC On Boot: "Enabled" (for Serial communication)
 * - Flash Size: "16MB (128Mb)"
 * - PSRAM: "OPI PSRAM"
 * 
 * SAFETY CONSIDERATIONS:
 * ======================
 * - Piezo buzzers can be loud - test at low volumes first
 * - Check buzzer polarity if it has marked terminals
 * - Avoid continuous operation at high duty cycles to prevent heating
 * 
 * LEARNING OBJECTIVES:
 * ====================
 * - Understand PWM vs DAC for analog signal simulation
 * - Learn LEDC peripheral configuration and usage
 * - Practice waveform mathematics and audio synthesis
 * - Implement error handling in embedded systems
 * - Use modern ESP32-S3 APIs effectively
 * 
 * TROUBLESHOOTING:
 * ================
 * - No sound: Check GPIO21 connection and buzzer polarity
 * - Compilation errors: Verify ESP32-S3 board selection and core version
 * - Distorted sound: Adjust PWM frequency or waveform amplitude
 * - Serial output issues: Enable "USB CDC On Boot" in board settings
 * 
 * REFERENCES:
 * ===========
 * - ESP32-S3 Technical Reference: https://www.espressif.com/sites/default/files/documentation/esp32-s3_technical_reference_manual_en.pdf
 * - ESP32-S3 Datasheet: https://www.espressif.com/sites/default/files/documentation/esp32-s3_datasheet_en.pdf
 * - Arduino ESP32 Core: https://github.com/espressif/arduino-esp32
 * 
 * Created: March 26 2019 by Peter Dalmaris (Original)
 * Updated: January 2025 for ESP32-S3 compatibility and modern practices
 */

// ================================================================================================
// CONFIGURATION CONSTANTS
// ================================================================================================

// Hardware Configuration
const uint8_t BUZZER_PIN = 21;           // GPIO21 for PWM output to buzzer
const uint32_t PWM_FREQUENCY = 1000;     // Base PWM frequency in Hz
const uint8_t PWM_RESOLUTION = 12;       // 12-bit resolution (0-4095)
const uint32_t PWM_MAX_VALUE = 4095;     // Maximum PWM value for 12-bit

// Waveform Configuration
const uint16_t TRIANGLE_STEP = 20;       // Step size for triangle wave (faster = higher pitch)
const uint16_t TRIANGLE_DELAY = 1;       // Delay between steps in milliseconds
const uint8_t SINE_HARMONIC_COUNT = 6;   // Number of harmonics for complex waveforms
const float WAVEFORM_AMPLITUDE = 0.8;    // Amplitude scaling (0.0-1.0) for volume control

// Debug and Performance
const uint32_t SERIAL_BAUD_RATE = 115200;
const uint32_t CYCLE_COUNT_DISPLAY = 100; // Display performance stats every N cycles

// ================================================================================================
// GLOBAL VARIABLES
// ================================================================================================

// Performance monitoring
uint32_t cycle_counter = 0;
uint32_t start_time = 0;
bool initialization_complete = false;

// Waveform selection (can be modified during runtime)
enum WaveformType {
    TRIANGLE_WAVE,
    SINE_WAVE,
    SQUARE_WAVE,
    TRIANGLE_FOURIER_WAVE
};

WaveformType current_waveform = TRIANGLE_WAVE;

// ================================================================================================
// UTILITY FUNCTIONS
// ================================================================================================

/**
 * @brief Print a separator line for better console output formatting
 * Educational note: This demonstrates good programming practice by creating
 * reusable utility functions instead of repeating code.
 * @param length Number of characters to print
 * @param character Character to use for the line
 */
void printSeparator(uint8_t length = 60, char character = '=') {
    for (uint8_t i = 0; i < length; i++) {
        Serial.print(character);
    }
    Serial.println();
}

/**
 * @brief Initialize PWM output for buzzer control
 * @return true if initialization successful, false otherwise
 */
bool initializePWM() {
    Serial.println(F("Initializing PWM for buzzer control..."));
    
    // Configure LEDC PWM channel using modern ESP32-S3 API
    // ledcAttach(pin, frequency, resolution) - replaces ledcSetup + ledcAttachPin
    if (!ledcAttach(BUZZER_PIN, PWM_FREQUENCY, PWM_RESOLUTION)) {
        Serial.println(F("ERROR: Failed to attach PWM to buzzer pin!"));
        return false;
    }
    
    // Test PWM output with brief tone
    Serial.println(F("Testing PWM output..."));
    ledcWrite(BUZZER_PIN, PWM_MAX_VALUE / 2); // 50% duty cycle
    delay(100);
    ledcWrite(BUZZER_PIN, 0); // Silence
    
    Serial.printf("PWM Configuration:\n");
    Serial.printf("  Pin: GPIO%d\n", BUZZER_PIN);
    Serial.printf("  Frequency: %lu Hz\n", PWM_FREQUENCY);
    Serial.printf("  Resolution: %d bits (0-%lu)\n", PWM_RESOLUTION, PWM_MAX_VALUE);
    Serial.printf("  Amplitude Scaling: %.1f%%\n", WAVEFORM_AMPLITUDE * 100);
    
    return true;
}

/**
 * @brief Convert 8-bit DAC value (0-255) to 12-bit PWM value (0-4095)
 * @param dac_value Original 8-bit value from legacy code
 * @return Scaled 12-bit PWM value
 */
uint16_t convertDACToPWM(uint8_t dac_value) {
    // Scale from 8-bit (0-255) to 12-bit (0-4095) with amplitude control
    uint16_t pwm_value = (uint16_t)((dac_value * PWM_MAX_VALUE * WAVEFORM_AMPLITUDE) / 255.0);
    return constrain(pwm_value, 0, PWM_MAX_VALUE);
}

/**
 * @brief Generate triangle waveform using PWM
 * Educational note: Triangle waves contain odd harmonics and create a bright, buzzy sound
 */
void generateTriangleWave() {
    static bool ascending = true;
    static uint16_t triangle_value = 0;
    
    if (cycle_counter % CYCLE_COUNT_DISPLAY == 0) {
        Serial.printf("Triangle Wave - Value: %d, Direction: %s\n", 
                     triangle_value, ascending ? "Up" : "Down");
    }
    
    // Generate ascending portion of triangle
    if (ascending) {
        for (triangle_value = 0; triangle_value <= 255; triangle_value += TRIANGLE_STEP) {
            uint16_t pwm_output = convertDACToPWM(triangle_value);
            ledcWrite(BUZZER_PIN, pwm_output);
            delay(TRIANGLE_DELAY);
        }
        ascending = false;
    }
    // Generate descending portion of triangle
    else {
        for (triangle_value = 255; triangle_value > 0; triangle_value -= TRIANGLE_STEP) {
            uint16_t pwm_output = convertDACToPWM(triangle_value);
            ledcWrite(BUZZER_PIN, pwm_output);
            delay(TRIANGLE_DELAY);
        }
        ascending = true;
    }
}

/**
 * @brief Generate pure sine wave
 * Educational note: Sine waves produce pure tones with no harmonics
 */
void generateSineWave() {
    if (cycle_counter % CYCLE_COUNT_DISPLAY == 0) {
        Serial.println(F("Generating Pure Sine Wave (360° cycle)"));
    }
    
    for (int degree = 0; degree < 360; degree++) {
        // Convert degree to radians and calculate sine value
        float radian = degree * PI / 180.0;
        float sine_value = sin(radian);
        
        // Scale sine value (-1 to +1) to PWM range (0 to 255)
        uint8_t dac_equivalent = (uint8_t)(128 + 127 * sine_value * WAVEFORM_AMPLITUDE);
        uint16_t pwm_output = convertDACToPWM(dac_equivalent);
        
        ledcWrite(BUZZER_PIN, pwm_output);
        delay(2); // Adjust for desired frequency
    }
}

/**
 * @brief Generate square wave using Fourier series approximation
 * Educational note: Square waves contain all odd harmonics (1st, 3rd, 5th, etc.)
 * Formula: 4/π * (sin(x) + sin(3x)/3 + sin(5x)/5 + sin(7x)/7 + ...)
 */
void generateSquareWave() {
    if (cycle_counter % CYCLE_COUNT_DISPLAY == 0) {
        Serial.printf("Generating Square Wave (Fourier Series with %d harmonics)\n", 
                     SINE_HARMONIC_COUNT);
    }
    
    for (int degree = 0; degree < 360; degree++) {
        float radian = degree * PI / 180.0;
        float fourier_sum = 0.0;
        
        // Calculate Fourier series for square wave approximation
        for (int harmonic = 1; harmonic <= SINE_HARMONIC_COUNT; harmonic += 2) {
            fourier_sum += sin(harmonic * radian) / harmonic;
        }
        
        // Scale and convert to PWM value
        uint8_t dac_equivalent = (uint8_t)(128 + 80 * fourier_sum * WAVEFORM_AMPLITUDE);
        uint16_t pwm_output = convertDACToPWM(dac_equivalent);
        
        ledcWrite(BUZZER_PIN, pwm_output);
        delay(2);
    }
}

/**
 * @brief Generate triangle wave using Fourier series approximation
 * Educational note: Triangle waves contain odd harmonics with 1/n² amplitude
 * Formula: 8/π² * (sin(x) - sin(3x)/9 + sin(5x)/25 - sin(7x)/49 + ...)
 */
void generateTriangleFourierWave() {
    if (cycle_counter % CYCLE_COUNT_DISPLAY == 0) {
        Serial.printf("Generating Triangle Wave (Fourier Series with %d harmonics)\n", 
                     SINE_HARMONIC_COUNT);
    }
    
    for (int degree = 0; degree < 360; degree++) {
        float radian = degree * PI / 180.0;
        float fourier_sum = 0.0;
        
        // Calculate Fourier series for triangle wave
        for (int harmonic = 1; harmonic <= SINE_HARMONIC_COUNT; harmonic += 2) {
            float term = sin(harmonic * radian) / (harmonic * harmonic);
            fourier_sum += (harmonic % 4 == 1) ? term : -term; // Alternating signs
        }
        
        // Scale and convert to PWM value
        uint8_t dac_equivalent = (uint8_t)(128 + 80 * fourier_sum * WAVEFORM_AMPLITUDE);
        uint16_t pwm_output = convertDACToPWM(dac_equivalent);
        
        ledcWrite(BUZZER_PIN, pwm_output);
        delay(2);
    }
}

/**
 * @brief Display performance statistics
 */
void displayPerformanceStats() {
    uint32_t elapsed_time = millis() - start_time;
    float cycles_per_second = (float)cycle_counter / (elapsed_time / 1000.0);
    
    Serial.println();
    printSeparator(30, '=');
    Serial.println(F("Performance Statistics"));
    printSeparator(30, '-');
    Serial.printf("Uptime: %lu ms\n", elapsed_time);
    Serial.printf("Cycles completed: %lu\n", cycle_counter);
    Serial.printf("Cycles per second: %.2f\n", cycles_per_second);
    Serial.printf("Free heap: %lu bytes\n", ESP.getFreeHeap());
    Serial.printf("CPU frequency: %lu MHz\n", ESP.getCpuFreqMHz());
    printSeparator(30, '=');
    Serial.println();
}

// ================================================================================================
// MAIN PROGRAM FUNCTIONS
// ================================================================================================

void setup() {
    // Initialize serial communication
    Serial.begin(SERIAL_BAUD_RATE);
    delay(1000); // Allow time for serial connection
    
    // Display startup information
    Serial.println();
    printSeparator(60, '=');
    Serial.println(F("ESP32-S3 PWM Waveform Generator"));
    Serial.println(F("Course: IoT Development with ESP32-S3"));
    Serial.println(F("Lesson: 04.063 - PWM Audio Waveforms"));
    printSeparator(60, '=');
    
    // Display hardware information
    Serial.printf("Hardware: ESP32-S3-DevKitC-1 v1.1\n");
    Serial.printf("Module: ESP32-S3-WROOM-1-N16R16V\n");
    Serial.printf("Flash: %lu MB\n", ESP.getFlashChipSize() / (1024 * 1024));
    Serial.printf("PSRAM: %lu KB\n", ESP.getPsramSize() / 1024);
    Serial.printf("CPU Frequency: %lu MHz\n", ESP.getCpuFreqMHz());
    
    // Educational note about hardware differences
    Serial.println(F("\n*** IMPORTANT EDUCATIONAL NOTE ***"));
    Serial.println(F("ESP32-S3 does not have built-in DAC channels like the original ESP32."));
    Serial.println(F("This example uses PWM via the LEDC peripheral to simulate analog output."));
    Serial.println(F("PWM creates the same audio effects by varying the duty cycle rapidly."));
    Serial.println(F("*********************************\n"));
    
    // Initialize PWM
    if (!initializePWM()) {
        Serial.println(F("FATAL ERROR: PWM initialization failed!"));
        Serial.println(F("Check your hardware connections and board selection."));
        while (true) {
            delay(1000); // Halt execution
        }
    }
    
    // Initialize performance monitoring
    start_time = millis();
    cycle_counter = 0;
    initialization_complete = true;
    
    Serial.println(F("Initialization complete!"));
    Serial.println(F("Listen for waveform audio from the buzzer..."));
    Serial.println(F("Triangle wave will play continuously."));
    Serial.println(F("Uncomment other waveforms in loop() to try them.\n"));
    
    // Brief startup melody to confirm operation
    for (int i = 0; i < 3; i++) {
        ledcWrite(BUZZER_PIN, PWM_MAX_VALUE / 4);
        delay(100);
        ledcWrite(BUZZER_PIN, 0);
        delay(100);
    }
}

void loop() {
    if (!initialization_complete) {
        return; // Safety check
    }
    
    // Increment cycle counter for performance monitoring
    cycle_counter++;
    
    // Display performance statistics periodically
    if (cycle_counter % CYCLE_COUNT_DISPLAY == 0) {
        displayPerformanceStats();
    }
    
    // =================================================================
    // WAVEFORM GENERATION SECTION
    // =================================================================
    // Uncomment ONE of the following waveform functions to hear it:
    
    // Default: Triangle wave (simple ramp up/down)
    generateTriangleWave();
    
    // Alternative waveforms (uncomment one at a time):
    // generateSineWave();              // Pure sine wave tone
    // generateSquareWave();            // Square wave via Fourier series
    // generateTriangleFourierWave();   // Triangle wave via Fourier series
    
    // =================================================================
    // EDUCATIONAL EXPERIMENTS:
    // =================================================================
    // Try modifying these parameters to experiment:
    // 1. Change TRIANGLE_STEP (line 57) for different pitches
    // 2. Modify WAVEFORM_AMPLITUDE (line 62) for volume control
    // 3. Adjust delay values in waveform functions for speed changes
    // 4. Experiment with PWM_FREQUENCY for different tonal qualities
    
    // Small delay to prevent overwhelming the system
    delay(10);
    
    // Safety: Reset buzzer every 1000 cycles to prevent overheating
    if (cycle_counter % 1000 == 0) {
        ledcWrite(BUZZER_PIN, 0);
        delay(50);
    }
}
