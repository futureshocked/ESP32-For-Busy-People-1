/*  06.021 - Single Seven Segment Display Control for ESP32-S3
 * 
 * Course: IoT Development with ESP32-S3
 * Target Hardware: ESP32-S3-DevKitC-1 v1.1
 * Module: ESP32-S3-WROOM-1-N16R16V
 * 
 * This comprehensive sketch demonstrates how to control a single common cathode
 * seven-segment display using the ESP32-S3's digital GPIO pins. The program
 * cycles through displaying digits 0-9 and a decimal point, with detailed
 * educational features including error handling, performance monitoring,
 * and multiple display modes for learning purposes.
 * 
 * LEARNING OBJECTIVES:
 * ===================
 * - Understand seven-segment display operation and segment mapping
 * - Learn binary pattern manipulation using bitwise operations
 * - Practice GPIO pin control and digital output management
 * - Implement modular programming with helper functions
 * - Apply error handling and input validation techniques
 * - Explore timing control and display refresh concepts
 * 
 * HARDWARE INFORMATION:
 * =====================
 * ESP32-S3-DevKitC-1 v1.1 Specifications:
 * - Dual-core Xtensa LX7 processor (240 MHz)
 * - 16 MB Flash Memory, 16 MB PSRAM
 * - 45 programmable GPIO pins
 * - Native USB support (GPIO19/20)
 * - WS2812 RGB LED on GPIO38
 * 
 * Seven-Segment Display:
 * - Common cathode configuration
 * - 8 segments: A, B, C, D, E, F, G, Dp (decimal point)
 * - Forward voltage: ~2.0V per segment
 * - Forward current: ~20mA per segment
 * 
 * CIRCUIT DIAGRAM:
 * ================
 *                    ESP32-S3-DevKitC-1
 *                  ┌─────────────────────┐
 *                  │                     │
 *             3.3V │3V3               1 │─── GPIO1  ───[220Ω]─── A segment
 *                  │                     │
 *              GND │GND               2 │─── GPIO2  ───[220Ω]─── B segment
 *                  │                     │
 *                  │                19  │─── GPIO19 ───[220Ω]─── C segment
 *                  │                     │
 *                  │                18  │─── GPIO18 ───[220Ω]─── D segment
 *                  │                     │
 *                  │                 5  │─── GPIO5  ───[220Ω]─── E segment
 *                  │                     │
 *                  │                 6  │─── GPIO6  ───[220Ω]─── F segment
 *                  │                     │
 *                  │                14  │─── GPIO14 ───[220Ω]─── G segment
 *                  │                     │
 *                  │                 4  │─── GPIO4  ───[220Ω]─── Dp segment
 *                  │                     │
 *                  └─────────────────────┘
 *                              │
 *                            GND ────────────────────── Common Cathode
 * 
 * Seven-Segment Display Layout:
 *                 A
 *               ┌───┐
 *            F  │   │  B
 *               │ G │
 *               ├───┤
 *            E  │   │  C
 *               │   │
 *               └───┘  ● Dp
 *                 D
 * 
 * COMPONENTS:
 * ===========
 * - 1x ESP32-S3-DevKitC-1 v1.1 development board
 * - 1x Common cathode seven-segment display (e.g., 5161AS)
 * - 8x 220Ω resistors (current limiting for LED segments)
 * - 1x Breadboard (minimum 830 tie points recommended)
 * - 10x Male-to-male jumper wires
 * - 1x USB-C cable for programming and power
 * 
 * GPIO ASSIGNMENTS:
 * =================
 * ┌─────────────┬─────────────┬─────────────┬─────────────┐
 * │ Segment     │ ESP32-S3    │ Resistor    │ Display Pin │
 * │ Name        │ GPIO        │ Value       │ Connection  │
 * ├─────────────┼─────────────┼─────────────┼─────────────┤
 * │ A (top)     │ GPIO1       │ 220Ω        │ Pin 7       │
 * │ B (top-R)   │ GPIO2       │ 220Ω        │ Pin 6       │
 * │ C (bot-R)   │ GPIO19      │ 220Ω        │ Pin 4       │
 * │ D (bottom)  │ GPIO18      │ 220Ω        │ Pin 2       │
 * │ E (bot-L)   │ GPIO5       │ 220Ω        │ Pin 1       │
 * │ F (top-L)   │ GPIO6       │ 220Ω        │ Pin 9       │
 * │ G (middle)  │ GPIO14      │ 220Ω        │ Pin 10      │
 * │ Dp (point)  │ GPIO4       │ 220Ω        │ Pin 5       │
 * │ Common      │ GND         │ Direct      │ Pins 3,8    │
 * └─────────────┴─────────────┴─────────────┴─────────────┘
 * 
 * TECHNICAL NOTES:
 * ================
 * - Updated pin assignments for ESP32-S3 compatibility
 * - Original GPIO33, GPIO32, GPIO27 not available on ESP32-S3
 * - GPIO19 is USB D- but can be used for general I/O when USB not needed
 * - Current limiting resistors prevent LED damage and excessive current draw
 * - Common cathode means segments light up when GPIO outputs HIGH (3.3V)
 * - Bit patterns are ordered as .GFEDCBA (bit 7 to bit 0)
 * 
 * IDE REQUIREMENTS:
 * =================
 * - Arduino IDE 2.0+
 * - ESP32 Arduino Core v3.0+
 * - Board: "ESP32S3 Dev Module"
 * - USB CDC On Boot: "Enabled" (for Serial output)
 * - Flash Size: "16MB"
 * - PSRAM: "OPI PSRAM"
 * 
 * Created: April 1 2019 by Peter Dalmaris
 * Updated: December 2024 for ESP32-S3 compatibility and educational enhancement
 */

