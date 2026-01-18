/*  08.031 - Reading and Writing from SPIFFS for ESP32-S3
 * 
 * Course: IoT Development with ESP32-S3
 * Target Hardware: ESP32-S3-DevKitC-1 v1.1
 * Module: ESP32-S3-WROOM-1-N16R16V
 * 
 * This sketch demonstrates how to mount the SPIFFS file system and perform 
 * basic file operations: read, write, and append. Students will learn how to:
 * - Initialize and check SPIFFS
 * - Write and append to files
 * - Read content from files
 * - Upload initial files using the Arduino IDE Data Upload tool
 * 
 * HARDWARE INFORMATION:
 * =====================
 * - 16 MB Flash with SPIFFS support
 * - No special peripherals required
 * 
 * CIRCUIT DIAGRAM:
 * ================
 * (No physical circuit required for this sketch)
 * 
 * COMPONENTS:
 * ===========
 * - ESP32-S3-DevKitC-1 v1.1
 * - USB-C cable
 * 
 * GPIO ASSIGNMENTS:
 * =================
 * None used in this lesson
 * 
 * TECHNICAL NOTES:
 * ================
 * - Use the "ESP32 Sketch Data Upload" tool to upload initial files to SPIFFS.
 * - Format SPIFFS if first run or on mount failure.
 * - File system must be defined in `partitions.csv` or via Tools > Partition Scheme
 * - Default SPIFFS mount point is root ("/")
 * 
 * IDE REQUIREMENTS:
 * =================
 * - Arduino IDE 2.0+
 * - ESP32 Arduino Core v3.0+
 * - Board: "ESP32S3 Dev Module"
 * 
 * Created: April 3, 2019 by Peter Dalmaris
 * Updated: July 28, 2025 for ESP32-S3 compatibility
 */

#include <SPIFFS.h>
#include <FS.h>

// Constants
constexpr bool FORMAT_SPIFFS_IF_FAILED = true;
constexpr char README_FILE[] = "/readme.txt";
constexpr char WRITE_FILE[] = "/writetest.txt";

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("\n[SPIFFS FILE SYSTEM DEMO]");
    
    if (!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED)) {
        Serial.println("❌ SPIFFS Mount Failed. Please check partition scheme.");
        return;
    }

    // Read a preloaded file
    readFile(SPIFFS, README_FILE);
    delay(2000);

    // Create or overwrite a file
    writeFile(SPIFFS, WRITE_FILE, "Hello ");
    delay(2000);

    // Read back
    readFile(SPIFFS, WRITE_FILE);
    delay(2000);

    // Append more data
    appendFile(SPIFFS, WRITE_FILE, "World!\r\n");
    delay(2000);

    // Final read
    readFile(SPIFFS, WRITE_FILE);
    Serial.println("\n✅ Demo finished.");
}

void loop() {
    // Nothing here for this demo
}

// Read a file from SPIFFS
void readFile(fs::FS &fs, const char * path) {
    Serial.printf("\n🔍 Reading file: %s\n", path);
    File file = fs.open(path);
    if (!file || file.isDirectory()) {
        Serial.println("⚠️ Failed to open file for reading.");
        return;
    }

    Serial.println("📄 File content:");
    while (file.available()) {
        Serial.write(file.read());
    }
    file.close();
}

// Write (overwrite) a file
void writeFile(fs::FS &fs, const char * path, const char * message) {
    Serial.printf("\n✍️ Writing file: %s\n", path);
    File file = fs.open(path, FILE_WRITE);
    if (!file) {
        Serial.println("⚠️ Failed to open file for writing.");
        return;
    }

    if (file.print(message)) {
        Serial.println("✅ File written.");
    } else {
        Serial.println("❌ Write failed.");
    }
    file.close();
}

// Append to an existing file
void appendFile(fs::FS &fs, const char * path, const char * message) {
    Serial.printf("\n➕ Appending to file: %s\n", path);
    File file = fs.open(path, FILE_APPEND);
    if (!file) {
        Serial.println("⚠️ Failed to open file for appending.");
        return;
    }

    if (file.print(message)) {
        Serial.println("✅ Message appended.");
    } else {
        Serial.println("❌ Append failed.");
    }
    file.close();
}
