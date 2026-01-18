/*  04.071 - GPIO Interrupt with Button and LED for ESP32-S3
 * 
 * Course: IoT Development with ESP32-S3
 * Target Hardware: ESP32-S3-DevKitC-1 v1.1
 * Module: ESP32-S3-WROOM-1-N16R16V
 * 
 * This advanced interrupt handling example demonstrates professional embedded
 * systems programming techniques for ESP32-S3. Students will learn GPIO 
 * interrupt handling, software debouncing, critical sections, and modern
 * ESP32-S3 peripheral management.
 * 
 * LEARNING OBJECTIVES:
 * ===================
 * - Master ESP32-S3 GPIO interrupt configuration and handling
 * - Implement robust software debouncing techniques
 * - Understand critical sections and thread safety in embedded systems
 * - Apply modern Arduino Core 3.0+ APIs for ESP32-S3
 * - Practice professional code documentation and error handling
 * - Learn ESP32-S3 specific hardware considerations and limitations
 * 
 * HARDWARE INFORMATION:
 * =====================
 * Board: ESP32-S3-DevKitC-1 v1.1
 * SoC: ESP32-S3 (Xtensa dual-core 32-bit LX7, up to 240 MHz)
 * Flash: 16 MB QSPI Flash
 * PSRAM: 16 MB Octal PSRAM  
 * ADC: 12-bit resolution (0-4095), ADC1 & ADC2 available
 * PWM: LEDC peripheral (16 channels, multiple timers)
 * USB: Native USB 1.1 Full Speed (GPIO19/20)
 * RGB LED: WS2812 on GPIO38 (v1.1 board)
 * 
 * CIRCUIT DIAGRAM:
 * ================
 *                   ESP32-S3-DevKitC-1 v1.1
 *                          +--------+
 *                    3V3   |        |   GND
 *                          |        |
 *               LED(+)--[R]|  GPIO2 |   
 *               LED(-)-----|  GND   |
 *                          |        |
 *            BUTTON(1)-----|  GPIO0 |   
 *            BUTTON(2)-----|  GND   |
 *                          |        |
 *              (Optional)   | GPIO38 |   RGB LED (WS2812)
 *                          +--------+
 * 
 *     External LED Circuit:
 *     GPIO2 ----[330Ohm]----(+)LED(-)---- GND
 * 
 *     Button Circuit (with internal pull-up):
 *     GPIO0 ----BUTTON---- GND
 *     (Internal pull-up resistor enabled in software)
 * 
 * COMPONENTS:
 * ===========
 * Required:
 * - ESP32-S3-DevKitC-1 v1.1 development board
 * - LED (any color, 3mm or 5mm)
 * - 330 Ohm resistor (orange-orange-brown-gold)
 * - Tactile push button (normally open)
 * - Breadboard and jumper wires
 * 
 * Optional:
 * - Oscilloscope for signal analysis
 * - Logic analyzer for debugging
 * 
 * GPIO ASSIGNMENTS:
 * =================
 * GPIO2  - External LED output (safe for general I/O)
 * GPIO0  - Button input with internal pull-up (BOOT button compatible)
 * GPIO38 - Onboard RGB LED (WS2812) - mentioned for reference
 * 
 * RESERVED PINS (DO NOT USE):
 * GPIO35, GPIO36, GPIO37 - Reserved for internal SPI flash/PSRAM communication
 * 
 * TECHNICAL NOTES:
 * ================
 * 1. Interrupt Service Routines (ISRs) must be placed in IRAM for fast access
 * 2. Software debouncing prevents multiple triggers from mechanical switch bounce
 * 3. Critical sections ensure atomic operations in multi-core environment
 * 4. ESP32-S3 supports both level and edge triggered interrupts
 * 5. GPIO0 can be used for general I/O when not in download mode
 * 6. Modern ESP32 Arduino Core uses improved interrupt allocation
 * 
 * SAFETY CONSIDERATIONS:
 * ======================
 * - Always use current-limiting resistors with LEDs
 * - Verify GPIO pin capabilities before assignment
 * - Avoid blocking operations in ISRs
 * - Test debouncing timing with actual hardware
 * 
 * IDE REQUIREMENTS:
 * =================
 * - Arduino IDE 2.0+
 * - ESP32 Arduino Core v3.0+
 * - Board: "ESP32S3 Dev Module"
 * - Partition Scheme: "Default 4MB with spiffs"
 * - PSRAM: "OPI PSRAM"
 * 
 * Created: March 26 2019 by Peter Dalmaris (Original ESP32 version)
 * Updated: July 25 2025 for ESP32-S3 compatibility and modern practices
 */