#include <Arduino.h>

// ====================================================================
// CONFIGURATION CONSTANTS
// ====================================================================

// Display configuration
constexpr uint8_t CHAR_COUNT = 11;              // Number of displayable symbols
constexpr uint16_t DISPLAY_DELAY_MS = 1000;     // Delay between character changes
constexpr uint16_t SERIAL_BAUD_RATE = 115200;   // Serial communication speed

// Timing and performance constants
constexpr uint32_t STATUS_UPDATE_INTERVAL_MS = 5000;  // Status report interval
constexpr uint16_t SEGMENT_TEST_DELAY_MS = 200;       // Individual segment test delay

// ====================================================================
// GPIO PIN ASSIGNMENTS
// ====================================================================

// Seven-segment display pin mapping (updated for ESP32-S3)
// Pin order: A, B, C, D, E, F, G, Dp
constexpr uint8_t SEGMENT_PINS[8] = {
    1,   // A segment  (top horizontal)
    2,   // B segment  (top right vertical)
    19,  // C segment  (bottom right vertical)
    18,  // D segment  (bottom horizontal)
    5,   // E segment  (bottom left vertical)
    6,   // F segment  (top left vertical)
    14,  // G segment  (middle horizontal)
    4    // Dp segment (decimal point)
};

// Segment names for debugging and educational purposes
const char* SEGMENT_NAMES[8] = {
    "A(top)", "B(top-right)", "C(bot-right)", "D(bottom)",
    "E(bot-left)", "F(top-left)", "G(middle)", "Dp(point)"
};

// ====================================================================
// DISPLAY PATTERNS
// ====================================================================

// Binary patterns for digits 0-9 and decimal point
// Bit order: .GFEDCBA (bit 7 = Dp, bit 0 = A)
constexpr uint8_t DIGIT_PATTERNS[CHAR_COUNT] = {
    0b00111111,  // 0: A,B,C,D,E,F
    0b00000110,  // 1: B,C
    0b01011011,  // 2: A,B,G,E,D
    0b01001111,  // 3: A,B,G,C,D
    0b01100110,  // 4: F,G,B,C
    0b01101101,  // 5: A,F,G,C,D
    0b01111101,  // 6: A,F,G,E,D,C
    0b00000111,  // 7: A,B,C
    0b01111111,  // 8: All segments
    0b01101111,  // 9: A,B,C,D,F,G
    0b10000000   // .: Decimal point only
};

