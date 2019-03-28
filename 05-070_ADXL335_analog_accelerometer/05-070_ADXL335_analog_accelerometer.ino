/*  05.070 - ADXL335 acceleration sensor 

   This sketch shows you how to use the ADXL335 analog acceleration sensor to
   detect the orientation of your gadget.

   After uploading the sketch to your ESP32, open the serial monitor to see the
   values from the sensor.

   This sketch was written by Peter Dalmaris using information from the
   ESP32 datasheet.


   Components
   ----------
    - ESP32 Dev Kit v4
    - ADXL335sensor breakout

    IDE
    ---
    Arduino IDE with ESP32 Arduino Code
    (https://github.com/espressif/arduino-esp32)


    Libraries
    ---------
    - 
    

   Connections
   -----------

    Connect the sensor breakout like this:

    ESP32   |    ADXL335
    -------------------
     3.3V   |    Vcc
     GND    |    GND
     GPIO34 |    X
     GPIO39 |    Y
     GPIO36 |    Z


    Other information
    -----------------

    1. ESP32 Datasheet: https://www.espressif.com/sites/default/files/documentation/esp32_datasheet_en.pdf
    2. Sensor product page: https://www.analog.com/en/products/adxl335.html
    3. Sensor datasheet: https://www.analog.com/media/en/technical-documentation/data-sheets/adxl335.pdf
    
    Created on March 28 2019 by Peter Dalmaris

*/

const byte xpin = 34;                  // x-axis of the accelerometer
const byte ypin = 39;                  // y-axis
const byte zpin = 36;                  // z-axis (only on 3-axis models)

void setup() {
  // initialize the serial communications:
  Serial.begin(9600);

  analogSetWidth(9);  // Default resolution is 12 bits. Choose between 9-12.
                      // 9-bit resolution will produce readings from 0 to 512
}

void loop() {
  // print the sensor values:
  Serial.print(analogRead(xpin));
  // print a tab between values:
  Serial.print("\t");
  Serial.print(analogRead(ypin));
  // print a tab between values:
  Serial.print("\t");
  Serial.print(analogRead(zpin));

  
  // The folowing code shows how to determine the orientation of the sensor relative to the table.
  // The values are determined experimentally.
  if (analogRead(zpin) > 270)
  {
    Serial.print("\t");
    Serial.print("Gadget is horizontal to table.");
  }

  if (analogRead(xpin) < 190)
  {
    Serial.print("\t");
    Serial.print("Gadget front to table.");
  }

  if ((analogRead(ypin) < 280 && analogRead(ypin) > 270) || 
      (analogRead(ypin) < 180 && analogRead(ypin) > 170) )
  {
    Serial.print("\t");
    Serial.print("Gadget is on its side.");
  }
  
  Serial.println();
  // delay before next reading:
  delay(100);
}