// ====================================================================
// CONFIGURATION CONSTANTS
// ====================================================================

// GPIO Pin Assignments for ESP32-S3-DevKitC-1 v1.1
constexpr gpio_num_t LED_GPIO = GPIO_NUM_2;        // External LED output
constexpr gpio_num_t BUTTON_GPIO = GPIO_NUM_0;     // Button input (BOOT button compatible)
constexpr gpio_num_t RGB_LED_GPIO = GPIO_NUM_38;   // Onboard RGB LED (reference only)

// Interrupt and Debouncing Configuration
constexpr uint32_t DEBOUNCE_TIME_MS = 50;          // Debouncing time in milliseconds
constexpr uint32_t SERIAL_BAUD_RATE = 115200;      // Serial communication speed
constexpr uint32_t STATUS_UPDATE_INTERVAL_MS = 5000; // Status update frequency

// Debug and Educational Features
constexpr bool ENABLE_DETAILED_LOGGING = true;     // Enable verbose debug output
constexpr bool ENABLE_PERFORMANCE_MONITORING = true; // Monitor ISR performance

// ====================================================================
// GLOBAL VARIABLES AND STATE MANAGEMENT
// ====================================================================

// LED State Management
volatile bool led_state = false;                   // Current LED state (volatile for ISR access)

// Interrupt Handling Variables
volatile uint32_t interrupt_counter = 0;           // Pending interrupt count
volatile uint32_t total_interrupts = 0;            // Total interrupts processed
volatile uint32_t last_interrupt_time = 0;         // Last interrupt timestamp (microseconds)

// Performance Monitoring (Educational)
volatile uint32_t isr_execution_count = 0;         // ISR execution counter
volatile uint32_t debounced_interrupts = 0;        // Successfully debounced interrupts
volatile uint32_t rejected_interrupts = 0;         // Rejected due to debouncing

// Critical Section Protection
portMUX_TYPE interrupt_mutex = portMUX_INITIALIZER_UNLOCKED;

// Status Display Timer
unsigned long last_status_update = 0;

// ====================================================================
// INTERRUPT SERVICE ROUTINE
// ====================================================================

/**
 * @brief GPIO Interrupt Service Routine with software debouncing
 * 
 * This ISR is called on falling edge of the button press. It implements
 * software debouncing to prevent multiple triggers from mechanical switch
 * bounce. The ISR must be fast and minimal - no Serial.print() or delay()!
 * 
 * Technical Notes:
 * - IRAM_ATTR ensures ISR code is stored in internal RAM for fast access
 * - portENTER_CRITICAL_ISR/portEXIT_CRITICAL_ISR ensure atomic operations
 * - Only essential operations should be performed in ISR
 * - Actual LED toggle is deferred to main loop for better responsiveness
 */
