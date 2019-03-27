/*  04.040 - ESP32 Digital input with button example

   This sketch shows you how to read the state of a button using an ESP32.

   When the button is pressed, the LED turns on.

   When the button is depressed, the LED turns off.

   This sketch was written by Peter Dalmaris using information from the
   ESP32 datasheet and examples.


   Components
   ----------
    - ESP32 Dev Kit v4
    - LED
    - 320 Ohm resistor
    - Button
    - 10 KOhm resistor

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

    ESP32 Dev Kit |     Component
    ------------------------------
          GND      |    LED Cathode
          GPIO32   |    LED Anode via 320 Ohm resistor
          GPIO36   |    Button read pin, plus 10 KOhm pull-down resistor to GND
          GND      |    Button GND pin


    It is possible to use any other GPIO that can be configured as an output or input.
    This excludes GPIO 34, 35, 36, 37, 38, 39 (these only work as inputs).

    Other information
    -----------------

    1. ESP32 Datasheet: https://www.espressif.com/sites/default/files/documentation/esp32_datasheet_en.pdf

    Created on March 26 2019 by Peter Dalmaris

*/


const byte LED_GPIO = 32;
const byte BUTTON_GPIO = 36;

// variables will change:
int buttonState = 0;         // variable for reading the pushbutton status

// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin LED_GPIO as an output.
  pinMode(LED_GPIO, OUTPUT);

  // initialize the pushbutton pin as an input:
  pinMode(BUTTON_GPIO, INPUT);
}

// the loop function runs over and over again forever
void loop() {
  // read the state of the pushbutton value:
  buttonState = digitalRead(BUTTON_GPIO);

  // check if the pushbutton is pressed. If it is, the buttonState is HIGH:
  if (buttonState == HIGH) {
    // turn LED on:
    digitalWrite(LED_GPIO, HIGH);
  } else {
    // turn LED off:
    digitalWrite(LED_GPIO, LOW);
  }
}
