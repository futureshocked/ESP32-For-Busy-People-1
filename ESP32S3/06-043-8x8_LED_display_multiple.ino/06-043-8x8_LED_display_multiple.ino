/*  06.043 - Multiple 8x8 LED Matrix Display with Graphics for ESP32-S3
 * 
 * Course: IoT Development with ESP32-S3
 * Target Hardware: ESP32-S3-DevKitC-1 v1.1
 * Module: ESP32-S3-WROOM-1-N16R16V
 * 
 * This comprehensive sketch demonstrates advanced graphics programming on multiple
 * 8x8 LED matrix displays using the MAX72xx driver IC. Students will learn about
 * SPI communication, 2D graphics primitives, animation techniques, and efficient
 * display management for embedded systems.
 * 
 * Learning Objectives:
 * - Master SPI communication protocols with multiple devices
 * - Understand 2D coordinate systems and graphics primitives
 * - Implement smooth animations and visual effects
 * - Learn efficient memory management for graphics data
 * - Practice modular programming with reusable functions
 * 
 * HARDWARE INFORMATION:
 * =====================
 * ESP32-S3-DevKitC-1 v1.1 Specifications:
 * - MCU: ESP32-S3 (dual-core Xtensa LX7 @ 240MHz)
 * - Flash: 16 MB
 * - PSRAM: 16 MB
 * - WiFi: 802.11 b/g/n
 * - Bluetooth: BLE 5.0
 * - USB: Native USB-C connector
 * - Operating Voltage: 3.3V
 * - SPI: Hardware SPI (HSPI/VSPI available)
 * 
 * LED Matrix Display Specifications:
 * - Type: 8x8 LED Matrix with MAX7219/MAX7221 driver
 * - Operating Voltage: 3.3V - 5V
 * - Communication: SPI (3-wire + power)
 * - Cascade: Multiple displays can be chained
 * - Brightness: 16 levels (0-15)
 * 
 * CIRCUIT DIAGRAM:
 * ================
 * 
 *     ESP32-S3-DevKitC-1                    LED Matrix Display Chain
 *     ┌─────────────────┐                  ┌──────┐  ┌──────┐  ┌──────┐  ┌──────┐
 *     │              3V3├──────────────────┤VCC   │  │VCC   │  │VCC   │  │VCC   │
 *     │              GND├──────────────────┤GND   │  │GND   │  │GND   │  │GND   │
 *     │         GPIO11  ├──────────────────┤DIN   ├──┤DIN   ├──┤DIN   ├──┤DIN   │
 *     │         GPIO12  ├──────────────────┤CLK   ├──┤CLK   ├──┤CLK   ├──┤CLK   │
 *     │         GPIO10  ├──────────────────┤CS    ├──┤CS    ├──┤CS    ├──┤CS    │
 *     │                 │                  │      │  │      │  │      │  │      │
 *     │      [USB-C]    │                  │ #0   │  │ #1   │  │ #2   │  │ #3   │
 *     └─────────────────┘                  └──────┘  └──────┘  └──────┘  └──────┘
 *                                           (Left)              (Right)
 * 
 * COMPONENTS:
 * ===========
 * 1x ESP32-S3-DevKitC-1 v1.1 Development Board
 * 4x 8x8 LED Matrix Display with MAX7219/MAX7221 driver
 * 1x Breadboard (full-size recommended)
 * 12x Male-to-Male jumper wires
 * 1x USB-C cable for programming and power
 * 
 * Optional:
 * 1x External 5V power supply (for higher brightness)
 * 1x Level shifter (if using 5V supply)
 * 
 * GPIO ASSIGNMENTS:
 * =================
 * GPIO10 (CS)   → LED Matrix CS (Chip Select)
 * GPIO11 (MOSI) → LED Matrix DIN (Data Input)
 * GPIO12 (SCK)  → LED Matrix CLK (Clock)
 * 3V3           → LED Matrix VCC (Power)
 * GND           → LED Matrix GND (Ground)
 * 
 * TECHNICAL NOTES:
 * ================
 * - MAX7219/MAX7221 supports up to 8 cascaded displays per CS line
 * - Each display draws ~150mA at full brightness
 * - Total current: 4 displays × 150mA = 600mA max
 * - ESP32-S3 3V3 pin can supply up to 600mA safely
 * - For production, consider external power supply
 * - SPI speed can be adjusted for longer wire runs
 * - Display orientation can be configured per panel
 * 
 * SAFETY CONSIDERATIONS:
 * ======================
 * - Always connect power before signal lines
 * - Verify voltage levels before connecting
 * - Use appropriate wire gauge for current load
 * - Add bypass capacitors for stable operation
 * - Implement current limiting in software
 * 
 * IDE REQUIREMENTS:
 * =================
 * - Arduino IDE 2.0+
 * - ESP32 Arduino Core v3.0+
 * - Board: "ESP32S3 Dev Module"
 * - Required Libraries:
 *   * SPI (Built-in)
 *   * Adafruit GFX Library (Install via Library Manager)
 *   * Max72xxPanel (Install via Library Manager)
 * 
 * Created: May 22, 2017 by Peter Dalmaris
 * Updated: December 28, 2024 for ESP32-S3 compatibility
 */

