/*  08.021 - Non-Volatile Storage with EEPROM for ESP32-S3
 * 
 * Course: IoT Development with ESP32-S3
 * Target Hardware: ESP32-S3-DevKitC-1 v1.1
 * Module: ESP32-S3-WROOM-1-N16R16V
 * 
 * This sketch demonstrates how to use non-volatile memory (EEPROM emulation) 
 * on the ESP32-S3 to retain the state of an LED across resets or power loss.
 * 
 * A button toggles the LED on each press. The new state is saved to flash memory
 * via the EEPROM library and restored at startup.
 * 
 * A software debounce and interrupt are used for reliable button presses.
 * 
 * HARDWARE INFORMATION:
 * =====================
 * - Flash: 16 MB
 * - PSRAM: 16 MB
 * - GPIO Input range: GPIO1–GPIO21 preferred
 * - Avoid GPIO35–GPIO37 (reserved)
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
 * - 1x ESP32-S3-DevKitC-1 v1.1
 * - 1x LED
 * - 1x 330Ω resistor
 * - 1x Pushbutton
 * - 1x 10KΩ pull-up resistor (if not using internal pull-up)
 * 
 * GPIO ASSIGNMENTS:
 * =================
 * LED_PIN     → GPIO10 (any safe GPIO output)
 * BUTTON_PIN  → GPIO3  (safe GPIO input with pull-up)
 * 
 * TECHNICAL NOTES:
 * ================
 * - EEPROM is emulated in flash (up to 512 bytes).
 * - EEPROM.begin() must be called before using.
 * - EEPROM.commit() is required after writing.
 * - Interrupts use IRAM_ATTR and critical sections for atomic operations.
 * - Software debounce is used inside the ISR.
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
constexpr unsigned long DEBOUNCE_MS = 1000;

// === GLOBAL VARIABLES ===
volatile bool ledState = false;
volatile bool buttonPressed = false;
portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
volatile unsigned long lastInterruptTime = 0;

// === INTERRUPT SERVICE ROUTINE ===
void IRAM_ATTR onButtonPress() {
    unsigned long now = micros();
    if ((now - lastInterruptTime) >= DEBOUNCE_MS * 1000) {
        portENTER_CRITICAL_ISR(&mux);
        buttonPressed = true;
        lastInterruptTime = now;
        portEXIT_CRITICAL_ISR(&mux);
    }
}

// === SETUP ===
void setup() {
    Serial.begin(115200);

    // Initialize EEPROM
    if (!EEPROM.begin(EEPROM_SIZE)) {
        Serial.println("EEPROM initialization failed!");
        while (true); // Halt system
    }

    // Restore last LED state from EEPROM
    ledState = EEPROM.readBool(EEPROM_ADDR);
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, ledState);
    Serial.printf("Restored LED state: %s\n", ledState ? "ON" : "OFF");

    // Set up button
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), onButtonPress, FALLING);
}

// === LOOP ===
void loop() {
    if (buttonPressed) {
        portENTER_CRITICAL(&mux);
        buttonPressed = false;
        portEXIT_CRITICAL(&mux);

        // Toggle LED state and store it
        ledState = !ledState;
        digitalWrite(LED_PIN, ledState);
        EEPROM.writeBool(EEPROM_ADDR, ledState);
        EEPROM.commit();

        Serial.printf("Button pressed. New LED state: %s\n", ledState ? "ON" : "OFF");
    }
}
