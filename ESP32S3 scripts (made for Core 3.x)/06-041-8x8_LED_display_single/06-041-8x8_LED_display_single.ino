/*  06.041 - Single 8x8 LED Matrix Display with MAX72xx Driver for ESP32-S3
 * 
 * Course: IoT Development with ESP32-S3
 * Target Hardware: ESP32-S3-DevKitC-1 v1.1
 * Module: ESP32-S3-WROOM-1-N16R16V
 * 
 * This comprehensive sketch demonstrates how to interface with an 8x8 LED matrix
 * display using the MAX72xx driver IC via SPI communication. Students will learn
 * about SPI protocol, graphics primitives, pixel manipulation, and basic animation
 * techniques. The sketch includes multiple display patterns and educational features.
 * 
 * HARDWARE INFORMATION:
 * =====================
 * ESP32-S3-DevKitC-1 v1.1 Specifications:
 * - Dual-core Xtensa LX7 @ 240MHz
 * - 16MB Flash Memory
 * - 16MB PSRAM
 * - Native USB support
 * - SPI Hardware: FSPI peripheral
 * - 45 programmable GPIOs
 * 
 * MAX72xx LED Matrix Driver:
 * - 8x8 LED matrix controller
 * - SPI interface (3-wire + power)
 * - Brightness control (16 levels)
 * - Cascadable for multiple displays
 * 
 * CIRCUIT DIAGRAM:
 * ================
 *     ESP32-S3-DevKitC-1          8x8 LED Matrix (MAX72xx)
 *    ┌─────────────────┐          ┌─────────────────┐
 *    │                 │          │                 │
 *    │            3.3V ├──────────┤ VCC             │
 *    │             GND ├──────────┤ GND             │
 *    │                 │          │                 │
 *    │ GPIO11 (MOSI)   ├──────────┤ DIN (Data In)   │
 *    │ GPIO12 (SCK)    ├──────────┤ CLK (Clock)     │
 *    │ GPIO10 (CS)     ├──────────┤ CS (Chip Sel)   │
 *    │                 │          │                 │
 *    └─────────────────┘          └─────────────────┘
 * 
 * COMPONENTS:
 * ===========
 * 1x ESP32-S3-DevKitC-1 v1.1 development board
 * 1x 8x8 LED Matrix with MAX7219/MAX7221 driver
 * 5x Female-to-male jumper wires
 * 1x Breadboard (optional)
 * 1x USB-C cable for programming
 * 
 * GPIO ASSIGNMENTS:
 * =================
 * GPIO10 - SPI CS (Chip Select) for LED matrix
 * GPIO11 - SPI MOSI (Master Out Slave In) - DIN
 * GPIO12 - SPI SCK (Serial Clock) - CLK  
 * GPIO13 - SPI MISO (not used, but reserved)
 * 
 * TECHNICAL NOTES:
 * ================
 * - Uses FSPI (default SPI peripheral) on ESP32-S3
 * - SPI frequency optimized for LED matrix refresh rate
 * - Brightness levels: 0 (dim) to 15 (maximum)
 * - Coordinate system: (0,0) at top-left, (7,7) at bottom-right
 * - Matrix orientation can be adjusted in software
 * - Power consumption: ~20mA per lit LED at max brightness
 * 
 * IDE REQUIREMENTS:
 * =================
 * - Arduino IDE 2.0+
 * - ESP32 Arduino Core v3.0+
 * - Board: "ESP32S3 Dev Module"
 * - Libraries: SPI (built-in), Adafruit_GFX, Max72xxPanel
 * 
 * LEARNING OBJECTIVES:
 * ====================
 * 1. Understand SPI communication protocol
 * 2. Learn graphics programming concepts
 * 3. Practice array manipulation and data structures
 * 4. Implement basic animation techniques
 * 5. Debug hardware connections and timing
 * 
 * Created: April 1, 2019 by Peter Dalmaris
 * Updated: December 2024 for ESP32-S3 compatibility and educational enhancement
 */

#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Max72xxPanel.h>

// ===============================================================================
// CONFIGURATION CONSTANTS
// ===============================================================================

// SPI Pin Assignments for ESP32-S3 (FSPI peripheral)
constexpr uint8_t PIN_CS = 10;      // Chip Select pin
constexpr uint8_t PIN_MOSI = 11;    // Master Out Slave In (DIN)
constexpr uint8_t PIN_SCK = 12;     // Serial Clock (CLK)
// Note: MISO (GPIO13) is available but not used by LED matrix

