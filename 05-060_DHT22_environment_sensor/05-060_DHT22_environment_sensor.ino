/*  05.060 - DHT22 environment sensor 

   This sketch shows you how to use the DHT22 digital temperature and
   humidity sensor with the ESP32.   

   After uploading the sketch to your ESP32, open the serial monitor to see the
   values from the sensor.

   This sketch was written by Peter Dalmaris using information from the
   ESP32 datasheet and the library examples.


   Components
   ----------
    - ESP32 Dev Kit v4
    - DHT22 (AM2302) 
    - 10 KOhm resistor (for pull-up)

    IDE
    ---
    Arduino IDE with ESP32 Arduino Code
    (https://github.com/espressif/arduino-esp32)


    Libraries
    ---------
    - SimpleDHT
    

   Connections
   -----------

    Connect the sensor breakout like this:

    ESP32   |    DHT22
    -------------------
     3.3V   |    Pin 1
     GPIO32 |    Pin 2
     -      |    Pin 3
     GND    |    Pin 4
     
    Connect a 10 K Ohm resistor between Pin 2 of the sensor and 3.3V on the ESP32 board.

    Other information
    -----------------

    1. ESP32 Datasheet: https://www.espressif.com/sites/default/files/documentation/esp32_datasheet_en.pdf
    2. Library: https://github.com/winlinvip/SimpleDHT
    3. Library error codes: https://github.com/winlinvip/SimpleDHT/blob/master/SimpleDHT.h
    3. Sensor datasheet: https://www.mouser.com/ds/2/737/dht-932870.pdf
    
    Created on March 28 2019 by Peter Dalmaris

*/

#include <SimpleDHT.h>

int pinDHT22 = 32; 

SimpleDHT22 dht22(pinDHT22);

void setup() {
  Serial.begin(115200);
}

void loop() {
  // start working...
  Serial.println("=================================");
  Serial.println("Sample DHT22...");
  
  // read without samples.
  // @remark We use read2 to get a float data, such as 10.1*C
  //    if user doesn't care about the accurate data, use read to get a byte data, such as 10*C.
  float temperature = 0;
  float humidity = 0;
  int err = SimpleDHTErrSuccess;
  if ((err = dht22.read2(&temperature, &humidity, NULL)) != SimpleDHTErrSuccess) {
    Serial.print("Read DHT22 failed, err="); Serial.println(err);delay(2000);
    return;
  }
  
  Serial.print("Sample OK: ");
  Serial.print((float)temperature); Serial.print(" *C, ");
  Serial.print((float)humidity); Serial.println(" RH%");
  
  // DHT22 sampling rate is 0.5HZ.
  delay(2500);
}
