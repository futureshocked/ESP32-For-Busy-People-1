/*  04.051 - ESP32-S3 Analog Input with Potentiometer

    This sketch reads the value of a potentiometer connected to an ADC-capable pin.

    As you turn the potentiometer, the value printed to the Serial Monitor will change
    between 0 and 4095 (12-bit resolution).

    Board: ESP32-S3 DevKit (e.g. DevKitC-1 N16R8)

    Components
    ----------
    - 10kΩ potentiometer

    Connections
    -----------
    ESP32-S3 DevKit | Potentiometer
    ------------------------------
    GND             | Pin 1 (outer)
    GPIO4           | Pin 2 (middle)
    3.3V            | Pin 3 (outer)

    Author: Peter Dalmaris
    Updated: July 2025 for ESP32-S3
*/

const byte POT_GPIO = 4; // GPIO4 is ADC1_CH3 on ESP32-S3

void setup() {
  Serial.begin(9600);
}

void loop() {
  int potValue = analogRead(POT_GPIO);
  Serial.println(potValue);
  delay(200); // Limit update rate for readability
}