// Character representations for serial output
const char DISPLAY_CHARS[CHAR_COUNT] = {
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '.'
};

// ====================================================================
// GLOBAL VARIABLES
// ====================================================================

uint32_t lastStatusUpdate = 0;    // Last status report timestamp
uint32_t displayCycles = 0;       // Count of complete display cycles
uint32_t segmentWrites = 0;       // Count of segment write operations
bool verboseMode = true;          // Enable detailed serial output

// ====================================================================
// SETUP FUNCTION
// ====================================================================

void setup() {
    // Initialize serial communication
    Serial.begin(SERIAL_BAUD_RATE);
    
    // Wait for serial connection (optional - remove for standalone operation)
    while (!Serial && millis() < 3000) {
        delay(10);
    }
    
    // Display startup information
    printStartupInfo();
    
    // Initialize GPIO pins
    if (initializeSegmentPins()) {
        Serial.println("✓ GPIO pins initialized successfully");
    } else {
        Serial.println("✗ GPIO initialization failed");
        return;
    }
    
    // Perform hardware test
    if (verboseMode) {
        performSegmentTest();
    }
    
    // Clear display
    clearDisplay();
    
    Serial.println("Setup complete. Starting main display loop...\n");
    
    // Record initial timestamp
    lastStatusUpdate = millis();
}

// ====================================================================
// MAIN LOOP
// ====================================================================

void loop() {
    // Main display sequence
    for (uint8_t i = 0; i < CHAR_COUNT; i++) {
        displayCharacter(i);
        delay(DISPLAY_DELAY_MS);
        
        // Print status updates periodically
        if (millis() - lastStatusUpdate >= STATUS_UPDATE_INTERVAL_MS) {
            printStatusUpdate();
            lastStatusUpdate = millis();
        }
    }
    
    displayCycles++;
    
    // Optional: Add a longer pause between complete cycles
    if (displayCycles % 5 == 0) {
        Serial.println("--- Cycle break ---");
        clearDisplay();
        delay(500);
    }
}

// ====================================================================
// DISPLAY CONTROL FUNCTIONS
// ====================================================================

/**
 * Display a character by index from the pattern array
 * @param index Character index (0-10)
 */
void displayCharacter(uint8_t index) {
    if (index >= CHAR_COUNT) {
        Serial.printf("Error: Invalid character index %d (max: %d)\n", index, CHAR_COUNT - 1);
        return;
    }
    
    uint8_t pattern = DIGIT_PATTERNS[index];
    char character = DISPLAY_CHARS[index];
    
    if (verboseMode) {
        Serial.printf("Displaying '%c' (pattern: 0x%02X, binary: ", character, pattern);
        printBinaryPattern(pattern);
        Serial.println(")");
    }
    
    writeSegmentPattern(pattern);
}

/**
 * Write a bit pattern to the seven-segment display
 * @param pattern 8-bit pattern where each bit controls one segment
 */
void writeSegmentPattern(uint8_t pattern) {
    for (uint8_t i = 0; i < 8; i++) {
        bool segmentState = bitRead(pattern, i);
        digitalWrite(SEGMENT_PINS[i], segmentState ? HIGH : LOW);
        
        if (verboseMode && segmentState) {
            Serial.printf("  → Segment %s ON\n", SEGMENT_NAMES[i]);
        }
    }
    segmentWrites++;
}

/**
 * Clear the display (turn off all segments)
 */
void clearDisplay() {
    writeSegmentPattern(0x00);
    if (verboseMode) {
        Serial.println("Display cleared");
    }
}

/**
 * Test all segments individually
 */
void performSegmentTest() {
    Serial.println("Performing segment test...");
    
    for (uint8_t i = 0; i < 8; i++) {
        Serial.printf("Testing segment %s (GPIO%d)...\n", SEGMENT_NAMES[i], SEGMENT_PINS[i]);
        
        // Turn on only this segment
        clearDisplay();
        digitalWrite(SEGMENT_PINS[i], HIGH);
        delay(SEGMENT_TEST_DELAY_MS);
    }
    
    clearDisplay();
    Serial.println("Segment test complete\n");
}

