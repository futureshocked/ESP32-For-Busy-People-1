/*  04.070 - GPIO interrupt with button and LED
 * 
 * This sketch shows you how to use an interrupt routine to respond to a
 * button press.
 * 
 * When the button is pressed, the LED state toggles.
 * 
 * This example includes software debouncing code.
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
 *  - Button (using an internal pull-up resistor)
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
 *  ESP32 Dev Kit |     LED
 *  ------------------------------
 *        GND      |    Cathode
 *        GPIO32   |    Anode via resistor
 *        
 *  Connect pin 1 of the button to ESP32 GPIO 25.
 *  Connect pin 2 of the button to ESP32 GND.
 *     
 *  It is possible to use any other GPIO that can be configured as an output.
 *  This excludes GPIO 34, 35, 36, 37, 38, 39.
 *  
 *  Other information
 *  -----------------
 *  
 *  1. ESP32 Datasheet: https://www.espressif.com/sites/default/files/documentation/esp32_datasheet_en.pdf
 *  2. Interrupt allocations (Doc): https://docs.espressif.com/projects/esp-idf/en/latest/api-reference/system/intr_alloc.html
 *  3. Technical reference (2. Interrupt Matrix): https://www.espressif.com/sites/default/files/documentation/esp32_technical_reference_manual_en.pdf
 *  4. ESP32-IDF portmacro.h: http://esp32.info/docs/esp_idf/html/dc/d35/portmacro_8h_source.html
 *  Created on March 26 2019 by Peter Dalmaris
 * 
 */

const byte LED_GPIO = 32;  // Marked volatile so it can be read inside the ISR
bool led_state = false;  // Keep track of the state of the LED

const byte interruptPin = 25;
volatile int interruptCounter = 0;
int numberOfInterrupts = 0;

// Debouncing parameters
long debouncing_time = 1000; //Debouncing Time in Milliseconds
volatile unsigned long last_micros;
 
portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
 
void IRAM_ATTR handleInterrupt() {
  portENTER_CRITICAL_ISR(&mux);
   if((long)(micros() - last_micros) >= debouncing_time * 1000) {
    interruptCounter++;
  }
  last_micros = micros();
  portEXIT_CRITICAL_ISR(&mux);
}
 
void setup() {
 
  Serial.begin(115200);
  pinMode(LED_GPIO, OUTPUT);
  Serial.println("Monitoring interrupts: ");
  pinMode(interruptPin, INPUT_PULLUP);  // Using an extarnal pull up instead of internal
  attachInterrupt(digitalPinToInterrupt(interruptPin), handleInterrupt, FALLING);
 
}
 
void loop() {
 
  if(interruptCounter>0){
 
      portENTER_CRITICAL(&mux);
      interruptCounter--;
      portEXIT_CRITICAL(&mux);

      led_state = !led_state;

      digitalWrite(LED_GPIO, led_state);   // turn the LED on (HIGH is the voltage level)
      
      numberOfInterrupts++;
      Serial.print("An interrupt has occurred. Total:");
      Serial.println(numberOfInterrupts);
  }
}
