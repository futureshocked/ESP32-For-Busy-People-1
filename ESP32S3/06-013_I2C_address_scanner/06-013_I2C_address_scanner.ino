/*  04.063 - I2C Scanner for ESP32-S3
 *
 * Course: IoT Development with ESP32-S3
 * Target Hardware: ESP32-S3-DevKitC-1 v1.1
 * Module: ESP32-S3-WROOM-1-N16R16V
 *
 * This sketch scans the I2C bus for connected devices and displays their
 * hexadecimal addresses. It's an essential debugging tool for I2C projects
 * and helps verify that I2C devices are properly connected and responding.
 *
 * HARDWARE INFORMATION:
 * =====================
 * Board: ESP32-S3-DevKitC-1 v1.1
 * Flash: 16 MB
 * PSRAM: 16 MB
 * Operating Voltage: 3.3V
 * I2C Bus: Hardware I2C peripheral with configurable pins
 *
 * CIRCUIT DIAGRAM:
 * ================
 *     ESP32-S3-DevKitC-1        I2C Device
 *     ==================        ==========
 *            3V3       ------>     VCC/VDD
 *            GND       ------>     GND
 *          GPIO8       ------>     SDA
 *          GPIO9       ------>     SCL
 *
 * Note: Add 4.7kΩ pull-up resistors from SDA and SCL to 3.3V
 * if your I2C devices don't have built-in pull-ups.
 *
 * COMPONENTS:
 * ===========
 * Required:
 * - ESP32-S3-DevKitC-1 v1.1
 * - At least one I2C device (for testing)
 * - Breadboard and jumper wires
 *
 * Optional:
 * - 2x 4.7kΩ resistors (pull-ups for SDA/SCL)
 * - I2C devices to test: RTC modules, sensors, displays, etc.
 *
 * GPIO ASSIGNMENTS:
 * =================
 * GPIO8  - I2C SDA (Serial Data Line)
 * GPIO9  - I2C SCL (Serial Clock Line)
 *
 * TECHNICAL NOTES:
 * ================
 * - I2C uses 7-bit addressing (0x01 to 0x7F)
 * - Address 0x00 is reserved for general call
 * - Addresses 0x78-0x7F are reserved for 10-bit addressing
 * - Standard I2C clock speed: 100kHz (default)
 * - Fast mode I2C: up to 400kHz
 * - ESP32-S3 supports multiple I2C buses (we use Wire/I2C0)
 *
 * COMMON I2C DEVICE ADDRESSES:
 * ============================
 * 0x20-0x27: PCF8574 I/O expander
 * 0x48-0x4F: ADS1115 ADC, PCF8591 ADC
 * 0x50-0x57: EEPROM (24C series)
 * 0x68: DS1307/DS3231 RTC, MPU6050 IMU
 * 0x76-0x77: BMP280/BME280 pressure sensor
 *
 * IDE REQUIREMENTS:
 * =================
 * - Arduino IDE 2.0+
 * - ESP32 Arduino Core v3.0+
 * - Board: "ESP32S3 Dev Module"
 * - USB CDC On Boot: "Enabled" (for Serial output)
 *
 * Created: April 1 2019 by Peter Dalmaris (original version)
 * Updated: January 2025 for ESP32-S3 compatibility
 * Original reference: https://playground.arduino.cc/Main/I2cScanner/
 */

#include <Wire.h>

// ================================================================================================
//                                    CONFIGURATION CONSTANTS
// ================================================================================================

// I2C pin assignments for ESP32-S3-DevKitC-1
const uint8_t I2C_SDA_PIN = 8;      // GPIO8 for I2C data line
const uint8_t I2C_SCL_PIN = 9;      // GPIO9 for I2C clock line

// I2C configuration
const uint32_t I2C_FREQUENCY = 100000;    // Standard I2C frequency (100kHz)
const uint32_t SERIAL_BAUD_RATE = 115200; // Serial communication speed
const uint16_t SCAN_DELAY_MS = 5000;      // Delay between scans (5 seconds)

// I2C address range (7-bit addressing)
const uint8_t I2C_START_ADDRESS = 0x01;   // First valid I2C address
const uint8_t I2C_END_ADDRESS = 0x7F;     // Last valid I2C address

// ================================================================================================
//                                      GLOBAL VARIABLES
// ================================================================================================

uint16_t totalScansPerformed = 0;          // Track number of completed scans

// ================================================================================================
//                                        SETUP FUNCTION
// ================================================================================================

void setup() {
    // Initialize serial communication first for debugging
    Serial.begin(SERIAL_BAUD_RATE);
    
    // Wait for serial connection (important for USB CDC)
    uint16_t timeout = 0;
    while (!Serial && timeout < 3000) {
        delay(10);
        timeout += 10;
    }
    
    // Initialize I2C communication with custom pins
    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
    Wire.setClock(I2C_FREQUENCY);
    
    // Display startup information
    printStartupMessage();
}

// ================================================================================================
//                                         MAIN LOOP
// ================================================================================================

void loop() {
    performI2CScan();
    
    // Wait before next scan
    Serial.println("Waiting 5 seconds before next scan...\n");
    Serial.println("========================================");
    delay(SCAN_DELAY_MS);
}