// ====================================================================
// INITIALIZATION FUNCTIONS
// ====================================================================

/**
 * Initialize all segment control pins as outputs
 * @return true if successful, false if any pin initialization fails
 */
bool initializeSegmentPins() {
    Serial.println("Initializing GPIO pins...");
    
    for (uint8_t i = 0; i < 8; i++) {
        uint8_t pin = SEGMENT_PINS[i];
        
        // Validate pin number
        if (!isValidGPIO(pin)) {
            Serial.printf("Error: Invalid GPIO pin %d for segment %s\n", pin, SEGMENT_NAMES[i]);
            return false;
        }
        
        // Set pin as output
        pinMode(pin, OUTPUT);
        digitalWrite(pin, LOW);  // Start with segment off
        
        Serial.printf("  GPIO%d → %s segment\n", pin, SEGMENT_NAMES[i]);
    }
    
    return true;
}

/**
 * Validate if a GPIO pin number is valid for ESP32-S3
 * @param pin GPIO pin number to validate
 * @return true if valid, false if invalid
 */
bool isValidGPIO(uint8_t pin) {
    // ESP32-S3 has GPIO 0-48, but some are restricted
    if (pin > 48) return false;
    
    // Check for restricted pins
    if (pin == 35 || pin == 36 || pin == 37) {
        // These are used for internal SPI flash/PSRAM
        return false;
    }
    
    return true;
}

// ====================================================================
// UTILITY AND DEBUG FUNCTIONS
// ====================================================================

/**
 * Print startup information and hardware details
 */
void printStartupInfo() {
    Serial.println("\n" + String('=', 60));
    Serial.println("ESP32-S3 Seven Segment Display Controller");
    Serial.println("Course: IoT Development with ESP32-S3");
    Serial.println("Lesson: 06.021 - Single Seven Segment Display");
    Serial.println(String('=', 60));
    
    // Hardware information
    Serial.printf("Target Board: ESP32-S3-DevKitC-1 v1.1\n");
    Serial.printf("Module: ESP32-S3-WROOM-1-N16R16V\n");
    Serial.printf("Flash: 16MB, PSRAM: 16MB\n");
    Serial.printf("CPU Frequency: %d MHz\n", getCpuFrequencyMhz());
    Serial.printf("Free Heap: %d bytes\n", ESP.getFreeHeap());
    Serial.printf("Chip Revision: %d\n", ESP.getChipRevision());
    
    // Display configuration
    Serial.println("\nDisplay Configuration:");
    Serial.printf("Characters: %d\n", CHAR_COUNT);
    Serial.printf("Display delay: %d ms\n", DISPLAY_DELAY_MS);
    Serial.printf("Status interval: %d ms\n", STATUS_UPDATE_INTERVAL_MS);
    
    // GPIO assignments
    Serial.println("\nGPIO Pin Assignments:");
    for (uint8_t i = 0; i < 8; i++) {
        Serial.printf("  GPIO%2d → %s\n", SEGMENT_PINS[i], SEGMENT_NAMES[i]);
    }
    
    Serial.println(String('-', 60));
}

/**
 * Print periodic status updates
 */
void printStatusUpdate() {
    uint32_t uptime = millis();
    uint32_t uptimeSeconds = uptime / 1000;
    
    Serial.println("\n" + String('-', 40));
    Serial.println("STATUS UPDATE");
    Serial.println(String('-', 40));
    Serial.printf("Uptime: %02d:%02d:%02d (%lu ms)\n", 
                  uptimeSeconds / 3600, 
                  (uptimeSeconds % 3600) / 60, 
                  uptimeSeconds % 60, 
                  uptime);
    Serial.printf("Display cycles completed: %lu\n", displayCycles);
    Serial.printf("Segment writes: %lu\n", segmentWrites);
    Serial.printf("Free heap: %d bytes\n", ESP.getFreeHeap());
    Serial.printf("Average writes per cycle: %.2f\n", 
                  displayCycles > 0 ? (float)segmentWrites / displayCycles : 0);
    Serial.println(String('-', 40) + "\n");
}

