/*  09.040 - Time and date with extarnal interrupts using the DS3231 RTC and I2C LCD

   This sketch demonstrates how to display the current time and date on a I2C LCD.
   Instead of using the delay function to refresh the LCD every second, we will use an
   external interrupt triggered by the RTC SQW pin, set to 1Hz square wave pulse.

   When equiped with a button battery, the DS3231 will retain correct time and date even
   when there is no main power from the microcontroller.

   The DS3231 also contains a temperature sensor that is readable via the library.

   This sketch was written by Peter Dalmaris using information from the
   ESP32 datasheet and examples.


   Components
   ----------
    - ESP32 Dev Kit v4
    - DS2331 RTC breakout

    IDE
    ---
    Arduino IDE with ESP32 Arduino Code
    (https://github.com/espressif/arduino-esp32)


    Libraries
    ---------
    - RtcDS3231
    - Wire

   Connections
   -----------

    Power is provided from the 5V pin to accomodate the requirements
    of the LCD. It is safe to use 5V with the DS3231 as long as it
    contains a voltage regulator.

      ESP32        |     DS3231 RTC
    -------------------------
      5V         |     Vcc
      GND          |     GND
      GPIO21 (SDA) |     SDA
      GPIO22 (SCL) |     SCL
      -            |     SQW
      -            |     32K

    Other information
    -----------------

    1. ESP32 Datasheet: https://www.espressif.com/sites/default/files/documentation/esp32_datasheet_en.pdf
    2. Rtc library (contains libraries for various RTCs): https://github.com/Makuna/Rtc
    3. Printf format parameters: http://www.cplusplus.com/reference/cstdio/printf/
    4. snprintf (very similar to snprintf_P): http://www.cplusplus.com/reference/cstdio/snprintf/?kw=snprintf
    5. snprintf_P: https://www.microchip.com/webdoc/AVRLibcReferenceManual/group__avr__stdio_1ga53ff61856759709eeceae10aaa10a0a3.html

    Created on April 3 2019 by Peter Dalmaris

*/

/* for normal hardware wire use below */
#include <Wire.h> // must be included here so that Arduino library object file references work
#include <RtcDS3231.h>
RtcDS3231<TwoWire> Rtc(Wire);

#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);

const byte interruptPin = 0;
volatile int seconds = 0;

portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

void handleInterrupt() {
  portENTER_CRITICAL_ISR(&mux);
  seconds++;
  portEXIT_CRITICAL_ISR(&mux);
}

void setup ()
{
  Serial.begin(115200);

  lcd.init();
  lcd.backlight();

  Serial.print("compiled: ");
  Serial.print(__DATE__);
  Serial.println(__TIME__);

  Rtc.Begin();

  RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
  printDateTime(compiled);
  Serial.println();

  if (!Rtc.IsDateTimeValid())
  {
    if (Rtc.LastError() != 0)
    {
      // we have a communications error
      // see https://www.arduino.cc/en/Reference/WireEndTransmission for
      // what the number means
      Serial.print("RTC communications error = ");
      Serial.println(Rtc.LastError());
    }
    else
    {
      // Common Cuases:
      //    1) first time you ran and the device wasn't running yet
      //    2) the battery on the device is low or even missing

      Serial.println("RTC lost confidence in the DateTime!");

      // following line sets the RTC to the date & time this sketch was compiled
      // it will also reset the valid flag internally unless the Rtc device is
      // having an issue

      Rtc.SetDateTime(compiled);
    }
  }

  if (!Rtc.GetIsRunning())
  {
    Serial.println("RTC was not actively running, starting now");
    Rtc.SetIsRunning(true);
  }

  RtcDateTime now = Rtc.GetDateTime();
  if (now < compiled)
  {
    Serial.println("RTC is older than compile time!  (Updating DateTime)");
    Rtc.SetDateTime(compiled);
  }
  else if (now > compiled)
  {
    Serial.println("RTC is newer than compile time. (this is expected)");
  }
  else if (now == compiled)
  {
    Serial.println("RTC is the same as compile time! (not expected but all is fine)");
  }

  // never assume the Rtc was last configured by you, so
  // just clear them to your needed state
  Rtc.Enable32kHzPin(false);
  Rtc.SetSquareWavePin(DS3231SquareWavePin_ModeClock); 
  Rtc.SetSquareWavePinClockFrequency(DS3231SquareWaveClock_1Hz); // Set the square wave to 1 sec period

  pinMode(interruptPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(interruptPin), handleInterrupt, FALLING);
}

void loop ()
{
  if (seconds > 0)
  {
    Serial.println(seconds);
    // another second has passed, update the LCD with the new time
    portENTER_CRITICAL_ISR(&mux);
    seconds = 0;  // Reset seconds and prepare for the next second
    portEXIT_CRITICAL_ISR(&mux);
    
    if (!Rtc.IsDateTimeValid())
    {
      if (Rtc.LastError() != 0)
      {
        // we have a communications error
        // see https://www.arduino.cc/en/Reference/WireEndTransmission for
        // what the number means
        Serial.print("RTC communications error = ");
        Serial.println(Rtc.LastError());
      }
      else
      {
        // Common Cuases:
        //    1) the battery on the device is low or even missing and the power line was disconnected
        Serial.println("RTC lost confidence in the DateTime!");
      }
    }

    RtcDateTime now = Rtc.GetDateTime();
    printDateTime(now);
    updateLCD(now, Rtc.GetTemperature());   
  }


}


#define countof(a) (sizeof(a) / sizeof(a[0]))

void updateLCD(const RtcDateTime& dt, RtcTemperature temp)
{
  char datestring[20];
  char timestring[20];

  // If you prefer to use the regular snprintf instead of snprintf_P, replace the PSTR macro with just the formating string ("%02u/%02u/%04u")
  snprintf_P(datestring,
             countof(datestring),
             PSTR("%02u/%02u/%04u"), // Construct the string in flash memory. Use printf format parameters
             dt.Month(),
             dt.Day(),
             dt.Year() );

  snprintf_P(timestring,
             countof(timestring),
             PSTR("%02u:%02u:%02u"), // Construct the string in flash memory. Use printf format parameters
             dt.Hour(),
             dt.Minute(),
             dt.Second() );

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(datestring);
  lcd.setCursor(0, 1);
  lcd.print(timestring);
  lcd.setCursor(10, 1);
  lcd.print(temp.AsFloatDegC());
  lcd.print("C");
}

void printDateTime(const RtcDateTime& dt)
{
  char datestring[20];

  snprintf_P(datestring,
             countof(datestring),
             PSTR("%02u/%02u/%04u %02u:%02u:%02u"), // Construct the string in flash memory. Use printf format parameters
             dt.Month(),
             dt.Day(),
             dt.Year(),
             dt.Hour(),
             dt.Minute(),
             dt.Second() );

  Serial.print(datestring);
}
