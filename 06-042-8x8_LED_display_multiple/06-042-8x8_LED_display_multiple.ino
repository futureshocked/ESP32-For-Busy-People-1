/*  06-042 - Multiple 8x8 LED matrix display
 * 
 * This sketch shows you how to display simple shapes and animation
 * on four 8x8 LED matrix display
 * 
 * The sketch contains instructions that demonstrate various primitives, like pixels
 * lines and circles. Uncomment the instructions that you are interested in to 
 * see them in action.
 * 
 * This sketch was written by Peter Dalmaris for Arduino Step by Step.
 * 
 * Components
 * ----------
 *  - ESP32
 *  - 4 x 8x8 LED matrix display with the Max72xx driver IC

 *  - Jumper wires
 *  
 *  Libraries
 *  ---------
 *  - SPI (Comes with the Arduino IDE)
 *  - Adafruit_GFX
 *  - Max72xxPanel
 *
 * Connections
 * -----------
 *
 *         ESP32          |     8x8 LED Matrix
 *   --------------------------------------------
 *         3.3V          |      VCC
 *         GND           |      GND
 *         GPIO23 (MOSI) |      DIN (SPI data)
 *         GPIO18 (SCK)  |      CLK (SPI Clock)
 *         GPIO5  (SS)   |      CS
 *        
 * Other information
 *   ------------------
 *   For more info on the graphics primitives that you can use, see https://github.com/adafruit/Adafruit-GFX-Library/blob/master/Adafruit_GFX.h
 *
 *   1. Adafruit GFX: https://github.com/adafruit/Adafruit-GFX-Library
 *   2. Max72xxPanel: https://github.com/markruys/arduino-Max72xxPanel
 *   3. Max7219 LED driver datasheet: https://datasheets.maximintegrated.com/en/ds/MAX7219-MAX7221.pdf
 *  
 *  Created on May 22 2017 by Peter Dalmaris
 * 
 */

#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Max72xxPanel.h>

int pinCS = 5; // Attach CS to this pin, DIN to MOSI and CLK to SCK (cf http://arduino.cc/en/Reference/SPI )

Max72xxPanel matrix = Max72xxPanel(pinCS, 4, 1);  // This constructor needs the CS pin and the display dimensions

const int total_pixels_in_smiley=25;
const int total_coordinates_per_pixel=2;
int smiley[total_pixels_in_smiley][total_coordinates_per_pixel] =
{
  {1, 0},
  {2, 0},
  {1, 1},
  {1, 2},
  {2, 1},
  {2, 2},
  {5, 0},
  {6, 0},
  {5, 1},
  {5, 2},
  {6, 1},
  {6, 2},
  {3, 1},
  {3, 2},
  {3, 3},
  {3, 4},
  {4, 4},
  {0, 5},
  {1, 6},
  {2, 7},
  {3, 7},
  {4, 7},
  {5, 7},
  {6, 6},
  {7, 5}
};