void IRAM_ATTR handleButtonInterrupt() {
    // Enter critical section to prevent race conditions
    portENTER_CRITICAL_ISR(&interrupt_mutex);
    
    // Get current timestamp for debouncing calculation
    uint32_t current_time = micros();
    
    // Software debouncing: reject interrupts that occur too quickly
    if ((current_time - last_interrupt_time) >= (DEBOUNCE_TIME_MS * 1000)) {
        // Valid interrupt - increment counter for main loop processing
        interrupt_counter++;
        debounced_interrupts++;
        last_interrupt_time = current_time;
    } else {
        // Rejected due to debouncing
        rejected_interrupts++;
    }
    
    // Track ISR performance for educational purposes
    isr_execution_count++;
    
    // Exit critical section
    portEXIT_CRITICAL_ISR(&interrupt_mutex);
}

// ====================================================================
// UTILITY FUNCTIONS
// ====================================================================

/**
 * @brief Initialize GPIO pins with proper configuration
 * 
 * Configures GPIO pins for LED output and button input with internal pull-up.
 * Includes error checking and status reporting for educational purposes.
 */
void initializeGPIO() {
    Serial.println("Initializing GPIO pins...");
    
    // Configure LED GPIO as output
    pinMode(LED_GPIO, OUTPUT);
    digitalWrite(LED_GPIO, LOW);  // Ensure LED starts in OFF state
    Serial.printf("   GPIO%d configured as LED output\n", LED_GPIO);
    
    // Configure button GPIO with internal pull-up resistor
    pinMode(BUTTON_GPIO, INPUT_PULLUP);
    Serial.printf("   GPIO%d configured as button input with pull-up\n", BUTTON_GPIO);
    
    // Verify initial button state
    bool initial_button_state = digitalRead(BUTTON_GPIO);
    Serial.printf("   Initial button state: %s\n", 
                  initial_button_state ? "RELEASED (HIGH)" : "PRESSED (LOW)");
}

/**
 * @brief Configure and attach GPIO interrupt
 * 
 * Sets up the interrupt on falling edge (button press) and attaches
 * the ISR function. attachInterrupt() returns void in ESP32 Arduino Core.
 */
void setupInterrupt() {
    Serial.println("Configuring GPIO interrupt...");
    
    // Attach interrupt on falling edge (button press)
    // FALLING: triggered when pin goes from HIGH to LOW
    // Note: attachInterrupt() returns void in ESP32 Arduino Core
    attachInterrupt(digitalPinToInterrupt(BUTTON_GPIO), 
                   handleButtonInterrupt, 
                   FALLING);
    
    Serial.printf("   Interrupt attached to GPIO%d (falling edge)\n", BUTTON_GPIO);
    Serial.printf("   Debounce time: %d ms\n", DEBOUNCE_TIME_MS);
}

/**
 * @brief Toggle LED state and provide visual feedback
 * 
 * Updates LED state and provides detailed logging for educational purposes.
 * This function is called from the main loop, not from the ISR.
 */
void toggleLED() {
    // Toggle LED state
    led_state = !led_state;
    digitalWrite(LED_GPIO, led_state);
    
    // Provide detailed feedback for learning
    Serial.printf("LED %s (GPIO%d = %s)\n", 
                  led_state ? "ON" : "OFF",
                  LED_GPIO,
                  led_state ? "HIGH" : "LOW");
}

/**
 * @brief Display detailed system status for educational purposes
 * 
 * Provides comprehensive status information including interrupt statistics,
 * performance metrics, and hardware state for learning and debugging.
 */
void displaySystemStatus() {
    Serial.println("\n=== SYSTEM STATUS REPORT ===");
    Serial.printf("Total Interrupts Processed: %d\n", total_interrupts);
    Serial.printf("Successfully Debounced: %d\n", debounced_interrupts);
    Serial.printf("Rejected (Debounced): %d\n", rejected_interrupts);
    Serial.printf("ISR Executions: %d\n", isr_execution_count);
    
    if (isr_execution_count > 0) {
        float debounce_effectiveness = (float)debounced_interrupts / isr_execution_count * 100.0;
        Serial.printf("Debounce Effectiveness: %.1f%%\n", debounce_effectiveness);
    }
    
    Serial.printf("Current LED State: %s\n", led_state ? "ON" : "OFF");
    Serial.printf("Current Button State: %s\n", 
                  digitalRead(BUTTON_GPIO) ? "RELEASED" : "PRESSED");
    Serial.printf("Free Heap: %d bytes\n", ESP.getFreeHeap());
    Serial.printf("Uptime: %.1f seconds\n", millis() / 1000.0);
    Serial.println("================================\n");
}

