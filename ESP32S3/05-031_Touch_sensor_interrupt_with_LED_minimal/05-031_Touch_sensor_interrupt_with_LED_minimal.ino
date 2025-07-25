/*  05.031 - Touch Sensor Interrupt with LED for ESP32-S3
 *
 * Course: IoT Development with ESP32-S3
 * Target Hardware: ESP32-S3-DevKitC-1 v1.1
 * Module: ESP32-S3-WROOM-1-N16R16V
 *
 * This sketch demonstrates how to trigger an LED using an interrupt from a capacitive touch sensor.
 * It replicates the behavior of legacy ESP32 Dev Kit code using GPIO13 (Touch Pad 4) but adapted
 * for the ESP32-S3 which does not support internal touch sensors. Instead, we use an external capacitive touch sensor
 * module (e.g., TTP223) connected to a digital input.
 *
 * HARDWARE INFORMATION:
 * =====================
 * - Board: ESP32-S3-DevKitC-1 v1.1
 * - Module: ESP32-S3-WROOM-1-N16R16V
 * - Flash: 16 MB
 * - PSRAM: 16 MB
 * - GPIOs used: GPIO10 (touch input), GPIO2 (LED output)
 *
 * CIRCUIT DIAGRAM:
 * ================
 *
 *                 +3V3 o----+
 *                          |
 *                      +---+---+
 *                      | TTP223|
 *                      +---+---+
 *                          |
 *         GPIO10 <---------+ TTP223 OUT
 *                          |
 *        ESP32 GND o-------+ GND
 *
 *                    +--[330Ω]--->|---+
 *                    |  LED           |
 *         GPIO2 <----+                |
 *                                     |
 *                                ESP32 GND
 *
 * COMPONENTS:
 * ===========
 * - ESP32-S3-DevKitC-1 v1.1
 * - LED (any color)
 * - 330 Ohm resistor
 * - TTP223 capacitive touch sensor module
 * - Breadboard and jumper wires
 *
 * GPIO ASSIGNMENTS:
 * =================
 * | Component         | GPIO  | Notes                         |
 * |-------------------|-------|-------------------------------|
 * | LED               | GPIO2 | Output to LED (via resistor)  |
 * | TTP223 OUT        | GPIO10| Input for interrupt trigger   |
 *
 * TECHNICAL NOTES:
 * ================
 * - The ESP32-S3 does not include internal capacitive touch sensors.
 * - External modules like the TTP223 provide simple digital HIGH/LOW outputs when touched.
 * - Interrupts are triggered on falling edge (touch = LOW).
 * - Use internal pull-up to ensure a stable high signal when not touched.
 *
 * IDE REQUIREMENTS:
 * =================
 * - Arduino IDE 2.0+
 * - ESP32 Arduino Core v3.0+
 * - Board: "ESP32S3 Dev Module"
 *
 * Created: March 27, 2019 by Peter Dalmaris
 * Updated: July 25, 2025 for ESP32-S3 compatibility
 */

// =======================
// Configuration Constants
// =======================
constexpr uint8_t LED_GPIO = 2;
constexpr uint8_t TOUCH_INPUT_GPIO = 10;  // Must be a usable input GPIO
constexpr unsigned long LED_ON_DURATION_MS = 500;

// =======================
// Global Variables
// =======================
volatile bool touchDetected = false;

// =======================
// Interrupt Service Routine
// =======================
void IRAM_ATTR handleTouchInterrupt() {
    touchDetected = true;
}

// =======================
// Setup Function
// =======================
void setup() {
    pinMode(LED_GPIO, OUTPUT);
    pinMode(TOUCH_INPUT_GPIO, INPUT_PULLUP);

    Serial.begin(115200);
    delay(1000); // Allow time to open Serial Monitor
    Serial.println("ESP32-S3 Touch Sensor Interrupt Example");

    // Attach interrupt to TTP223 output (active LOW when touched)
    attachInterrupt(digitalPinToInterrupt(TOUCH_INPUT_GPIO), handleTouchInterrupt, FALLING);
}

// =======================
// Main Loop
// =======================
void loop() {
    if (touchDetected) {
        Serial.println("Touch detected");
        digitalWrite(LED_GPIO, HIGH);
        delay(LED_ON_DURATION_MS);
        digitalWrite(LED_GPIO, LOW);
        touchDetected = false;
    }
}