// Display Configuration
constexpr uint8_t MATRIX_WIDTH = 8;           // Matrix width in pixels
constexpr uint8_t MATRIX_HEIGHT = 8;          // Matrix height in pixels
constexpr uint8_t DISPLAY_INTENSITY = 4;     // Brightness (0-15)
constexpr uint8_t NUM_MATRICES_HORIZONTAL = 1; // Number of matrices horizontally
constexpr uint8_t NUM_MATRICES_VERTICAL = 1;   // Number of matrices vertically

// Animation Settings
constexpr uint16_t ANIMATION_DELAY_FAST = 20;  // Fast animation delay (ms)
constexpr uint16_t ANIMATION_DELAY_SLOW = 60;  // Slow animation delay (ms)
constexpr uint16_t PATTERN_DISPLAY_TIME = 2000; // Time to show each pattern (ms)

// Pattern Data Structure Constants
constexpr uint8_t TOTAL_PIXELS_IN_SMILEY = 25;
constexpr uint8_t COORDINATES_PER_PIXEL = 2;   // X and Y coordinates

// ===============================================================================
// UTILITY HELPER FUNCTIONS
// ===============================================================================

// Helper function to create repeated characters (Arduino String has no repeat method)
String repeatChar(char character, int count) {
    String result = "";
    for (int i = 0; i < count; i++) {
        result += character;
    }
    return result;
}

// ===============================================================================
// GLOBAL OBJECTS AND VARIABLES
// ===============================================================================

// Initialize LED matrix object
Max72xxPanel matrix = Max72xxPanel(PIN_CS, NUM_MATRICES_HORIZONTAL, NUM_MATRICES_VERTICAL);

// Performance monitoring
unsigned long lastPatternChange = 0;
uint8_t currentPattern = 0;
constexpr uint8_t TOTAL_PATTERNS = 6;

// Smiley face pattern coordinates [pixel_index][x,y]
// This array stores the coordinates of each pixel that makes up a smiley face
const uint8_t smileyPattern[TOTAL_PIXELS_IN_SMILEY][COORDINATES_PER_PIXEL] = {
    // Left eye
    {1, 0}, {2, 0}, {1, 1}, {1, 2}, {2, 1}, {2, 2},
    // Right eye  
    {5, 0}, {6, 0}, {5, 1}, {5, 2}, {6, 1}, {6, 2},
    // Nose
    {3, 1}, {3, 2}, {3, 3}, {3, 4}, {4, 4},
    // Mouth (smile curve)
    {0, 5}, {1, 6}, {2, 7}, {3, 7}, {4, 7}, {5, 7}, {6, 6}, {7, 5}
};

// ===============================================================================
// SETUP FUNCTION
// ===============================================================================

void setup() {
    // Initialize Serial Communication for debugging
    Serial.begin(115200);
    while (!Serial && millis() < 3000) {
        // Wait for Serial connection or timeout after 3 seconds
        delay(10);
    }
    
    Serial.println("\n==================================================");
    Serial.println("ESP32-S3 8x8 LED Matrix Display Demo");
    Serial.println("Course: IoT Development with ESP32-S3");
    Serial.println("Lesson: 06.041 - SPI LED Matrix Control");
    Serial.println("==================================================");
    
    // Initialize SPI with explicit pin configuration
    initializeSPI();
    
    // Configure LED matrix display
    initializeMatrix();
    
    // Display startup information
    displayStartupInfo();
    
    // Show initial pattern
    displaySmileyFace();
    
    Serial.println("Setup complete! Starting pattern cycle...");
    Serial.println("Patterns will change every " + String(PATTERN_DISPLAY_TIME) + "ms");
}

// ===============================================================================
// MAIN LOOP
// ===============================================================================

