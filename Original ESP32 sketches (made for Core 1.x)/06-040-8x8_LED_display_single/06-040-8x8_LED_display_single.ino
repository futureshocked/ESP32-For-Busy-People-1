/*  0760 - Single 8x8 LED matrix display

   This sketch shows you how to display simple shapes and animation
   on a 8x8 LED matrix display

   The sketch contains instructions that demonstrate various primitives, like pixels
   lines and circles. Uncomment the instructions that you are interested in to
   see them in action.

   This sketch was written by Peter Dalmaris for Arduino Step by Step.

   Components
   ----------
    - ESP32
    - 1 x 8x8 LED matrix display with the Max72xx driver IC

    - Jumper wires

    Libraries
    ---------
    - SPI (Comes with the ESP32 Arduino Core)
    - Adafruit_GFX
    - Max72xxPanel

   Connections
   -----------

         ESP32          |     8x8 LED Matrix
    --------------------------------------------
          3.3V          |      VCC
          GND           |      GND
          GPIO23 (MOSI) |      DIN (SPI data)
          GPIO18 (SCK)  |      CLK (SPI Clock)
          GPIO5  (SS)   |      CS

    Other information
    ------------------
    For more info on the graphics primitives that you can use, see https://github.com/adafruit/Adafruit-GFX-Library/blob/master/Adafruit_GFX.h

    1. Adafruit GFX: https://github.com/adafruit/Adafruit-GFX-Library
    2. Max72xxPanel: https://github.com/markruys/arduino-Max72xxPanel
    3. Max7219 LED driver datasheet: https://datasheets.maximintegrated.com/en/ds/MAX7219-MAX7221.pdf


    Created on April 1 2019 by Peter Dalmaris



*/

#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Max72xxPanel.h>

int pinCS = 5; // Attach CS to this pin, DIN to MOSI and CLK to SCK (cf http://arduino.cc/en/Reference/SPI )

Max72xxPanel matrix = Max72xxPanel(pinCS, 1, 1);  // This constructor needs the CS pin and the display dimensions

const int total_pixels_in_smiley = 25;
const int total_coordinates_per_pixel = 2;
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
  //                 x, y, color

//    matrix.drawPixel(0, 7, HIGH);
//    // LEFT EYE
//    matrix.drawPixel(1, 0, HIGH);
//    matrix.drawPixel(2, 0, HIGH);
//    matrix.drawPixel(1, 1, HIGH);
//    matrix.drawPixel(1, 2, HIGH);
//    matrix.drawPixel(2, 1, HIGH);
//    matrix.drawPixel(2, 2, HIGH);
//    //--------
//    // RIGHT EYE
//    matrix.drawPixel(5, 0, HIGH);
//    matrix.drawPixel(6, 0, HIGH);
//    matrix.drawPixel(5, 1, HIGH);
//    matrix.drawPixel(5, 2, HIGH);
//    matrix.drawPixel(6, 1, HIGH);
//    matrix.drawPixel(6, 2, HIGH);
//    //--------
//    // NOSE
//    matrix.drawPixel(3, 1, HIGH);
//    matrix.drawPixel(3, 2, HIGH);
//    matrix.drawPixel(3, 3, HIGH);
//    matrix.drawPixel(3, 4, HIGH);
//    matrix.drawPixel(4, 4, HIGH);
//    //--------
//    // MOUTH
//    matrix.drawPixel(0, 5, HIGH);
//    matrix.drawPixel(1, 6, HIGH);
//    matrix.drawPixel(2, 7, HIGH);
//    matrix.drawPixel(3, 7, HIGH);
//    matrix.drawPixel(4, 7, HIGH);
//    matrix.drawPixel(5, 7, HIGH);
//    matrix.drawPixel(6, 6, HIGH);
//    matrix.drawPixel(7, 5, HIGH);

  //Instead of setting the pixels manually, using the individual drawPixel instructions above, just use this loop:
    for (int pixel=0; pixel< total_pixels_in_smiley;pixel++)
      matrix.drawPixel(smiley[pixel][0], smiley[pixel][1], HIGH);

  //There are also a few other things you can do:
  //Draw a line
  //               x0, y0, x1, y1, color
  //matrix.drawLine(0, 0, 7, 7, HIGH);

  //Draw a box
  //              x, y, w, h, color
  //matrix.drawRect(0, 0, 8, 8, HIGH);
  //matrix.fillRect(0, 0, 6, 5, HIGH);

  //Draw a circle
  //                x, y, r, color
//  matrix.drawCircle(3, 3, 3, HIGH);
  //matrix.fillCircle(3, 3, 3, HIGH);

  //Draw a letter
  //              x, y, char, clr, bg, size
  //matrix.drawChar(0, 0, 'a', HIGH, LOW, 1);

  matrix.write(); // Send bitmap to display
}

void loop() {
    //animate_pixel();
}

void animate_pixel() {
  //Simple animation demo
  //Make a pixel move left-right

  matrix.fillScreen(LOW); // Clear the screen before we start
  for (int x_pixel_loc = 0; x_pixel_loc < 8; x_pixel_loc++)
  {
    matrix.drawPixel(x_pixel_loc, 3, HIGH);
    matrix.write();
    delay(20);
    matrix.drawPixel(x_pixel_loc, 3, LOW);
    matrix.write();
  }
  for (int x_pixel_loc = 7; x_pixel_loc > -1; x_pixel_loc--)
  {
    matrix.drawPixel(x_pixel_loc, 3, HIGH);
    matrix.write();
    delay(60);
    matrix.drawPixel(x_pixel_loc, 3, LOW);
    matrix.write();
  }
}