#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Max72xxPanel.h>

// =====================================================================================
// CONFIGURATION CONSTANTS
// =====================================================================================

// Hardware Configuration
constexpr uint8_t PIN_CS = 10;           // Chip Select pin (SPI CS)
constexpr uint8_t PIN_MOSI = 11;         // Master Out Slave In (SPI Data)
constexpr uint8_t PIN_SCK = 12;          // Serial Clock (SPI Clock)

// Display Configuration
constexpr uint8_t MATRIX_COUNT_H = 4;    // Number of displays horizontally
constexpr uint8_t MATRIX_COUNT_V = 1;    // Number of displays vertically
constexpr uint8_t TOTAL_WIDTH = MATRIX_COUNT_H * 8;  // Total display width (32 pixels)
constexpr uint8_t TOTAL_HEIGHT = MATRIX_COUNT_V * 8; // Total display height (8 pixels)

// Animation Configuration
constexpr uint8_t DEFAULT_BRIGHTNESS = 4; // Brightness level (0-15)
constexpr uint16_t ANIMATION_DELAY = 50;  // Milliseconds between animation frames
constexpr uint16_t CHAR_DELAY = 40;       // Milliseconds for character animation

// Debug Configuration
constexpr bool ENABLE_DEBUG = true;      // Enable serial debug output
constexpr uint32_t SERIAL_BAUD = 115200; // Serial communication speed

// =====================================================================================
// GLOBAL OBJECTS AND VARIABLES
// =====================================================================================

// Create matrix object with CS pin and display dimensions
Max72xxPanel matrix = Max72xxPanel(PIN_CS, MATRIX_COUNT_H, MATRIX_COUNT_V);

// Performance monitoring variables
unsigned long lastFrameTime = 0;
unsigned long frameCount = 0;
unsigned long lastFPSUpdate = 0;

// Animation state variables
int8_t currentAnimationMode = 0;  // Current animation being displayed
bool animationDirection = true;   // Animation direction flag

// =====================================================================================
// SMILEY FACE PATTERN DATA
// =====================================================================================

constexpr uint8_t SMILEY_PIXEL_COUNT = 25;
constexpr uint8_t COORDINATES_PER_PIXEL = 2;

// Smiley face pattern: {x, y} coordinates for each pixel
const uint8_t smileyPattern[SMILEY_PIXEL_COUNT][COORDINATES_PER_PIXEL] = {
  // Left Eye
  {1, 0}, {2, 0}, {1, 1}, {1, 2}, {2, 1}, {2, 2},
  // Right Eye  
  {5, 0}, {6, 0}, {5, 1}, {5, 2}, {6, 1}, {6, 2},
  // Nose
  {3, 1}, {3, 2}, {3, 3}, {3, 4}, {4, 4},
  // Mouth (smile curve)
  {0, 5}, {1, 6}, {2, 7}, {3, 7}, {4, 7}, {5, 7}, {6, 6}, {7, 5}
};

