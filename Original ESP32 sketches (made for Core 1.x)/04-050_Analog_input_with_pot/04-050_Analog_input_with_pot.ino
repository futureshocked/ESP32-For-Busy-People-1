/*  04.050 - ESP32 Analog input with potentiometer 

   This sketch shows you how to read the state of a potentiometer using an ESP32.

   As you turn the potentiometer knob, see the measured value in the serial monitor.

   This sketch was written by Peter Dalmaris using information from the
   ESP32 datasheet and examples.


   Components
   ----------
    - ESP32 Dev Kit v4
    - 10 KOhm potentiometer

    IDE
    ---
    Arduino IDE with ESP32 Arduino Code
    (https://github.com/espressif/arduino-esp32)


    Libraries
    ---------
    - None
    -

   Connections
   -----------

    Refer to wiring diagram for a visual wiring guide

    ESP32 Dev Kit |     Potentiometer
    ------------------------------
          GND      |    Pin 1         
          GPIO36   |    Pin 2 (middle)
          GND      |    Pin 3


    Other information
    -----------------

    1. ESP32 Datasheet: https://www.espressif.com/sites/default/files/documentation/esp32_datasheet_en.pdf
    2. Espressif Docs: https://docs.espressif.com/projects/esp-idf/en/latest/api-reference/peripherals/adc.html#configuration-and-reading-adc
    
    Created on March 26 2019 by Peter Dalmaris

*/

const byte POT_GPIO = 36;

// the setup routine runs once when you press reset:
void setup() {
  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);
}

// the loop routine runs over and over again forever:
void loop() {
  // print out the value you read:
  Serial.println(analogRead(POT_GPIO));
}
