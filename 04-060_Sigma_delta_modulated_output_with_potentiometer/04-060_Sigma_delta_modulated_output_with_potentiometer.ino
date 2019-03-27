/*  04.060 - Sigma Delta modulation with potentiometer example
 * 
 * This sketch shows you how to create a sigma-delta modulated analog output using a potentiometer.
 * 
 * Connect your LED to GPIO 25.
 * 
 * Run the sketch.
 * 
 * Turn the knob of the potentiometer, and see how the intensity of the LED changes.
 * 
 * 
 * This sketch was written by Peter Dalmaris using information from the 
 * ESP32 datasheet and examples.
 * 
 * 
 * Components
 * ----------
 *  - ESP32 Dev Kit v4
 *  - Potentiometer, 10 KOhm
 *  - LED with a 330 Ohm resistor
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
 *  Potentiometer: Pin 1 to 3.3V, Pin 2 to GPIO36, Pin 3 to GND
 *  LED: Anode to GPIO32, cathode to 330 Ohm resistor, resistor to GND
 *  
 *  Other information
 *  -----------------
 *  
 *  1. ESP32 Datasheet: https://www.espressif.com/sites/default/files/documentation/esp32_datasheet_en.pdf
 *  2. Learn about Sigma Delta modulation: https://en.wikipedia.org/wiki/Delta-sigma_modulation
 *  3. Sigma Delta modulation in the ESP32 (Docs): https://docs.espressif.com/projects/esp-idf/en/latest/api-reference/peripherals/sigmadelta.html?highlight=sigmaDelta
 *  4. Wikibooks: https://en.wikibooks.org/wiki/Digital_Signal_Processing/Sigma-Delta_modulation
 *  5. Electronic Design: https://www.electronicdesign.com/analog/understanding-delta-sigma-modulators
 *  6. Sigma-delta header file: https://github.com/espressif/arduino-esp32/blob/master/cores/esp32/esp32-hal-sigmadelta.h
 *  
 *  Created on March 26 2019 by Peter Dalmaris
 * 
 */

const byte POT_GPIO = 36;
const byte LED_GPIO = 32;

void setup()
{
    //setup channel 0 with frequency 312500 Hz
    sigmaDeltaSetup(0, 312500);
    //attach pin 32 to channel 0
    sigmaDeltaAttachPin(LED_GPIO,0);
    //initialize channel 0 to off
    sigmaDeltaWrite(0, 0);
}

void loop()
{
    int pot_value = analogRead(POT_GPIO);  
    sigmaDeltaWrite(0, map(pot_value, 0, 4095, 0, 255));
    delay(10);
}
