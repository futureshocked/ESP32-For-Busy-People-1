/*  07.011 - DC Motor Control with DRV8871 (Minimal, Modern API)
 * 
 * Course: IoT Development with ESP32-S3
 * Target Hardware: ESP32-S3-DevKitC-1 v1.1
 * Module: ESP32-S3-WROOM-1-N16R16V
 * 
 * This minimal script demonstrates basic control of a brushed DC motor using the 
 * DRV8871 motor driver with the ESP32-S3's LEDC peripheral. It gradually changes 
 * motor speed in three steps, then reverses direction. It uses the modern LEDC API.
 * 
 * HARDWARE INFORMATION:
 * =====================
 * - ESP32-S3-DevKitC-1 v1.1
 * - 16MB Flash, 16MB PSRAM
 * - GPIO-based PWM using LEDC
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
 * - Brushed DC motor (6–9V)
 * - 9V external power supply
 * 
 * GPIO ASSIGNMENTS:
 * =================
 * - DIR_PIN  -> GPIO6
 * - PWM_PIN  -> GPIO7
 * 
 * TECHNICAL NOTES:
 * ================
 * - Uses new `ledcAttach()` and `ledcWrite()` APIs (ESP32 Arduino Core v3.0+)
 * - 8-bit PWM resolution with 4kHz frequency
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

const int DIR_PIN  = 6;
const int PWM_PIN  = 7;
const int PWM_FREQ = 4000;
const int PWM_RES  = 8;

bool direction = HIGH;

void setup() {
  pinMode(DIR_PIN, OUTPUT);
  digitalWrite(DIR_PIN, direction);
  ledcAttach(PWM_PIN, PWM_FREQ, PWM_RES);
}

void loop() {
  digitalWrite(DIR_PIN, direction);
  ledcWrite(PWM_PIN, 100);
  delay(1000);
  ledcWrite(PWM_PIN, 155);
  delay(1000);
  ledcWrite(PWM_PIN, 255);
  delay(1000);
  ledcWrite(PWM_PIN, 0);
  delay(500);
  direction = !direction;
}