/**
 * Print binary representation of a pattern
 * @param pattern 8-bit pattern to display
 */
void printBinaryPattern(uint8_t pattern) {
    for (int8_t i = 7; i >= 0; i--) {
        Serial.print(bitRead(pattern, i) ? '1' : '0');
        if (i == 4) Serial.print(' ');  // Add space between nibbles
    }
}

/**
 * Display the current segment mapping table
 */
void printSegmentMapping() {
    Serial.println("\nSegment Mapping Reference:");
    Serial.println("┌─────────┬─────────┬─────────────┐");
    Serial.println("│ Segment │ GPIO    │ Function    │");
    Serial.println("├─────────┼─────────┼─────────────┤");
    
    for (uint8_t i = 0; i < 8; i++) {
        Serial.printf("│ %7s │ GPIO%-2d │ %-11s │\n", 
                      SEGMENT_NAMES[i], 
                      SEGMENT_PINS[i], 
                      i < 7 ? "Display" : "Dec. Point");
    }
    
    Serial.println("└─────────┴─────────┴─────────────┘");
}

// ====================================================================
// EDUCATIONAL DEMONSTRATION FUNCTIONS
// ====================================================================

/**
 * Demonstrate all possible segment combinations
 * Call this function from setup() for educational purposes
 */
void demonstrateAllPatterns() {
    Serial.println("Demonstrating all digit patterns...");
    
    for (uint8_t i = 0; i < CHAR_COUNT; i++) {
        Serial.printf("Pattern %d ('%c'): ", i, DISPLAY_CHARS[i]);
        printBinaryPattern(DIGIT_PATTERNS[i]);
        Serial.printf(" = 0x%02X\n", DIGIT_PATTERNS[i]);
        
        writeSegmentPattern(DIGIT_PATTERNS[i]);
        delay(800);
    }
    
    clearDisplay();
}

/**
 * Count in binary on the display (educational)
 */
void binaryCountDemo() {
    Serial.println("Binary counting demonstration (0-255)...");
    
    for (uint16_t i = 0; i <= 255; i++) {
        Serial.printf("Binary count: %3d (0x%02X) = ", i, i);
        printBinaryPattern(i);
        Serial.println();
        
        writeSegmentPattern(i);
        delay(300);
    }
    
    clearDisplay();
}

// ====================================================================
// ERROR HANDLING AND DIAGNOSTICS
// ====================================================================

/**
 * Perform comprehensive hardware diagnostics
 * @return true if all tests pass
 */
bool performDiagnostics() {
    Serial.println("Performing hardware diagnostics...");
    
    bool allTestsPassed = true;
    
    // Test 1: GPIO pin validation
    for (uint8_t i = 0; i < 8; i++) {
        if (!isValidGPIO(SEGMENT_PINS[i])) {
            Serial.printf("FAIL: Invalid GPIO%d for segment %s\n", 
                          SEGMENT_PINS[i], SEGMENT_NAMES[i]);
            allTestsPassed = false;
        }
    }
    
    // Test 2: Pin conflict check
    for (uint8_t i = 0; i < 8; i++) {
        for (uint8_t j = i + 1; j < 8; j++) {
            if (SEGMENT_PINS[i] == SEGMENT_PINS[j]) {
                Serial.printf("FAIL: GPIO conflict - GPIO%d used for both %s and %s\n",
                              SEGMENT_PINS[i], SEGMENT_NAMES[i], SEGMENT_NAMES[j]);
                allTestsPassed = false;
            }
        }
    }
    
    // Test 3: Memory check
    if (ESP.getFreeHeap() < 1000) {
        Serial.println("WARN: Low memory condition detected");
    }
    
    if (allTestsPassed) {
        Serial.println("✓ All diagnostics passed");
    } else {
        Serial.println("✗ Some diagnostics failed");
    }
    
    return allTestsPassed;
}
