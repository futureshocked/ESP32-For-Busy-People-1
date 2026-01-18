/*  04.052 - ESP32-S3 Potentiometer + PWM LED Example

    This sketch reads an analog value from a potentiometer and maps it to a PWM
    signal that controls LED brightness.

    Board: ESP32-S3 DevKit (e.g. DevKitC-1 N16R8)

    Components
    ----------
    - 10K potentiometer
    - LED + 330Ω resistor

    Connections
    -----------
    ESP32-S3 DevKit | Component
    ---------------------------
    GND             | Pot pin 1
    GPIO5           | Pot pin 2 (middle)
    3.3V            | Pot pin 3
    GPIO4           | LED anode via resistor
    GND             | LED cathode

    Author: Peter Dalmaris
    Updated: July 2025 for ESP32-S3
*/

const byte POT_GPIO = 5;   // ADC1_CH4
const byte LED_GPIO = 4;   // PWM-capable GPIO

void setup() {
  ledcAttach(LED_GPIO, 4000, 12); // 12 kHz PWM, 12-bit resolution (0–4095)
}

void loop() {
  int adcValue = analogRead(POT_GPIO); // 0–4095
  ledcWrite(LED_GPIO, adcValue);       // Directly use ADC value as PWM duty
  delay(10); // Optional: smooth out ADC jitter
}
