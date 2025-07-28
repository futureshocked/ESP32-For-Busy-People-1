/*  08.021 - Non-Volatile Storage with EEPROM (Minimal) for ESP32-S3
 * 
 * Course: IoT Development with ESP32-S3
 * Target Hardware: ESP32-S3-DevKitC-1 v1.1
 * Module: ESP32-S3-WROOM-1-N16R16V
 * 
 * This minimal sketch demonstrates how to use the EEPROM library with the ESP32-S3
 * to store and retrieve a Boolean value representing the state of an LED. 
 * A button press toggles the LED, and the new state is written to non-volatile flash.
 * On reset or power cycle, the stored LED state is restored.
 * 
 * HARDWARE INFORMATION:
 * =====================
 * - Flash: 16 MB
 * - PSRAM: 16 MB
 * - ADC: 12-bit (GPIO1 to GPIO20)
 * - PWM: LEDC supported on all GPIOs
 * - GPIO Restrictions: Avoid GPIO35–37 (reserved for flash/PSRAM)
 * 
 * CIRCUIT DIAGRAM:
 * ================
 *    +3.3V o----+
 *               |
 *            [10KΩ]
 *               |
 *               +-------------o GPIO3 (BUTTON_PIN)
 *               |
 *              ===
 *              GND
 * 
 *         GPIO10 (LED_PIN)
 *              |
 *            [330Ω]
 *              |
 *             LED
 *              |
 *             GND
 * 
 * COMPONENTS:
 * ===========
 * - ESP32-S3-DevKitC-1 v1.1 board
 * - 1x LED
 * - 1x 330Ω resistor
 * - 1x Pushbutton
 * - 1x 10KΩ pull-up resistor (if not using internal pull-up)
 * 
 * GPIO ASSIGNMENTS:
 * =================
 * LED_PIN     → GPIO10
 * BUTTON_PIN  → GPIO3
 * 
 * TECHNICAL NOTES:
 * ================
 * - EEPROM emulation on ESP32 allows storing small data blocks in flash.
 * - EEPROM.begin(size) must be called before use.
 * - EEPROM.commit() is required after write to store changes.
 * - GPIO3 and GPIO10 are safe for input and output respectively on ESP32-S3.
 * - Software debounce (~1000ms) is implemented inside the ISR for simplicity.
 * 
 * IDE REQUIREMENTS:
 * =================
 * - Arduino IDE 2.0+
 * - ESP32 Arduino Core v3.0+
 * - Board: "ESP32S3 Dev Module"
 * 
 * Created: April 2, 2019 by Peter Dalmaris
 * Updated: July 28, 2025 for ESP32-S3 compatibility
 */

#include <EEPROM.h>

// === CONFIGURATION CONSTANTS ===
constexpr uint8_t LED_PIN = 10;
constexpr uint8_t BUTTON_PIN = 3;
constexpr int EEPROM_SIZE = 1;
constexpr int EEPROM_ADDR = 0;

// === GLOBAL STATE VARIABLES ===
volatile bool ledState = false;
volatile bool buttonPressed = false;
portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
volatile unsigned long lastMicros = 0;

// === INTERRUPT SERVICE ROUTINE ===
void IRAM_ATTR onButton() {
    unsigned long now = micros();
    if ((now - lastMicros) >= 1000000) { // 1000 ms debounce
        portENTER_CRITICAL_ISR(&mux);
        buttonPressed = true;
        lastMicros = now;
        portEXIT_CRITICAL_ISR(&mux);
    }
}

// === SETUP FUNCTION ===
void setup() {
    Serial.begin(115200);
    EEPROM.begin(EEPROM_SIZE);

    ledState = EEPROM.readBool(EEPROM_ADDR);
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, ledState);

    pinMode(BUTTON_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), onButton, FALLING);

    Serial.printf("Startup LED state: %s\n", ledState ? "ON" : "OFF");
}

// === MAIN LOOP ===
void loop() {
    if (buttonPressed) {
        portENTER_CRITICAL(&mux);
        buttonPressed = false;
        portEXIT_CRITICAL(&mux);

        ledState = !ledState;
        digitalWrite(LED_PIN, ledState);
        EEPROM.writeBool(EEPROM_ADDR, ledState);
        EEPROM.commit();

        Serial.printf("New LED state stored: %s\n", ledState ? "ON" : "OFF");
    }
}
