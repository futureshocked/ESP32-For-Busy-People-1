/*  04.041 - ESP32-S3 Digital Input with Button Example

    This sketch demonstrates reading a pushbutton input and driving an LED.

    When the button is pressed, the LED turns on.
    When released, the LED turns off.

    Board: ESP32-S3 DevKit (e.g. DevKitC-1 N16R8)

    Components
    ----------
    - LED + resistor (e.g. 330Ω)
    - Pushbutton + 10kΩ pull-down resistor (or use internal pull-down)

    Connections
    -----------
    ESP32-S3 DevKit | Component
    --------------------------
    GND             | LED cathode
    GPIO4           | LED anode via resistor
    GND             | Pushbutton ground pin
    GPIO5           | Pushbutton signal pin (with 10kΩ pull-down to GND)

    Author: Peter Dalmaris
    Updated: July 2025 for ESP32-S3
*/

const byte LED_GPIO = 4;       // output LED pin
const byte BUTTON_GPIO = 5;    // input button pin

int buttonState = 0;

void setup() {
  pinMode(LED_GPIO, OUTPUT);

  // Use INPUT_PULLDOWN if external pull-down is not used
  pinMode(BUTTON_GPIO, INPUT_PULLDOWN);
}

void loop() {
  buttonState = digitalRead(BUTTON_GPIO);

  if (buttonState == HIGH) {
    digitalWrite(LED_GPIO, HIGH);
  } else {
    digitalWrite(LED_GPIO, LOW);
  }
}
