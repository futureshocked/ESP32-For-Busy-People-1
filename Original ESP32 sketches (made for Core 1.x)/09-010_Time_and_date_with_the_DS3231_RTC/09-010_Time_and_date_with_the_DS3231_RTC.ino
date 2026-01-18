/*  09.010 - Time and date with the DS3231 RTC

   This sketch demonstrates how to set the time and date, and then print them
   to the serial monitor using the highly accurate DS3231 real time clock.

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

      ESP32        |     DS3231 RTC
    -------------------------
      3.3V         |     Vcc
      GND          |     GND
      GPIO21 (SDA) |     SDA
      GPIO22 (SCL) |     SCL
      -            |     SQW
      -            |     32K

    Other information
    -----------------

    1. ESP32 Datasheet: https://www.espressif.com/sites/default/files/documentation/esp32_datasheet_en.pdf
    2. Rtc library (contains libraries for various RTCs): https://github.com/Makuna/Rtc
    3. Rtc library wiki: https://github.com/Makuna/Rtc/wiki
    4. DS3231 datasheet: https://datasheets.maximintegrated.com/en/ds/DS3231.pdf
    5. Printf format parameters: http://www.cplusplus.com/reference/cstdio/printf/
    6. snprintf (very similar to snprintf_P): http://www.cplusplus.com/reference/cstdio/snprintf/?kw=snprintf
    7. snprintf_P: https://www.microchip.com/webdoc/AVRLibcReferenceManual/group__avr__stdio_1ga53ff61856759709eeceae10aaa10a0a3.html

    Created on April 3 2019 by Peter Dalmaris

*/

/* for normal hardware wire use below */
#include <Wire.h> // must be included here so that Arduino library object file references work
#include <RtcDS3231.h>
RtcDS3231<TwoWire> Rtc(Wire);
/* for normal hardware wire use above */


void setup ()
{
  Serial.begin(115200);

  Serial.print("compiled: ");
  Serial.print(__DATE__);
  Serial.println(__TIME__);

  Rtc.Begin();

  RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
  Serial.print("Compiled: ");
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
  Rtc.SetSquareWavePin(DS3231SquareWavePin_ModeNone);
}

void loop ()
{
  RtcDateTime now = Rtc.GetDateTime();
  printDateTime(now);
  Serial.println();

  RtcTemperature temp = Rtc.GetTemperature();
  temp.Print(Serial);
  // you may also get the temperature as a float and print it
  // Serial.print(temp.AsFloatDegC());
  Serial.println("C");

  delay(10000); // ten seconds
}

#define countof(a) (sizeof(a) / sizeof(a[0]))

void printDateTime(const RtcDateTime& dt)
{
  char datestring[20];

  snprintf_P(datestring,
             countof(datestring),
             PSTR("%02u/%02u/%04u %02u:%02u:%02u"),
             dt.Month(),
             dt.Day(),
             dt.Year(),
             dt.Hour(),
             dt.Minute(),
             dt.Second() );
  Serial.print(datestring);
}
