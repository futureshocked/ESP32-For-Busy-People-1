/*  04.052 - ESP32 Digital in with potentiometer and PWM LED 
 * 
 * This sketch shows you how to read the value of a potentiometer and
 * then drive an LED using PWM output.
 * 
 * This sketch was written by Peter Dalmaris using information from the 
 * ESP32 datasheet and examples.
 * 
 * 
 * Components
 * ----------
 *  - ESP32 Dev Kit v4
 *  - LED
 *  - 320 Ohm resistor
 *  - 10 KOhm potentiometer
 *  
 *  IDE
 *  ---
 *  Arduino IDE with ESP32 Arduino Code 
 *  (https://github.com/espressif/arduino-esp32)
 *  
 *  
 *  Libraries
 *  ---------
 *  - None
 *  - 
 *
 * Connections
 * -----------
 *  
 *  ESP32 Dev Kit  |     Component
 *  ------------------------------
 *        GND      |      LED Cathode
 *        GPIO32   |      LED Anode via resistor
 *        GND      |      Pot Pin 2
 *        GPIO36   |      Pot middle pin
 *        3.3V     |      Pot Pin 1
 *     
 * 
 *  
 *  It is possible to use any other PWM-capable GPIO.
 *  
 *  Other information
 *  -----------------
 *  
 *  1. ledc.h source code: https://github.com/espressif/arduino-esp32/blob/master/cores/esp32/esp32-hal-ledc.h
 *  2. ESP32 Datasheet: https://www.espressif.com/sites/default/files/documentation/esp32_datasheet_en.pdf
 *  
 *  Created on March 27 2019 by Peter Dalmaris
 * 
 */

const byte POT_GPIO = 36; // Potentiometer
const byte led_gpio = 32; // the PWM pin the LED is attached to

// the setup routine runs once when you press reset:
void setup() {
  ledcAttachPin(led_gpio, 0); // assign a led pins to a channel

  // Initialize channels
  // channels 0-15, resolution 1-16 bits, freq limits depend on resolution
  // ledcSetup(uint8_t channel, uint32_t freq, uint8_t resolution_bits);
  ledcSetup(0, 4000, 12); // 12 kHz PWM, 8-bit resolution 
}

// the loop routine runs over and over again forever:
void loop() {
  ledcWrite(0, analogRead(POT_GPIO)); // set the brightness of the LED

}
