/*  05.021 - Touch sensor with LED (Minimal Version) for ESP32-S3
 * 
 * Simple touch sensor example for ESP32-S3-DevKitC-1 v1.1
 * Touch GPIO4 to turn LED on/off
 * 
 * Hardware:
 * - ESP32-S3-DevKitC-1 v1.1
 * - LED + 330Ω resistor on GPIO2
 * - Touch wire on GPIO4
 * 
 * Connections:
 * GPIO2 -> LED Anode (via 330Ω resistor)
 * GPIO4 -> Touch wire
 * GND   -> LED Cathode
 * 
 * Updated for ESP32-S3 by Peter Dalmaris, July 2025
 */

const uint8_t LED_PIN = 2;        // LED pin (safe for ESP32-S3)
const uint8_t TOUCH_PIN = 4;      // Touch sensor pin (T4)

void setup() {
    pinMode(LED_PIN, OUTPUT);
    Serial.begin(115200);
    delay(1000);
    Serial.println("ESP32-S3 Touch Test - Minimal Version");
    Serial.println("Touch GPIO4 to control LED");
    
    // Show baseline reading for threshold adjustment
    Serial.print("Baseline (no touch): ");
    Serial.println(touchRead(TOUCH_PIN));
    Serial.println("Adjust threshold in code if needed");
    Serial.println("---");
}

void loop() {
    uint16_t touchValue = touchRead(TOUCH_PIN);
    Serial.println("Touch value: " + String(touchValue));
    
    // Turn LED on when touch value drops below threshold
    // Note: ESP32-S3 touch values are typically 20000-40000 when not touched
    // and drop to 5000-15000 when touched. Adjust threshold as needed.
    if (touchValue < 20000) {  
        digitalWrite(LED_PIN, HIGH);
        Serial.println("LED ON - Touch detected!");
    } else {
        digitalWrite(LED_PIN, LOW);
        Serial.println("LED OFF");
    }
    
    delay(500);
}
