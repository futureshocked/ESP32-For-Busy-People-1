/*  07.011 - DC Motor Control with DRV8871 using LEDC PWM (modern API) on ESP32-S3
 * 
 * Course: IoT Development with ESP32-S3
 * Target Hardware: ESP32-S3-DevKitC-1 v1.1
 * Module: ESP32-S3-WROOM-1-N16R16V
 * 
 * This sketch demonstrates how to control a small brushed DC motor using the DRV8871 
 * motor driver and the modern LEDC API. It adjusts speed in steps and toggles direction.
 * 
 * HARDWARE INFORMATION:
 * =====================
 * - ESP32-S3-DevKitC-1 v1.1
 * - 16MB Flash, 16MB PSRAM
 * - Use GPIO 6 for direction
 * - Use GPIO 7 for PWM speed control
 * - Avoid GPIOs 35–37 (reserved)
 * 
 * CIRCUIT DIAGRAM:
 * ================
 * 
 * ESP32-S3         DRV8871
 * --------         -------
 * GPIO6    -->     IN1 (Direction)
 * GPIO7    -->     IN2 (PWM Speed)
 * GND      -->     GND
 * MOTOR OUT1 -->   DC Motor +
 * MOTOR OUT2 -->   DC Motor -
 * POWER+    -->    9V motor PSU
 * POWER-    -->    PSU GND (shared with ESP32 GND)
 * 
 * COMPONENTS:
 * ===========
 * - ESP32-S3-DevKitC-1 v1.1
 * - DRV8871 motor driver
 * - Brushed DC motor
 * - 9V external power supply
 * 
 * GPIO ASSIGNMENTS:
 * =================
 * - DIR_PIN  -> GPIO6
 * - PWM_PIN  -> GPIO7
 * 
 * TECHNICAL NOTES:
 * ================
 * - Uses new `ledcAttach()` and `ledcWrite()` APIs from ESP32 Arduino Core v3.0+
 * - 8-bit PWM resolution with 4kHz frequency
 * - Duty cycle range: 0–255
 * - Always stop the motor before reversing direction
 * 
 * IDE REQUIREMENTS:
 * =================
 * - Arduino IDE 2.0+
 * - ESP32 Arduino Core v3.0+
 * - Board: "ESP32S3 Dev Module"
 * 
 * Created: April 1, 2019 by Peter Dalmaris
 * Updated: July 28, 2025 for ESP32-S3 compatibility (modern LEDC API)
 */

constexpr uint8_t DIR_PIN       = 6;
constexpr uint8_t PWM_PIN       = 7;
constexpr uint32_t PWM_FREQ     = 4000; // 4 kHz
constexpr uint8_t PWM_RES_BITS  = 8;    // 0–255

bool directionState = HIGH;

void setup() {
    Serial.begin(115200);
    Serial.println("Starting DRV8871 motor control (modern API)");

    pinMode(DIR_PIN, OUTPUT);
    digitalWrite(DIR_PIN, directionState);

    // Use modern LEDC attach API
    ledcAttach(PWM_PIN, PWM_FREQ, PWM_RES_BITS);
}

void loop() {
    Serial.print("Direction: ");
    Serial.println(directionState ? "FORWARD" : "REVERSE");

    setMotorSpeed(100);
    delay(1000);

    setMotorSpeed(155);
    delay(1000);

    setMotorSpeed(255);
    delay(1000);

    stopMotor();
    Serial.println("Motor stopped. Reversing...");
    delay(500);

    // Toggle direction
    directionState = !directionState;
    digitalWrite(DIR_PIN, directionState);
}

/**
 * Sets the motor speed by writing to PWM pin.
 */
void setMotorSpeed(uint8_t duty) {
    ledcWrite(PWM_PIN, duty);
    Serial.print("PWM set to: ");
    Serial.println(duty);
}

/**
 * Stops the motor (PWM = 0).
 */
void stopMotor() {
    ledcWrite(PWM_PIN, 0);
    Serial.println("Motor stopped");
}
