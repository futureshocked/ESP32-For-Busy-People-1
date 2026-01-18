/*  06.011 - I2C LCD Display with Advanced Features for ESP32-S3
 * 
 * Course: IoT Development with ESP32-S3
 * Target Hardware: ESP32-S3-DevKitC-1 v1.1
 * Module: ESP32-S3-WROOM-1-N16R16V
 * 
 * This comprehensive sketch demonstrates how to interface a 16x2 LCD display
 * using an I2C backpack adaptor with the ESP32-S3. This modern implementation
 * includes error handling, I2C device scanning, multiple display modes, and
 * educational debugging features.
 * 
 * The I2C backpack uses the PCF8574 port expander IC to reduce the number
 * of GPIO pins required from 6+ to just 2 (SDA and SCL).
 * 
 * HARDWARE INFORMATION:
 * =====================
 * ESP32-S3-DevKitC-1 v1.1 Specifications:
 * - ESP32-S3-WROOM-1-N16R16V module
 * - 16 MB Flash, 16 MB PSRAM
 * - Dual-core Xtensa LX7 @ 240MHz
 * - Native USB support
 * - Multiple I2C interfaces available
 * 
 * CIRCUIT DIAGRAM:
 * ================
 * 
 *   ESP32-S3-DevKitC-1 v1.1        I2C LCD Backpack (PCF8574)
 *   ┌─────────────────────┐        ┌─────────────────────────┐
 *   │                     │        │  ┌─────────────────────┐│
 *   │ 3V3 ────────────────┼────────┼──│ VCC                 ││
 *   │                     │        │  │                     ││
 *   │ GND ────────────────┼────────┼──│ GND                 ││
 *   │                     │        │  │                     ││
 *   │ GPIO21 (SDA) ───────┼────────┼──│ SDA   16x2 LCD      ││
 *   │                     │        │  │       Character     ││
 *   │ GPIO8 (SCL) ────────┼────────┼──│ SCL   Display       ││
 *   │                     │        │  └─────────────────────┘│
 *   └─────────────────────┘        └─────────────────────────┘
 * 
 * COMPONENTS:
 * ===========
 * 1x ESP32-S3-DevKitC-1 v1.1 development board
 * 1x 16x2 Character LCD display (HD44780 compatible)
 * 1x I2C LCD backpack module with PCF8574 chip
 * 4x Female-to-Female jumper wires
 * 1x Breadboard (optional)
 * 1x USB-C cable for power and programming
 * 
 * GPIO ASSIGNMENTS:
 * =================
 * GPIO21 (J3-18): I2C SDA (Serial Data)
 * GPIO8  (J1-12): I2C SCL (Serial Clock)
 * 3V3:           Power supply (+3.3V)
 * GND:           Ground reference
 * 
 * TECHNICAL NOTES:
 * ================
 * - I2C bus operates at 3.3V logic levels
 * - Default I2C speed: 100kHz (can be increased to 400kHz)
 * - PCF8574 default addresses: 0x27, 0x3F (depends on A0-A2 jumpers)
 * - Built-in pull-up resistors on ESP32-S3 I2C pins
 * - Maximum I2C cable length: ~1 meter for reliable operation
 * - The script includes automatic I2C address detection
 * 
 * IDE REQUIREMENTS:
 * =================
 * - Arduino IDE 2.0+
 * - ESP32 Arduino Core v3.0+
 * - Board: "ESP32S3 Dev Module"
 * - Library: LiquidCrystal_I2C (install via Library Manager)
 * 
 * TROUBLESHOOTING:
 * ================
 * - If "Device not found": Check wiring and run I2C scanner
 * - If garbled text: Verify LCD compatibility (HD44780 standard)
 * - If no backlight: Check backpack jumper or call lcd.backlight()
 * - If partial display: Check power supply capacity (>200mA)
 * 
 * Created: April 1, 2019 by Peter Dalmaris
 * Updated: December 2024 for ESP32-S3 compatibility and modern practices
 */

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// ===================================================================
// CONFIGURATION CONSTANTS
// ===================================================================
const uint8_t SDA_PIN = 21;              // I2C Serial Data pin
const uint8_t SCL_PIN = 8;               // I2C Serial Clock pin
const uint32_t I2C_FREQUENCY = 100000;   // I2C bus speed (100kHz)
const uint32_t SERIAL_BAUD = 115200;     // Serial monitor baud rate

// LCD Configuration
const uint8_t LCD_COLUMNS = 16;          // Number of LCD columns
const uint8_t LCD_ROWS = 2;              // Number of LCD rows
const uint16_t DEMO_DELAY = 2000;        // Delay between demonstrations (ms)

// Common I2C addresses for PCF8574-based LCD backpacks
const uint8_t POSSIBLE_ADDRESSES[] = {0x27, 0x3F, 0x26, 0x20};
const uint8_t NUM_ADDRESSES = sizeof(POSSIBLE_ADDRESSES) / sizeof(POSSIBLE_ADDRESSES[0]);