void loop() {
    // Cycle through different display patterns for educational demonstration
    unsigned long currentTime = millis();
    
    if (currentTime - lastPatternChange >= PATTERN_DISPLAY_TIME) {
        lastPatternChange = currentTime;
        
        // Clear display before showing new pattern
        clearDisplay();
        
        // Display different patterns based on current pattern index
        switch (currentPattern) {
            case 0:
                Serial.println("Pattern 0: Smiley Face");
                displaySmileyFace();
                break;
                
            case 1:
                Serial.println("Pattern 1: Border Rectangle");
                displayBorderRectangle();
                break;
                
            case 2:
                Serial.println("Pattern 2: Diagonal Lines");
                displayDiagonalLines();
                break;
                
            case 3:
                Serial.println("Pattern 3: Circle");
                displayCircle();
                break;
                
            case 4:
                Serial.println("Pattern 4: Character 'A'");
                displayCharacter('A');
                break;
                
            case 5:
                Serial.println("Pattern 5: Pixel Animation");
                animateMovingPixel();
                break;
        }
        
        // Move to next pattern
        currentPattern = (currentPattern + 1) % TOTAL_PATTERNS;
        
        // Show memory usage for educational purposes
        showMemoryUsage();
    }
    
    // Small delay to prevent overwhelming the system
    delay(10);
}

// ===============================================================================
// INITIALIZATION FUNCTIONS
// ===============================================================================

void initializeSPI() {
    Serial.println("Initializing SPI communication...");
    
    // Initialize SPI with custom pins for ESP32-S3
    SPI.begin(PIN_SCK, -1, PIN_MOSI, PIN_CS); // SCK, MISO, MOSI, SS
    
    Serial.println("SPI Configuration:");
    Serial.println("  Clock (SCK): GPIO" + String(PIN_SCK));
    Serial.println("  Data (MOSI): GPIO" + String(PIN_MOSI));
    Serial.println("  Chip Select: GPIO" + String(PIN_CS));
    Serial.println("  Frequency: Default SPI speed");
}

void initializeMatrix() {
    Serial.println("Configuring LED matrix...");
    
    // Set display brightness (0 = dim, 15 = bright)
    matrix.setIntensity(DISPLAY_INTENSITY);
    
    // Set display orientation if needed
    // matrix.setRotation(0, 1); // Uncomment to rotate display 90 degrees
    
    // Clear the display
    matrix.fillScreen(LOW);
    matrix.write();
    
    Serial.println("Matrix Configuration:");
    Serial.println("  Dimensions: " + String(MATRIX_WIDTH) + "x" + String(MATRIX_HEIGHT));
    Serial.println("  Brightness: " + String(DISPLAY_INTENSITY) + "/15");
    Serial.println("  Number of displays: " + String(NUM_MATRICES_HORIZONTAL * NUM_MATRICES_VERTICAL));
}

void displayStartupInfo() {
    Serial.println("\nHardware Information:");
    Serial.println("  Chip: " + String(ESP.getChipModel()));
    Serial.println("  Chip Revision: " + String(ESP.getChipRevision()));
    Serial.println("  CPU Frequency: " + String(ESP.getCpuFreqMHz()) + " MHz");
    Serial.println("  Flash Size: " + String(ESP.getFlashChipSize() / 1024 / 1024) + " MB");
    Serial.println("  Free Heap: " + String(ESP.getFreeHeap()) + " bytes");
}

// ===============================================================================
// DISPLAY PATTERN FUNCTIONS
// ===============================================================================

void displaySmileyFace() {
    Serial.println("Drawing smiley face using coordinate array...");
    
    // Use the smiley pattern array to draw the face
    for (uint8_t pixel = 0; pixel < TOTAL_PIXELS_IN_SMILEY; pixel++) {
        uint8_t x = smileyPattern[pixel][0];
        uint8_t y = smileyPattern[pixel][1];
        
        // Validate coordinates before drawing
        if (x < MATRIX_WIDTH && y < MATRIX_HEIGHT) {
            matrix.drawPixel(x, y, HIGH);
        }
    }
    
    // Update the display
    matrix.write();
    
    Serial.println("  Pixels drawn: " + String(TOTAL_PIXELS_IN_SMILEY));
    Serial.println("  Pattern data size: " + String(sizeof(smileyPattern)) + " bytes");
}

void displayBorderRectangle() {
    Serial.println("Drawing border rectangle...");
    
    // Draw rectangle border (not filled)
    matrix.drawRect(0, 0, MATRIX_WIDTH, MATRIX_HEIGHT, HIGH);
    matrix.write();
    
    Serial.println("  Rectangle: 8x8 border only");
}

void displayDiagonalLines() {
    Serial.println("Drawing diagonal lines...");
    
    // Draw diagonal lines from corners
    matrix.drawLine(0, 0, 7, 7, HIGH);  // Top-left to bottom-right
    matrix.drawLine(0, 7, 7, 0, HIGH);  // Bottom-left to top-right
    matrix.write();
    
    Serial.println("  Lines: Two diagonals forming an X");
}

