/*  08.030 - Reading and writting from the SPIFFS
 * 
 * This sketch demonstrates how to read, write and append text to files stored in
 * the SPIFFS of an ESP32.
 * 
 * Before running the sketch, create a text file with the file name readme.txt and 
 * any text content you like (keep it small!). Go in the sketch directory and create 
 * a new folder named "data". Store the text file in the data directory. Use the 
 * SPIFFS uploaded tool from the Arduino IDE to upload the file to the SPIFFS. Then run
 * the sketch. 
 * 
 * The sketch will demonstrate the 3 basic functionalities for the SPIFFS library.
 * 
 * This sketch was written by Peter Dalmaris using information from the 
 * ESP32 datasheet and examples.
 * 
 * 
 * Components
 * ----------
 *  - ESP32 Dev Kit v4
 *  
 *  IDE
 *  ---
 *  Arduino IDE with ESP32 Arduino Code 
 *  (https://github.com/espressif/arduino-esp32)
 *  
 *  
 *  Libraries
 *  ---------
 *  - FS
 *  - SPIFFS
 *
 * Connections
 * -----------
 *  
 *  
 *  Other information
 *  -----------------
 *  
 *  1. ESP32 Datasheet: https://www.espressif.com/sites/default/files/documentation/esp32_datasheet_en.pdf
 *  2. FS library: https://github.com/espressif/arduino-esp32/tree/master/libraries/FS
 *  3. SPIFFS library: https://github.com/espressif/arduino-esp32/tree/master/libraries/SPIFFS
 *  4. ESP32 sketch data upload tool: https://github.com/me-no-dev/arduino-esp32fs-plugin/releases/
 *  5. ESP32 FS tool (useful for many things, including erasing flash memory): https://github.com/espressif/esptool
 *  
 *  
 *  Created on April 3 2019 by Peter Dalmaris
 * 
 */

#include "FS.h"
#include "SPIFFS.h"

/* You only need to format SPIFFS the first time you run a
   test or else use the SPIFFS plugin to create a partition
   https://github.com/me-no-dev/arduino-esp32fs-plugin */
#define FORMAT_SPIFFS_IF_FAILED true


void setup() {
    Serial.begin(115200);
    if(!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED)){
        Serial.println("SPIFFS Mount Failed");
        return;
    } 
    
    readFile(SPIFFS, "/readme.txt");
    Serial.println();
    delay(2000);

    writeFile(SPIFFS, "/writetest.txt", "Hello ");
    Serial.println();
    delay(2000);

    readFile(SPIFFS, "/writetest.txt");
    Serial.println();
    delay(2000);

    appendFile(SPIFFS, "/writetest.txt", "World!\r\n");
    Serial.println();
    delay(2000);

    readFile(SPIFFS, "/writetest.txt");
    Serial.println();
    delay(2000);

    Serial.println("Demo finished.");
    
}

void loop() {
  // put your main code here, to run repeatedly:
  
}

void readFile(fs::FS &fs, const char * path){
    Serial.printf("Reading file: %s\r\n", path);

    File file = fs.open(path);
    if(!file || file.isDirectory()){
        Serial.println("- failed to open file for reading");
        return;
    }

    Serial.println("- read from file:");
    while(file.available()){
        Serial.write(file.read());
    }
}

void writeFile(fs::FS &fs, const char * path, const char * message){
    Serial.printf("Writing file: %s\r\n", path);

    File file = fs.open(path, FILE_WRITE);
    if(!file){
        Serial.println("- failed to open file for writing");
        return;
    }
    if(file.print(message)){
        Serial.println("- file written");
    } else {
        Serial.println("- frite failed");
    }
}

void appendFile(fs::FS &fs, const char * path, const char * message){
    Serial.printf("Appending to file: %s\r\n", path);

    File file = fs.open(path, FILE_APPEND);
    if(!file){
        Serial.println("- failed to open file for appending");
        return;
    }
    if(file.print(message)){
        Serial.println("- message appended");
    } else {
        Serial.println("- append failed");
    }
}