// =====================================================================================
// UTILITY FUNCTIONS
// =====================================================================================

void debugPrint(const char* message) {
  if (ENABLE_DEBUG) {
    Serial.print("[LED Matrix] ");
    Serial.println(message);
  }
}

void debugPrintValue(const char* label, int value) {
  if (ENABLE_DEBUG) {
    Serial.print("[LED Matrix] ");
    Serial.print(label);
    Serial.print(": ");
    Serial.println(value);
  }
}

void updateFPS() {
  frameCount++;
  unsigned long currentTime = millis();
  
  if (currentTime - lastFPSUpdate >= 1000) {
    if (ENABLE_DEBUG) {
      Serial.print("[Performance] FPS: ");
      Serial.println(frameCount);
    }
    frameCount = 0;
    lastFPSUpdate = currentTime;
  }
}

// =====================================================================================
// DISPLAY INITIALIZATION AND SETUP
// =====================================================================================

bool initializeDisplay() {
  debugPrint("Initializing LED matrix display...");
  
  // Configure SPI pins explicitly for ESP32-S3
  SPI.begin(PIN_SCK, -1, PIN_MOSI, PIN_CS);
  debugPrint("SPI interface initialized");
  
  // Set display brightness (0-15)
  matrix.setIntensity(DEFAULT_BRIGHTNESS);
  debugPrintValue("Brightness set to", DEFAULT_BRIGHTNESS);
  
  // Clear the display
  matrix.fillScreen(LOW);
  debugPrint("Display cleared");
  
  // Configure display positions (important for proper orientation)
  // Position format: setPosition(display_number, x_position, y_position)
  matrix.setPosition(0, 3, 0); // Rightmost display
  matrix.setPosition(1, 2, 0); // Second from right
  matrix.setPosition(2, 1, 0); // Second from left  
  matrix.setPosition(3, 0, 0); // Leftmost display
  debugPrint("Display positions configured");
  
  // Optional: Set rotation for specific displays if needed
  // matrix.setRotation(0, 2);    // Rotate first display 180 degrees
  // matrix.setRotation(3, 2);    // Rotate last display 180 degrees
  
  // Test display connectivity
  matrix.drawPixel(0, 0, HIGH);
  matrix.drawPixel(TOTAL_WIDTH - 1, 0, HIGH);
  matrix.write();
  delay(500);
  matrix.fillScreen(LOW);
  matrix.write();
  
  debugPrint("Display initialization complete");
  return true;
}

// =====================================================================================
// GRAPHICS DEMONSTRATION FUNCTIONS
// =====================================================================================

void demonstratePixels() {
  debugPrint("Demonstrating pixel drawing...");
  matrix.fillScreen(LOW);
  
  // Draw corner pixels
  matrix.drawPixel(0, 0, HIGH);                    // Top-left
  matrix.drawPixel(TOTAL_WIDTH - 1, 0, HIGH);     // Top-right
  matrix.drawPixel(0, TOTAL_HEIGHT - 1, HIGH);    // Bottom-left
  matrix.drawPixel(TOTAL_WIDTH - 1, TOTAL_HEIGHT - 1, HIGH); // Bottom-right
  
  matrix.write();
  delay(2000);
}

void demonstrateLines() {
  debugPrint("Demonstrating line drawing...");
  matrix.fillScreen(LOW);
  
  // Horizontal line
  matrix.drawLine(0, 2, TOTAL_WIDTH - 1, 2, HIGH);
  matrix.write();
  delay(1000);
  
  // Vertical lines
  matrix.drawLine(8, 0, 8, TOTAL_HEIGHT - 1, HIGH);
  matrix.drawLine(16, 0, 16, TOTAL_HEIGHT - 1, HIGH);
  matrix.drawLine(24, 0, 24, TOTAL_HEIGHT - 1, HIGH);
  matrix.write();
  delay(2000);
}