/**
 * @brief Process pending interrupts from main loop
 * 
 * Safely processes interrupt events outside of ISR context, allowing
 * for more complex operations like serial output and LED control.
 */
void processInterrupts() {
    // Check if there are pending interrupts to process
    if (interrupt_counter > 0) {
        // Enter critical section to safely read/modify interrupt_counter
        portENTER_CRITICAL(&interrupt_mutex);
        interrupt_counter--;  // Decrement pending interrupt count
        total_interrupts++;   // Increment total processed count
        portEXIT_CRITICAL(&interrupt_mutex);
        
        // Perform LED toggle and logging (safe to do outside ISR)
        toggleLED();
        
        if (ENABLE_DETAILED_LOGGING) {
            uint32_t current_time = millis();
            Serial.printf("Interrupt #%d processed at %d ms\n", 
                          total_interrupts, current_time);
        }
    }
}

// ====================================================================
// MAIN PROGRAM FUNCTIONS
// ====================================================================

/**
 * @brief Arduino setup function - initialization and configuration
 * 
 * Performs all necessary initialization including serial communication,
 * GPIO configuration, interrupt setup, and welcome message display.
 */
void setup() {
    // Initialize serial communication
    Serial.begin(SERIAL_BAUD_RATE);
    delay(2000);  // Allow time for serial monitor to connect
    
    // Display welcome message and system information
    Serial.println("\n============================================================");
    Serial.println("ESP32-S3 GPIO Interrupt Example - Lesson 04.071");
    Serial.println("Course: IoT Development with ESP32-S3");
    Serial.println("Hardware: ESP32-S3-DevKitC-1 v1.1");
    Serial.println("============================================================");
    
    // Display chip information for educational purposes
    Serial.printf("Chip Model: %s\n", ESP.getChipModel());
    Serial.printf("CPU Frequency: %d MHz\n", ESP.getCpuFreqMHz());
    Serial.printf("Flash Size: %d bytes\n", ESP.getFlashChipSize());
    Serial.printf("Free Heap: %d bytes\n", ESP.getFreeHeap());
    Serial.printf("Arduino Core Version: %s\n", ESP.getSdkVersion());
    Serial.println();
    
    // Initialize hardware components
    initializeGPIO();
    setupInterrupt();
    
    // Initialize timing variables
    last_status_update = millis();
    
    // Display usage instructions
    Serial.println("INSTRUCTIONS:");
    Serial.println("   * Press the button connected to GPIO0 to toggle the LED");
    Serial.printf("   * LED is connected to GPIO%d with 330 Ohm resistor\n", LED_GPIO);
    Serial.printf("   * Debounce time is set to %d ms\n", DEBOUNCE_TIME_MS);
    Serial.println("   * Watch for detailed interrupt logging below");
    Serial.println("\nReady! Press the button to see interrupt handling in action...\n");
}

/**
 * @brief Arduino main loop - continuous program execution
 * 
 * Handles interrupt processing, status updates, and system monitoring.
 * Demonstrates proper main loop structure for embedded systems.
 */
void loop() {
    // Process any pending interrupts
    processInterrupts();
    
    // Periodic status updates for educational monitoring
    unsigned long current_time = millis();
    if (current_time - last_status_update >= STATUS_UPDATE_INTERVAL_MS) {
        if (ENABLE_PERFORMANCE_MONITORING) {
            displaySystemStatus();
        }
        last_status_update = current_time;
    }
    
    // Small delay to prevent excessive CPU usage
    // In real applications, this might be replaced with deep sleep
    delay(10);
}