// ================================================================================================
//                                    UTILITY FUNCTIONS
// ================================================================================================

/**
 * Displays startup information and configuration details
 */
void printStartupMessage() {
    Serial.println("\n" + String('=', 50));
    Serial.println("         ESP32-S3 I2C Scanner v2.0");
    Serial.println(String('=', 50));
    Serial.println("Hardware: ESP32-S3-DevKitC-1 v1.1");
    Serial.println("I2C Configuration:");
    Serial.println("  SDA Pin: GPIO" + String(I2C_SDA_PIN));
    Serial.println("  SCL Pin: GPIO" + String(I2C_SCL_PIN));
    Serial.println("  Frequency: " + String(I2C_FREQUENCY / 1000) + " kHz");
    Serial.println("  Address Range: 0x" + String(I2C_START_ADDRESS, HEX) + 
                   " to 0x" + String(I2C_END_ADDRESS, HEX));
    Serial.println(String('=', 50));
    Serial.println("Starting I2C device scan...\n");
}

/**
 * Performs a complete I2C bus scan and reports results
 */
void performI2CScan() {
    uint8_t devicesFound = 0;
    uint8_t errorCount = 0;
    
    totalScansPerformed++;
    
    Serial.println("Scan #" + String(totalScansPerformed) + " - Scanning I2C bus...");
    Serial.println("Address  Status     Device Info");
    Serial.println("-------  ---------  -----------");
    
    // Scan all possible 7-bit I2C addresses
    for (uint8_t address = I2C_START_ADDRESS; address <= I2C_END_ADDRESS; address++) {
        Wire.beginTransmission(address);
        uint8_t error = Wire.endTransmission();
        
        // Process scan result
        switch (error) {
            case 0:
                // Device found and responding
                Serial.print("0x");
                if (address < 0x10) Serial.print("0");
                Serial.print(address, HEX);
                Serial.print("    SUCCESS    ");
                Serial.println(getDeviceInfo(address));
                devicesFound++;
                break;
                
            case 1:
                // Data too long for transmit buffer (shouldn't happen in scan)
                reportError(address, "Buffer overflow");
                errorCount++;
                break;
                
            case 2:
                // NACK received on address transmission (no device)
                // This is normal - don't report as error
                break;
                
            case 3:
                // NACK received on data transmission (shouldn't happen in scan)
                reportError(address, "Data NACK");
                errorCount++;
                break;
                
            case 4:
                // Unknown error
                reportError(address, "Unknown error");
                errorCount++;
                break;
                
            default:
                // Unexpected error code
                reportError(address, "Error code " + String(error));
                errorCount++;
                break;
        }
        
        // Small delay between address checks
        delay(1);
    }
    
    // Display scan summary
    printScanSummary(devicesFound, errorCount);
}

/**
 * Reports an I2C communication error
 */
void reportError(uint8_t address, String errorDescription) {
    Serial.print("0x");
    if (address < 0x10) Serial.print("0");
    Serial.print(address, HEX);
    Serial.print("    ERROR      ");
    Serial.println(errorDescription);
}

/**
 * Provides information about common I2C device addresses
 */
String getDeviceInfo(uint8_t address) {
    switch (address) {
        case 0x20: case 0x21: case 0x22: case 0x23:
        case 0x24: case 0x25: case 0x26: case 0x27:
            return "Possible PCF8574 I/O Expander";
            
        case 0x48: case 0x49: case 0x4A: case 0x4B:
            return "Possible ADS1115 ADC or similar";
            
        case 0x50: case 0x51: case 0x52: case 0x53:
        case 0x54: case 0x55: case 0x56: case 0x57:
            return "Possible EEPROM (24C series)";
            
        case 0x68:
            return "Possible DS1307/DS3231 RTC or MPU6050 IMU";
            
        case 0x69:
            return "Possible MPU6050 IMU (alt address)";
            
        case 0x76:
            return "Possible BMP280/BME280 sensor";
            
        case 0x77:
            return "Possible BMP280/BME280 sensor (alt address)";
            
        case 0x3C:
            return "Possible SSD1306 OLED display";
            
        case 0x3D:
            return "Possible SSD1306 OLED display (alt address)";
            
        default:
            return "Unknown device type";
    }
}

/**
 * Displays scan results summary
 */
void printScanSummary(uint8_t devicesFound, uint8_t errorCount) {
    Serial.println("-------  ---------  -----------");
    
    if (devicesFound == 0) {
        Serial.println("No I2C devices found.");
        Serial.println("\nTroubleshooting tips:");
        Serial.println("- Check wiring connections");
        Serial.println("- Verify device power supply");
        Serial.println("- Add 4.7kΩ pull-up resistors to SDA/SCL");
        Serial.println("- Check if device requires different voltage");
    } else {
        Serial.println("Found " + String(devicesFound) + " device(s).");
        
        if (errorCount > 0) {
            Serial.println("Warning: " + String(errorCount) + " error(s) occurred during scan.");
        }
    }
    
    Serial.println("Scan completed in ~" + String((I2C_END_ADDRESS - I2C_START_ADDRESS + 1) * 2) + "ms");
}