void demonstrateShapes() {
  debugPrint("Demonstrating shape drawing...");
  matrix.fillScreen(LOW);
  
  // Draw rectangles
  matrix.drawRect(1, 1, 6, 6, HIGH);       // Outline rectangle
  matrix.fillRect(10, 2, 4, 4, HIGH);      // Filled rectangle
  
  // Draw circles (if space permits)
  matrix.drawCircle(20, 3, 3, HIGH);       // Outline circle
  matrix.fillCircle(28, 3, 2, HIGH);       // Filled circle
  
  matrix.write();
  delay(3000);
}

void drawSmileyFace() {
  debugPrint("Drawing smiley face pattern...");
  matrix.fillScreen(LOW);
  
  // Draw smiley face using the pattern array
  for (uint8_t pixel = 0; pixel < SMILEY_PIXEL_COUNT; pixel++) {
    uint8_t x = smileyPattern[pixel][0];
    uint8_t y = smileyPattern[pixel][1];
    matrix.drawPixel(x, y, HIGH);
  }
  
  matrix.write();
  delay(3000);
}

// =====================================================================================
// ANIMATION FUNCTIONS
// =====================================================================================

void animateMovingPixel() {
  debugPrint("Starting pixel animation...");
  matrix.fillScreen(LOW);
  
  const uint8_t y_position = TOTAL_HEIGHT / 2;
  
  // Move pixel left to right
  for (int16_t x = 0; x < TOTAL_WIDTH; x++) {
    matrix.drawPixel(x, y_position, HIGH);
    matrix.write();
    delay(ANIMATION_DELAY);
    matrix.drawPixel(x, y_position, LOW);
    updateFPS();
  }
  
  // Move pixel right to left
  for (int16_t x = TOTAL_WIDTH - 1; x >= 0; x--) {
    matrix.drawPixel(x, y_position, HIGH);
    matrix.write();
    delay(ANIMATION_DELAY);
    matrix.drawPixel(x, y_position, LOW);
    updateFPS();
  }
}

void animateMovingCharacter(char character) {
  debugPrint("Starting character animation...");
  matrix.fillScreen(LOW);
  
  // Move character left to right
  for (int16_t x = 0; x < TOTAL_WIDTH; x++) {
    matrix.drawChar(x, 0, character, HIGH, LOW, 1);
    matrix.write();
    delay(CHAR_DELAY);
    matrix.drawChar(x, 0, character, LOW, LOW, 1);
    updateFPS();
  }
  
  // Move character right to left
  for (int16_t x = TOTAL_WIDTH - 1; x >= -6; x--) {  // -6 to let character exit completely
    matrix.drawChar(x, 0, character, HIGH, LOW, 1);
    matrix.write();
    delay(CHAR_DELAY);
    matrix.drawChar(x, 0, character, LOW, LOW, 1);
    updateFPS();
  }
}

void animateScrollingText(const char* text) {
  debugPrint("Starting text scrolling animation...");
  int16_t textLength = strlen(text) * 6; // Approximate character width
  
  for (int16_t x = TOTAL_WIDTH; x >= -textLength; x--) {
    matrix.fillScreen(LOW);
    matrix.setCursor(x, 0);
    matrix.print(text);
    matrix.write();
    delay(CHAR_DELAY);
    updateFPS();
  }
}

void animateWaveEffect() {
  debugPrint("Starting wave effect animation...");
  
  for (uint16_t frame = 0; frame < 100; frame++) {
    matrix.fillScreen(LOW);
    
    for (uint8_t x = 0; x < TOTAL_WIDTH; x++) {
      uint8_t y = 3 + sin((x + frame) * 0.2) * 2;  // Create wave pattern
      if (y < TOTAL_HEIGHT) {
        matrix.drawPixel(x, y, HIGH);
      }
    }
    
    matrix.write();
    delay(ANIMATION_DELAY);
    updateFPS();
  }
}

