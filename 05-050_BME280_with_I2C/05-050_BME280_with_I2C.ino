/*  05.050 - BME280 environment sensor using I2C
 * 
 * This sketch shows you how to read temperature, humidity and barometric pressure
 * values from the BME280 sensor. This sketch also show the calculated altitude.
 * 
 * After uploading the sketch to your ESP32, open the serial monitor to see the
 * values from the sensor.
 * 
 * This sketch was written by Peter Dalmaris using information from the 
 * ESP32 datasheet and the Adafruit example.
 * 
 * 
 * Components
 * ----------
 *  - ESP32 Dev Kit v4
 *  - BME280 sensor breakout
 *  
 *  IDE
 *  ---
 *  Arduino IDE with ESP32 Arduino Code 
 *  (https://github.com/espressif/arduino-esp32)
 *  
 *  
 *  Libraries
 *  ---------
 *  - Adafruit_Sensor
 *  - Adafruit_BME280
 *
 * Connections
 * -----------
 *  
 *  Connect the sensor breakout like this:
 *  
 *  ESP32   |    BME280
 *  -------------------
 *   3.3V   |    Vin   
 *   GND    |    GND
 *   GPIO22 |    SCL
 *   GPIO21 |    SDA
 *        
 *  
 *  Other information
 *  -----------------
 *  
 *  1. ESP32 Datasheet: https://www.espressif.com/sites/default/files/documentation/esp32_datasheet_en.pdf 
 *  2. Sensor product page: https://www.bosch-sensortec.com/bst/products/all_products/bme280
 *  3. Sensor datasheet: https://ae-bst.resource.bosch.com/media/_tech/media/datasheets/BST-BME280-DS002.pdf
 *  3. Library Adafruit_Sensor: https://github.com/adafruit/Adafruit_Sensor
 *  4. Library Adafruit_BME280: https://github.com/adafruit/Adafruit_BME280_Library
 *  
 *  Created on March 28 2019 by Peter Dalmaris
 *  
 */

/***************************************************************************
  This is a library for the BME280 humidity, temperature & pressure sensor

  Designed specifically to work with the Adafruit BME280 Breakout
  ----> http://www.adafruit.com/products/2650

  These sensors use I2C or SPI to communicate, 2 or 4 pins are required
  to interface. The device's I2C address is either 0x76 or 0x77.

  Adafruit invests time and resources providing this open source code,
  please support Adafruit andopen-source hardware by purchasing products
  from Adafruit!

  Written by Limor Fried & Kevin Townsend for Adafruit Industries.
  BSD license, all text above must be included in any redistribution
 ***************************************************************************/

#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#define SEALEVELPRESSURE_HPA (1013.25)

Adafruit_BME280 bme; // I2C

unsigned long delayTime;

void setup() {
    Serial.begin(9600);
    Serial.println(F("BME280 test"));

    bool status;
    
    // default settings
    status = bme.begin(0x76);  // BME280 sensors are usually set to address 0x76 or 0x77
                               // If your BME280 sensor module has an SD0 pin, then:
                               // SD0 unconnected configures the address to 0x77
                               // SD0 to GND configures the address to 0x76
                               // If there is no SD0 pin, try either address to find out which
                               // one works with your sensor.
    if (!status) {
        Serial.println("Could not find a valid BME280 sensor, check wiring!");
        while (1);
    }
    
    Serial.println("-- Default Test --");
    delayTime = 1000;

    Serial.println();
}


void loop() { 
    printValues();
    delay(delayTime);  // Blocking!!! 
}


void printValues() {
    Serial.print("Temperature = ");
    Serial.print(bme.readTemperature());
    Serial.println(" *C");

    Serial.print("Pressure = ");

    Serial.print(bme.readPressure() / 100.0F);
    Serial.println(" hPa");

    Serial.print("Approx. Altitude = ");
    Serial.print(bme.readAltitude(SEALEVELPRESSURE_HPA));
    Serial.println(" m");

    Serial.print("Humidity = ");
    Serial.print(bme.readHumidity());
    Serial.println(" %");

    Serial.println();
}
