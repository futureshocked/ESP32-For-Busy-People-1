/*  10.011 - WiFi Network Connection with Enhanced Monitoring for ESP32-S3
 * 
 * Course: IoT Development with ESP32-S3
 * Target Hardware: ESP32-S3-DevKitC-1 v1.1
 * Module: ESP32-S3-WROOM-1-N16R16V
 * 
 * This comprehensive sketch demonstrates WiFi connection establishment on the ESP32-S3
 * with enhanced error handling, connection monitoring, network diagnostics, and 
 * educational features. It provides detailed feedback about the connection process
 * and includes utilities for network troubleshooting.
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
 * - Connection timeout and retry mechanisms prevent infinite blocking
 * - Network diagnostics help troubleshoot connection issues
 * - Power management options available for battery-powered applications
 * 
 * LEARNING OBJECTIVES:
 * ====================
 * 1. Understand WiFi initialization on ESP32-S3
 * 2. Implement robust connection handling with error recovery
 * 3. Monitor network status and connection quality
 * 4. Perform network diagnostics and troubleshooting
 * 5. Apply modern C++ practices in embedded systems
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
const char* WIFI_SSID = "__WIFI_SSID__";           // Change this to your network SSID
const char* WIFI_PASSWORD = "__WIFI_PASSWORD__";    // Change this to your network password

// Connection parameters
const uint32_t WIFI_TIMEOUT_MS = 20000;      // WiFi connection timeout (20 seconds)
const uint32_t RETRY_DELAY_MS = 500;         // Delay between connection attempts
const uint8_t MAX_RETRY_ATTEMPTS = 3;        // Maximum connection retry attempts
const uint32_t STATUS_CHECK_INTERVAL = 10000; // Check connection status every 10 seconds

// Serial communication
const uint32_t SERIAL_BAUD_RATE = 115200;    // Serial monitor baud rate

// ========================================
// GLOBAL VARIABLES
// ========================================
uint32_t lastStatusCheck = 0;                // Last time we checked WiFi status
uint8_t connectionAttempts = 0;               // Current connection attempt counter
bool isConnected = false;                     // Connection status flag

// ========================================
// UTILITY FUNCTIONS
// ========================================

/**
 * Print a formatted section separator for better readability
 * @param title Section title to display
 */
void printSectionSeparator(const String& title) {
    Serial.println("\n" + String('=', 50));
    Serial.println("  " + title);
    Serial.println(String('=', 50));
}

/**
 * Print detailed WiFi network information
 */
void printNetworkInfo() {
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nNetwork Information:");
        Serial.println("--------------------");
        Serial.printf("SSID: %s\n", WiFi.SSID().c_str());
        Serial.printf("IP Address: %s\n", WiFi.localIP().toString().c_str());
        Serial.printf("Subnet Mask: %s\n", WiFi.subnetMask().toString().c_str());
        Serial.printf("Gateway: %s\n", WiFi.gatewayIP().toString().c_str());
        Serial.printf("DNS Server: %s\n", WiFi.dnsIP().toString().c_str());
        Serial.printf("MAC Address: %s\n", WiFi.macAddress().c_str());
        Serial.printf("Signal Strength (RSSI): %d dBm\n", WiFi.RSSI());
        Serial.printf("Channel: %d\n", WiFi.channel());
        
        // Signal quality assessment
        int32_t rssi = WiFi.RSSI();
        String signalQuality;
        if (rssi > -50) signalQuality = "Excellent";
        else if (rssi > -60) signalQuality = "Good";
        else if (rssi > -70) signalQuality = "Fair";
        else signalQuality = "Poor";
        
        Serial.printf("Signal Quality: %s\n", signalQuality.c_str());
    }
}

/**
 * Convert WiFi status code to human-readable string
 * @param status WiFi status code
 * @return String description of the status
 */
String getWiFiStatus(wl_status_t status) {
    switch(status) {
        case WL_IDLE_STATUS:     return "Idle";
        case WL_NO_SSID_AVAIL:   return "No SSID Available";
        case WL_SCAN_COMPLETED:  return "Scan Completed";
        case WL_CONNECTED:       return "Connected";
        case WL_CONNECT_FAILED:  return "Connection Failed";
        case WL_CONNECTION_LOST: return "Connection Lost";
        case WL_DISCONNECTED:    return "Disconnected";
        default:                 return "Unknown Status";
    }
}

/**
 * Perform WiFi connection with enhanced error handling
 * @return true if connection successful, false otherwise
 */