// =====================================================================================
// MAIN SETUP FUNCTION
// =====================================================================================

void setup() {
  // Initialize serial communication for debugging
  if (ENABLE_DEBUG) {
    Serial.begin(SERIAL_BAUD);
    while (!Serial && millis() < 3000) {
      delay(10); // Wait for serial connection or timeout
    }
    Serial.println();
    Serial.println("===============================================");
    Serial.println("ESP32-S3 Multiple LED Matrix Display Demo");
    Serial.println("Course: IoT Development with ESP32-S3");
    Serial.println("===============================================");
  }
  
  // Display system information
  debugPrint("System Information:");
  debugPrintValue("ESP32-S3 CPU Frequency (MHz)", ESP.getCpuFreqMHz());
  debugPrintValue("Free Heap Memory (bytes)", ESP.getFreeHeap());
  debugPrintValue("Flash Size (MB)", ESP.getFlashChipSize() / (1024 * 1024));
  
  // Initialize display
  if (!initializeDisplay()) {
    debugPrint("ERROR: Display initialization failed!");
    while (true) {
      delay(1000); // Halt execution on error
    }
  }
  
  // Run demonstration sequence
  debugPrint("Starting demonstration sequence...");
  
  demonstratePixels();
  demonstrateLines();
  demonstrateShapes();
  drawSmileyFace();
  
  // Show some text
  matrix.fillScreen(LOW);
  matrix.setCursor(2, 0);
  matrix.print("ESP32-S3");
  matrix.write();
  delay(2000);
  
  debugPrint("Setup complete. Starting main loop...");
  lastFPSUpdate = millis();
}

// =====================================================================================
// MAIN LOOP FUNCTION
// =====================================================================================

void loop() {
  // Cycle through different animations
  switch (currentAnimationMode) {
    case 0:
      animateMovingPixel();
      break;
      
    case 1:
      animateMovingCharacter('A');
      break;
      
    case 2:
      animateMovingCharacter('#');
      break;
      
    case 3:
      animateScrollingText("ESP32-S3 LED Matrix Demo");
      break;
      
    case 4:
      animateWaveEffect();
      break;
      
    default:
      drawSmileyFace();
      delay(2000);
      break;
  }
  
  // Cycle to next animation
  currentAnimationMode++;
  if (currentAnimationMode > 5) {
    currentAnimationMode = 0;
    debugPrint("Animation cycle completed, starting over...");
  }
  
  // Small delay between animations
  delay(1000);
}

// =====================================================================================
// ADDITIONAL HELPER FUNCTIONS FOR ADVANCED FEATURES
// =====================================================================================

void setBrightness(uint8_t brightness) {
  if (brightness > 15) brightness = 15;
  matrix.setIntensity(brightness);
  debugPrintValue("Brightness changed to", brightness);
}

void displaySystemStats() {
  if (ENABLE_DEBUG) {
    Serial.println("\n--- System Statistics ---");
    Serial.print("Uptime: ");
    Serial.print(millis() / 1000);
    Serial.println(" seconds");
    Serial.print("Free Heap: ");
    Serial.print(ESP.getFreeHeap());
    Serial.println(" bytes");
    Serial.print("CPU Frequency: ");
    Serial.print(ESP.getCpuFreqMHz());
    Serial.println(" MHz");
    Serial.println("------------------------\n");
  }
}

// Test function for troubleshooting
void runDisplayTest() {
  debugPrint("Running display connectivity test...");
  
  matrix.fillScreen(HIGH);
  matrix.write();
  delay(1000);
  
  matrix.fillScreen(LOW);
  matrix.write();
  delay(500);
  
  // Test each display individually
  for (uint8_t display = 0; display < MATRIX_COUNT_H; display++) {
    matrix.fillScreen(LOW);
    matrix.fillRect(display * 8, 0, 8, 8, HIGH);
    matrix.write();
    delay(500);
  }
  
  matrix.fillScreen(LOW);
  matrix.write();
  debugPrint("Display test complete");
}