void setup() {
  matrix.setIntensity(4); // Set brightness between 0 and 15
  matrix.fillScreen(LOW); // Clear the screen
//// Adjust to your own needs
  matrix.setPosition(3, 0, 0); // The first display is at <0, 0>
  matrix.setPosition(2, 1, 0); // The second display is at <1, 0>
  matrix.setPosition(1, 2, 0); // The third display is at <2, 0>
  matrix.setPosition(0, 3, 0); // And the last display is at <3, 0>
//  ...
//  matrix.setRotation(0, 2);    // The first display is position upside down
//  matrix.setRotation(3, 2);    // The same hold for the last display
  //               x  y

// Draw a pixel at 0,0
//               x, y, color
//matrix.drawPixel(0, 0, HIGH);
//matrix.drawPixel(31, 0, HIGH);

//  // LEFT EYE
//  matrix.drawPixel(1, 0, HIGH);
//  matrix.drawPixel(2, 0, HIGH);
//  matrix.drawPixel(1, 1, HIGH);
//  matrix.drawPixel(1, 2, HIGH);
//  matrix.drawPixel(2, 1, HIGH);
//  matrix.drawPixel(2, 2, HIGH);
//  //--------
//  // RIGHT EYE
//  matrix.drawPixel(5, 0, HIGH);
//  matrix.drawPixel(6, 0, HIGH);
//  matrix.drawPixel(5, 1, HIGH);
//  matrix.drawPixel(5, 2, HIGH);
//  matrix.drawPixel(6, 1, HIGH);
//  matrix.drawPixel(6, 2, HIGH);
//  //--------
//  // NOSE
//  matrix.drawPixel(3, 1, HIGH);
//  matrix.drawPixel(3, 2, HIGH);
//  matrix.drawPixel(3, 3, HIGH);
//  matrix.drawPixel(3, 4, HIGH);
//  matrix.drawPixel(4, 4, HIGH);
//  //--------
//  // MOUTH
//  matrix.drawPixel(0, 5, HIGH);
//  matrix.drawPixel(1, 6, HIGH);
//  matrix.drawPixel(2, 7, HIGH);
//  matrix.drawPixel(3, 7, HIGH);
//  matrix.drawPixel(4, 7, HIGH);
//  matrix.drawPixel(5, 7, HIGH);
//  matrix.drawPixel(6, 6, HIGH);
//  matrix.drawPixel(7, 5, HIGH);

//Instead of setting the pixels manually, using the individual drawPixel instructions above, just use this loop:
//  for (int pixel=0; pixel< total_pixels_in_smiley;pixel++)
//    matrix.drawPixel(smiley[pixel][0], smiley[pixel][1], HIGH);

//There are also a few other things you can do:
//Draw a line
//               x0, y0, x1, y1, color
//matrix.drawLine(0, 0, 31, 7, HIGH);

//Draw a box
//              x, y, w, h, color
//matrix.drawRect(0, 0, 32, 8, HIGH);
//matrix.fillRect(0, 0, 32, 8, HIGH);

//Draw a circle
//                x, y, r, color
//matrix.drawCircle(3, 3, 3, HIGH);
//matrix.fillCircle(3, 3, 3, HIGH);

//Draw a letter
//              x, y, char, clr, bg, size
//matrix.drawChar(0, 0, 'A', HIGH, LOW, 1);


matrix.write(); // Send bitmap to display
}

void loop() {

//animate_pixel();  //Animate a single pixel
//animate_letter();   //Animate a character
}

void animate_letter(){
  matrix.fillScreen(LOW); // Clear the screen before we start

  for (int x_pixel_loc=0; x_pixel_loc<32; x_pixel_loc++)
{
  matrix.drawChar(x_pixel_loc, 0, 'A', HIGH, LOW, 1);
  matrix.write();
  delay(40);
  matrix.drawChar(x_pixel_loc, 0, 'A', LOW, LOW, 1);
  matrix.write();
}

for (int x_pixel_loc=31; x_pixel_loc>-1; x_pixel_loc--)
{
  matrix.drawChar(x_pixel_loc, 0, 'A', HIGH, LOW, 1);
  matrix.write();
  delay(40);
  matrix.drawChar(x_pixel_loc, 0, 'A', LOW, LOW, 1);
  matrix.write();
}
  
}
void animate_pixel(){
//Simple animation demo
//Make a pixel move left-right

matrix.fillScreen(LOW); // Clear the screen before we start
for (int x_pixel_loc=0; x_pixel_loc<32; x_pixel_loc++)
{
  matrix.drawPixel(x_pixel_loc, 3, HIGH);
  matrix.write();
  delay(50);
  matrix.drawPixel(x_pixel_loc, 3, LOW);
  matrix.write();
}
for (int x_pixel_loc=31; x_pixel_loc>-1; x_pixel_loc--)
{
  matrix.drawPixel(x_pixel_loc, 3, HIGH);
  matrix.write();
  delay(50);
  matrix.drawPixel(x_pixel_loc, 3, LOW);
  matrix.write();
}
  

}