bool connectToWiFi() {
    printSectionSeparator("WiFi Connection Process");
    
    connectionAttempts++;
    Serial.printf("Connection attempt %d of %d\n", connectionAttempts, MAX_RETRY_ATTEMPTS);
    Serial.printf("Connecting to network: %s\n", WIFI_SSID);
    
    // Initialize WiFi in station mode
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    
    uint32_t startTime = millis();
    
    // Wait for connection with timeout
    while (WiFi.status() != WL_CONNECTED && (millis() - startTime) < WIFI_TIMEOUT_MS) {
        delay(RETRY_DELAY_MS);
        Serial.print(".");
        
        // Print status update every 5 seconds
        if ((millis() - startTime) % 5000 < RETRY_DELAY_MS) {
            Serial.printf("\nStatus: %s (%.1fs elapsed)\n", 
                         getWiFiStatus(WiFi.status()).c_str(), 
                         (millis() - startTime) / 1000.0);
        }
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\n\nWiFi connection successful!");
        printNetworkInfo();
        isConnected = true;
        return true;
    } else {
        Serial.printf("\nConnection failed: %s\n", getWiFiStatus(WiFi.status()).c_str());
        Serial.printf("Time elapsed: %.1f seconds\n", (millis() - startTime) / 1000.0);
        return false;
    }
}

/**
 * Monitor WiFi connection status and attempt reconnection if needed
 */
void monitorConnection() {
    if (millis() - lastStatusCheck >= STATUS_CHECK_INTERVAL) {
        lastStatusCheck = millis();
        
        if (WiFi.status() != WL_CONNECTED && isConnected) {
            Serial.println("\nWarning: WiFi connection lost!");
            Serial.printf("Current status: %s\n", getWiFiStatus(WiFi.status()).c_str());
            isConnected = false;
            
            // Attempt to reconnect
            if (connectionAttempts < MAX_RETRY_ATTEMPTS) {
                Serial.println("Attempting to reconnect...");
                connectToWiFi();
            } else {
                Serial.println("Maximum retry attempts reached. Please check network settings.");
            }
        } else if (WiFi.status() == WL_CONNECTED && isConnected) {
            // Periodic status update for connected state
            Serial.printf("WiFi Status: Connected | RSSI: %d dBm | Uptime: %.1f minutes\n", 
                         WiFi.RSSI(), millis() / 60000.0);
        }
    }
}

/**
 * Display system information and diagnostics
 */
void printSystemInfo() {
    printSectionSeparator("ESP32-S3 System Information");
    
    Serial.printf("Chip Model: %s\n", ESP.getChipModel());
    Serial.printf("Chip Revision: %d\n", ESP.getChipRevision());
    Serial.printf("CPU Frequency: %d MHz\n", ESP.getCpuFreqMHz());
    Serial.printf("Flash Size: %d bytes (%.1f MB)\n", ESP.getFlashChipSize(), ESP.getFlashChipSize() / (1024.0 * 1024.0));
    Serial.printf("Free Heap: %d bytes (%.1f KB)\n", ESP.getFreeHeap(), ESP.getFreeHeap() / 1024.0);
    Serial.printf("PSRAM Size: %d bytes (%.1f MB)\n", ESP.getPsramSize(), ESP.getPsramSize() / (1024.0 * 1024.0));
    
    // WiFi MAC address
    Serial.printf("WiFi MAC Address: %s\n", WiFi.macAddress().c_str());
    
    Serial.println("\nESP32 Arduino Core Information:");
    Serial.printf("ESP-IDF Version: %s\n", ESP.getSdkVersion());
}

// ========================================
// MAIN PROGRAM FUNCTIONS
// ========================================

void setup() {
    // Initialize serial communication
    Serial.begin(SERIAL_BAUD_RATE);
    delay(100); // Allow serial to stabilize
    
    // Clear serial monitor and display header
    Serial.println("\n\n\n");
    printSectionSeparator("ESP32-S3 WiFi Connection Demo");
    Serial.println("Lesson 10.011 - IoT Development with ESP32-S3");
    Serial.println("Target: ESP32-S3-DevKitC-1 v1.1");
    
    // Display system information
    printSystemInfo();
    
    // Attempt initial WiFi connection
    connectionAttempts = 0; // Reset counter
    while (!isConnected && connectionAttempts < MAX_RETRY_ATTEMPTS) {
        if (!connectToWiFi()) {
            if (connectionAttempts < MAX_RETRY_ATTEMPTS) {
                Serial.printf("Retrying in 3 seconds... (Attempt %d/%d)\n", 
                             connectionAttempts + 1, MAX_RETRY_ATTEMPTS);
                delay(3000);
            }
        }
    }
    
    if (!isConnected) {
        Serial.println("\nTroubleshooting Tips:");
        Serial.println("1. Verify SSID and password are correct");
        Serial.println("2. Check if network is 2.4GHz (ESP32-S3 doesn't support 5GHz)");
        Serial.println("3. Ensure network is broadcasting SSID");
        Serial.println("4. Check WiFi signal strength");
        Serial.println("5. Try moving closer to the WiFi router");
        Serial.println("\nThe program will continue monitoring for connection...");
    }
    
    Serial.println("\nSetup complete. Monitoring connection status...");
}

void loop() {
    // Monitor WiFi connection status
    monitorConnection();
    
    // Small delay to prevent excessive CPU usage
    delay(100);
    
    // Optional: Add your main application code here
    // The WiFi connection will be maintained automatically
}