void displayCircle() {
    Serial.println("Drawing circle...");
    
    // Draw circle centered at (3,3) with radius 3
    matrix.drawCircle(3, 3, 3, HIGH);
    matrix.write();
    
    Serial.println("  Circle: Center (3,3), Radius 3");
}

void displayCharacter(char character) {
    Serial.println("Drawing character: " + String(character));
    
    // Draw character at position (0,0) with size 1
    matrix.drawChar(0, 0, character, HIGH, LOW, 1);
    matrix.write();
    
    Serial.println("  Character size: 1 (5x7 pixels)");
}

void clearDisplay() {
    matrix.fillScreen(LOW);
    matrix.write();
}

// ===============================================================================
// ANIMATION FUNCTIONS
// ===============================================================================

void animateMovingPixel() {
    Serial.println("Starting pixel animation...");
    
    const uint8_t ANIMATION_ROW = 3;  // Middle row
    uint32_t animationStart = millis();
    
    // Move pixel left to right
    for (uint8_t x = 0; x < MATRIX_WIDTH; x++) {
        clearDisplay();
        matrix.drawPixel(x, ANIMATION_ROW, HIGH);
        matrix.write();
        delay(ANIMATION_DELAY_FAST);
    }
    
    // Move pixel right to left
    for (int8_t x = MATRIX_WIDTH - 1; x >= 0; x--) {
        clearDisplay();
        matrix.drawPixel(x, ANIMATION_ROW, HIGH);
        matrix.write();
        delay(ANIMATION_DELAY_SLOW);
    }
    
    uint32_t animationDuration = millis() - animationStart;
    Serial.println("  Animation completed in " + String(animationDuration) + "ms");
}

// ===============================================================================
// UTILITY FUNCTIONS
// ===============================================================================

void showMemoryUsage() {
    uint32_t freeHeap = ESP.getFreeHeap();
    Serial.println("Free heap memory: " + String(freeHeap) + " bytes");
    
    if (freeHeap < 10000) {  // Warning threshold
        Serial.println("WARNING: Low memory detected!");
    }
}

void drawCustomPattern(const uint8_t pattern[][COORDINATES_PER_PIXEL], uint8_t patternSize) {
    // Generic function to draw any pattern from a coordinate array
    Serial.println("Drawing custom pattern with " + String(patternSize) + " pixels");
    
    for (uint8_t i = 0; i < patternSize; i++) {
        uint8_t x = pattern[i][0];
        uint8_t y = pattern[i][1];
        
        if (x < MATRIX_WIDTH && y < MATRIX_HEIGHT) {
            matrix.drawPixel(x, y, HIGH);
        } else {
            Serial.println("WARNING: Coordinate (" + String(x) + "," + String(y) + ") out of bounds");
        }
    }
    
    matrix.write();
}

// ===============================================================================
// EDUCATIONAL HELPER FUNCTIONS
// ===============================================================================

void demonstratePixelAddressing() {
    // Educational function to show how pixel addressing works
    Serial.println("\nPixel Addressing Demonstration:");
    Serial.println("Matrix coordinate system: (0,0) = top-left, (7,7) = bottom-right");
    
    clearDisplay();
    
    // Light up corners to show coordinate system
    matrix.drawPixel(0, 0, HIGH);  // Top-left
    matrix.drawPixel(7, 0, HIGH);  // Top-right
    matrix.drawPixel(0, 7, HIGH);  // Bottom-left
    matrix.drawPixel(7, 7, HIGH);  // Bottom-right
    matrix.write();
    
    Serial.println("Corner pixels lit: (0,0), (7,0), (0,7), (7,7)");
}

void testSPICommunication() {
    // Function to verify SPI communication is working
    Serial.println("\nTesting SPI Communication:");
    
    // Try different brightness levels
    for (uint8_t brightness = 0; brightness <= 15; brightness += 5) {
        matrix.setIntensity(brightness);
        matrix.fillScreen(HIGH);
        matrix.write();
        delay(200);
        Serial.println("  Brightness level: " + String(brightness));
    }
    
    // Reset to normal brightness
    matrix.setIntensity(DISPLAY_INTENSITY);
    clearDisplay();
    
    Serial.println("SPI communication test completed");
}
