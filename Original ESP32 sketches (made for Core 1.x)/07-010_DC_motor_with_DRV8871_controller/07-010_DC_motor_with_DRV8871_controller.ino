/*  07-010 - Simple DC motor control with the DRV8871 controller IC
 * 
 * This sketch shows you how to control a small DC motor using the DRV8871 controller
 * breakout.
 * 
 * The sketch speeds up the motor in three steps towards one direction,
 * and then does the same in the other direction.
 * 
 * 
 * Components
 * ----------
 *  - ESP32
 *  - a DRV8871 motor controller breakout
 *  - a small DC motor
 *  
 *  Libraries
 *  ---------
 *  None
 *
 * Connections
 * -----------
 *  
 * ESP32     |   DRV8871 
 * ---------------------
 *   GND     |   GND
 *   GPIO2   |   IN1  (direction)
 *   GPIO21  |   IN2 (speed)
 *           |   MOTOR1 --> Motor wire 1
 *           |   MOTOR1 --> Motor wire 2
 *           |   POWER+ --> 9V power supply 
 *           |   POWER- --> GND power supply
 *     
 * More information:
 *
 * 1. More about the Adafruit DRV8871: https://www.adafruit.com/product/3190
 * 2. DRV8871 datasheet: http://www.ti.com/lit/ds/symlink/drv8871.pdf
 *  
 *  Created on April 1 2019 by Peter Dalmaris
 * 
 */

int speed1     = 21; //Controls speed (PWM) 
int direction1 = 2; // Controls direction. This pin is connected to the 
                    // on-board blue LED so we can also "see" the direction.
   
bool direction = HIGH;  // Start the motor by moving it towards one direction. Whether it is
                        // clockwise or anticlockwise depends on how you have connected the 
                        // motor's coil.
 
void setup() 
{ 
    pinMode(direction1, OUTPUT);  

    ledcAttachPin(speed1, 0); // assign the speed control PWM pin to a channel

    // Initialize channels
    // channels 0-15, resolution 1-16 bits, freq limits depend on resolution
    // ledcSetup(uint8_t channel, uint32_t freq, uint8_t resolution_bits);
    ledcSetup(0, 4000, 8);
 
} 
 
void loop() 
{ 
  
    digitalWrite(direction1,direction);   // Direction control
    ledcWrite(0, 100);             //PWM Speed Control
    delay(1000); 
    ledcWrite(0, 155);             //PWM Speed Control
    delay(1000); 
    ledcWrite(0, 255);             //PWM Speed Control
    delay(1000); 
    
    direction = !direction;               //switch direction
    ledcWrite(0, 0);               // Stop the motor before starting again
}
