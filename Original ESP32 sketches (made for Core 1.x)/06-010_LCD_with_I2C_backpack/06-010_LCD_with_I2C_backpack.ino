/*  06.010 - LCD screen with the I2C backpack demo sketch

   This sketch shows you how to use the 16x2 LCD display
   using the I2C backpack adaptor. This way, we can save a
   lot of digital pins on the Arduino.

   It is provided in this course to support projects that
   require an LCD screen for text output

   This I2C LCD backpack contains the PCF8574 port-expander
   IC. Beware that this sketch can work with backpacks that
   contains this IC, but may not work with variations.


   Components
   ----------
    - ESP32
    - An I2C to LCD backpack adaptor
    - Jumper wires
    - Breadboard

    Libraries
    ---------
    - LiquidCrystal_I2C

   Connections
   -----------
    ESP32         |    I2C backpack
    -----------------------------
        GND       |      GND
        5V        |      5V
        GPIO42    |      SDA
        GPIO39    |      SCL


   Other information
   -----------------
    1. ESP32 Datasheet: https://www.espressif.com/sites/default/files/documentation/esp32_datasheet_en.pdf
    2. For information on the LiquidCrystal library: https://github.com/johnrickman/LiquidCrystal_I2C
    3. If you need to run the port scanner, look for sketch 06-012 in this course repository

    Created on April 1 2019 by Peter Dalmaris

*/

#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2); // If this address is not working for your I2C backpack,
// run the address scanner sketch to determine the actual
// address.

void setup()
{
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Hello world!");
  lcd.setCursor(0, 1);
  lcd.print("Row number: ");
  lcd.setCursor(12, 1);
  lcd.print("2");
  delay(2000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Backlight On");
  lcd.backlight();
  delay(2000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Backlight Off");
  lcd.noBacklight();
  delay(2000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Backlight On");
  lcd.backlight();
  delay(2000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Scroll");
  for (int i = 0; i < 5; i++)
  {
    delay(500);
    lcd.scrollDisplayRight();
  }
  delay(2000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Demo finished");

}
void loop()
{

}
