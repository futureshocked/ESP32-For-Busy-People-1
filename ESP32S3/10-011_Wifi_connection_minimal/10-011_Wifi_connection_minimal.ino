/*  10.011 - WiFi Network Connection (Minimal Version) for ESP32-S3
 * 
 * Course: IoT Development with ESP32-S3
 * Target Hardware: ESP32-S3-DevKitC-1 v1.1
 * Module: ESP32-S3-WROOM-1-N16R16V
 * 
 * This minimal sketch demonstrates basic WiFi connection on the ESP32-S3.
 * It connects to a WiFi network and displays the assigned IP address.
 * This version focuses on simplicity and core functionality for beginners.
 * 
 * HARDWARE INFORMATION:
 * =====================
 * Board: ESP32-S3-DevKitC-1 v1.1
 * Module: ESP32-S3-WROOM-1-N16R16V
 * Flash: 16 MB
 * PSRAM: 16 MB
 * WiFi: IEEE 802.11 b/g/n (2.4 GHz)
 * Bluetooth: Bluetooth LE 5.0
 * USB: USB-C connector with native USB support
 * 
 * CIRCUIT DIAGRAM:
 * ================
 * No external components required - WiFi is integrated into the ESP32-S3 module
 * 
 *    ESP32-S3-DevKitC-1
 *    ┌─────────────────┐
 *    │                 │
 *    │    ESP32-S3     │ <-- Integrated WiFi antenna
 *    │   WROOM-1       │
 *    │                 │
 *    │  [USB-C Port]   │ <-- Connect to computer for programming/serial
 *    └─────────────────┘
 * 
 * COMPONENTS:
 * ===========
 * - ESP32-S3-DevKitC-1 v1.1 development board
 * - USB-C cable for programming and serial communication
 * - WiFi network with known SSID and password
 * 
 * GPIO ASSIGNMENTS:
 * =================
 * No GPIO pins used - WiFi is handled internally by the ESP32-S3
 * Serial communication uses USB-C connector (native USB)
 * 
 * TECHNICAL NOTES:
 * ================
 * - ESP32-S3 supports both station (STA) and access point (AP) modes
 * - WiFi operates on 2.4 GHz band with 802.11 b/g/n protocols
 * - This minimal version uses basic connection without advanced error handling
 * - For production use, consider implementing timeout and retry mechanisms
 * 
 * LEARNING OBJECTIVES:
 * ====================
 * 1. Understand basic WiFi initialization on ESP32-S3
 * 2. Connect to a WiFi network using SSID and password
 * 3. Obtain and display network IP address
 * 4. Use Serial monitor for debugging WiFi connections
 * 
 * IDE REQUIREMENTS:
 * =================
 * - Arduino IDE 2.0+
 * - ESP32 Arduino Core v3.0+
 * - Board: "ESP32S3 Dev Module"
 * - USB CDC On Boot: "Enabled" (for serial communication)
 * 
 * Created: April 8 2019 by Peter Dalmaris
 * Updated: January 2025 for ESP32-S3 compatibility and modern practices
 */

#include <WiFi.h>

// ========================================
// CONFIGURATION CONSTANTS
// ========================================
const char* WIFI_SSID = "ardwifi";        // Change this to your network SSID
const char* WIFI_PASSWORD = "ardwifi987"; // Change this to your network password

// ========================================
// MAIN PROGRAM FUNCTIONS
// ========================================

void setup() {
    // Initialize serial communication for debugging
    Serial.begin(115200);
    delay(10);
    
    // Display connection attempt information
    Serial.println();
    Serial.println("ESP32-S3 WiFi Connection Demo");
    Serial.println("=============================");
    Serial.print("Connecting to: ");
    Serial.println(WIFI_SSID);
    
    // Start WiFi connection
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    
    // Wait for connection (with simple progress indicator)
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    
    // Connection successful - display results
    Serial.println("");
    Serial.println("WiFi connected successfully!");
    Serial.println("Network Information:");
    Serial.println("-------------------");
    Serial.print("SSID: ");
    Serial.println(WiFi.SSID());
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.print("Signal strength (RSSI): ");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm");
    
    Serial.println("\nConnection established. Ready for your application!");
}

void loop() {
    // Main loop - your application code goes here
    // WiFi connection is maintained automatically by the ESP32-S3
    
    // Optional: Simple connection status check
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("Warning: WiFi connection lost!");
        Serial.println("Attempting to reconnect...");
        WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
        
        // Wait for reconnection (simple approach)
        while (WiFi.status() != WL_CONNECTED) {
            delay(500);
            Serial.print(".");
        }
        Serial.println("\nReconnected successfully!");
    }
    
    delay(10000); // Check status every 10 seconds
}