// ===================================================================
// GLOBAL VARIABLES
// ===================================================================
LiquidCrystal_I2C lcd(0x27, LCD_COLUMNS, LCD_ROWS);  // Default address
uint8_t detected_address = 0x00;                     // Detected I2C address
bool lcd_initialized = false;                        // LCD initialization status

// ===================================================================
// SETUP FUNCTION
// ===================================================================
void setup() {
    // Initialize serial communication for debugging
    Serial.begin(SERIAL_BAUD);
    delay(2000);  // Allow time for Serial Monitor to connect
    
    Serial.println("========================================");
    Serial.println("ESP32-S3 I2C LCD Display Demo");
    Serial.println("========================================");
    Serial.printf("Target: ESP32-S3-DevKitC-1 v1.1\n");
    Serial.printf("SDA Pin: GPIO%d\n", SDA_PIN);
    Serial.printf("SCL Pin: GPIO%d\n", SCL_PIN);
    Serial.printf("I2C Frequency: %d Hz\n", I2C_FREQUENCY);
    Serial.println("========================================");
    
    // Initialize I2C with custom pins
    if (!initializeI2C()) {
        Serial.println("❌ Failed to initialize I2C bus!");
        Serial.println("Check wiring and restart the device.");
        while (true) {
            delay(1000);
        }
    }
    
    // Scan for I2C devices and find LCD
    if (!findLCDAddress()) {
        Serial.println("❌ No LCD found on I2C bus!");
        Serial.println("Check connections and device address.");
        while (true) {
            delay(1000);
        }
    }
    
    // Initialize LCD with detected address
    if (!initializeLCD()) {
        Serial.println("❌ Failed to initialize LCD!");
        while (true) {
            delay(1000);
        }
    }
    
    // Run comprehensive LCD demonstration
    runLCDDemonstration();
    
    Serial.println("\n✅ Setup completed successfully!");
    Serial.println("📝 Check the LCD for demonstration results.");
    Serial.println("🔄 The device will now enter the main loop.\n");
}

// ===================================================================
// MAIN LOOP
// ===================================================================
void loop() {
    // Main loop runs a continuous clock display
    displaySystemClock();
    delay(1000);  // Update every second
}

// ===================================================================
// I2C INITIALIZATION FUNCTION
// ===================================================================
bool initializeI2C() {
    Serial.println("🔧 Initializing I2C bus...");
    
    // Initialize I2C with custom pins and frequency
    Wire.begin(SDA_PIN, SCL_PIN, I2C_FREQUENCY);
    
    // Test I2C bus by scanning for any device
    Wire.beginTransmission(0x00);
    uint8_t error = Wire.endTransmission();
    
    if (error == 0) {
        Serial.println("✅ I2C bus initialized successfully");
        return true;
    } else {
        Serial.printf("❌ I2C initialization failed with error: %d\n", error);
        return false;
    }
}

// ===================================================================
// I2C DEVICE SCANNER FUNCTION
// ===================================================================
bool findLCDAddress() {
    Serial.println("🔍 Scanning I2C bus for LCD device...");
    
    uint8_t devices_found = 0;
    
    for (uint8_t i = 0; i < NUM_ADDRESSES; i++) {
        uint8_t address = POSSIBLE_ADDRESSES[i];
        
        Wire.beginTransmission(address);
        uint8_t error = Wire.endTransmission();
        
        if (error == 0) {
            Serial.printf("✅ Device found at address 0x%02X\n", address);
            detected_address = address;
            devices_found++;
        } else {
            Serial.printf("⚪ No device at address 0x%02X\n", address);
        }
        
        delay(100);  // Small delay between scans
    }
    
    if (devices_found > 0) {
        Serial.printf("🎯 Using LCD at address 0x%02X\n", detected_address);
        return true;
    } else {
        Serial.println("❌ No I2C LCD devices found!");
        return false;
    }
}

// ===================================================================
// LCD INITIALIZATION FUNCTION
// ===================================================================
bool initializeLCD() {
    Serial.println("📺 Initializing LCD display...");
    
    // Create new LCD object with detected address
    lcd = LiquidCrystal_I2C(detected_address, LCD_COLUMNS, LCD_ROWS);
    
    // Initialize the LCD
    lcd.init();
    
    // Test LCD communication
    lcd.backlight();
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("ESP32-S3 Ready!");
    
    // Verify LCD is responding
    delay(500);
    lcd.clear();
    
    lcd_initialized = true;
    Serial.println("✅ LCD initialized successfully");
    
    return true;
}

// ===================================================================
// COMPREHENSIVE LCD DEMONSTRATION
// ===================================================================
void runLCDDemonstration() {
    Serial.println("\n🎬 Starting LCD demonstration sequence...");
    
    // Demo 1: Welcome message
    Serial.println("Demo 1: Welcome message");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Hello ESP32-S3!");
    lcd.setCursor(0, 1);
    lcd.print("I2C LCD Demo");
    delay(DEMO_DELAY);
    
    // Demo 2: Display information
    Serial.println("Demo 2: System information");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Addr: 0x");
    lcd.print(detected_address, HEX);
    lcd.setCursor(9, 0);
    lcd.print("16x2");
    lcd.setCursor(0, 1);
    lcd.print("GPIO21/8 I2C");
    delay(DEMO_DELAY);
    
    // Demo 3: Backlight control
    Serial.println("Demo 3: Backlight control");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Backlight Test");
    
    for (int i = 0; i < 3; i++) {
        lcd.setCursor(0, 1);
        lcd.print("OFF          ");
        lcd.noBacklight();
        delay(800);
        
        lcd.setCursor(0, 1);
        lcd.print("ON           ");
        lcd.backlight();
        delay(800);
    }
    
    // Demo 4: Scrolling text
    Serial.println("Demo 4: Text scrolling");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Scroll Demo");
    lcd.setCursor(0, 1);
    lcd.print("Text Moving -->");
    
    for (int i = 0; i < 8; i++) {
        lcd.scrollDisplayRight();
        delay(400);
    }
    
    for (int i = 0; i < 8; i++) {
        lcd.scrollDisplayLeft();
        delay(400);
    }
    
    // Demo 5: Character positioning
    Serial.println("Demo 5: Character positioning");
    lcd.clear();
    
    // Draw border
    lcd.setCursor(0, 0);
    lcd.print("****************");
    lcd.setCursor(0, 1);
    lcd.print("****************");
    
    delay(1000);
    
    // Clear center and add content
    lcd.setCursor(1, 0);
    lcd.print("  Position  ");
    lcd.setCursor(1, 1);
    lcd.print("    Test    ");
    delay(DEMO_DELAY);
    
    // Demo 6: Counter demonstration
    Serial.println("Demo 6: Counter demonstration");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Counter Demo:");
    
    for (int count = 0; count <= 20; count++) {
        lcd.setCursor(0, 1);
        lcd.print("Count: ");
        lcd.print(count);
        lcd.print("      ");  // Clear remaining characters
        delay(200);
    }
    
    Serial.println("✅ Demonstration sequence completed");
}

// ===================================================================
// SYSTEM CLOCK DISPLAY FUNCTION
// ===================================================================
void displaySystemClock() {
    static uint32_t seconds = 0;
    static uint32_t last_update = 0;
    
    uint32_t current_time = millis();
    
    // Update every second
    if (current_time - last_update >= 1000) {
        seconds++;
        last_update = current_time;
        
        uint32_t hours = seconds / 3600;
        uint32_t minutes = (seconds % 3600) / 60;
        uint32_t secs = seconds % 60;
        
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("ESP32-S3 Clock");
        
        lcd.setCursor(0, 1);
        if (hours < 10) lcd.print("0");
        lcd.print(hours);
        lcd.print(":");
        if (minutes < 10) lcd.print("0");
        lcd.print(minutes);
        lcd.print(":");
        if (secs < 10) lcd.print("0");
        lcd.print(secs);
        
        // Add uptime indicator
        lcd.setCursor(10, 1);
        lcd.print("UP:");
        lcd.print(hours);
        lcd.print("h");
        
        // Debug output to serial
        if (seconds % 10 == 0) {  // Every 10 seconds
            Serial.printf("⏰ System uptime: %02d:%02d:%02d\n", hours, minutes, secs);
        }
    }
}

// ===================================================================
// UTILITY FUNCTIONS
// ===================================================================

// Function to perform a complete I2C bus scan (educational)
void performI2CScan() {
    Serial.println("\n🔍 Performing complete I2C bus scan...");
    Serial.println("Scanning addresses 0x08 to 0x77...");
    
    uint8_t devices_found = 0;
    
    for (uint8_t address = 8; address < 120; address++) {
        Wire.beginTransmission(address);
        uint8_t error = Wire.endTransmission();
        
        if (error == 0) {
            Serial.printf("Device found at address 0x%02X\n", address);
            devices_found++;
        }
        
        delay(10);
    }
    
    Serial.printf("Scan complete. Found %d device(s).\n", devices_found);
}

// Function to display custom message (useful for projects)
void displayCustomMessage(const char* line1, const char* line2) {
    if (!lcd_initialized) return;
    
    lcd.clear();
    
    if (line1) {
        lcd.setCursor(0, 0);
        lcd.print(line1);
    }
    
    if (line2) {
        lcd.setCursor(0, 1);
        lcd.print(line2);
    }
}

// Function to check I2C connection health
bool checkI2CHealth() {
    Wire.beginTransmission(detected_address);
    uint8_t error = Wire.endTransmission();
    return (error == 0);
}
