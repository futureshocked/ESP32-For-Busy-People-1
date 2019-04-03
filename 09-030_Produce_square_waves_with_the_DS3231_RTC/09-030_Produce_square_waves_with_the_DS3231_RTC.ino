/*  09.030 - Square waves with the DS3231 RTC

   This sketch demonstrates how to generate square waves of different
   frequencies with the DS3231 real time clock.

   To see the produces waveforms, you will need an osciloscope.

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
      5V           |     Vcc
      GND          |     GND
      GPIO21 (SDA) |     SDA
      GPIO22 (SCL) |     SCL
      -            |     SQW 
      -            |     32K

    Connect the positive electrode of the osciloscope lead to SQW, and the negative to GND.

    To view the waveform from the 32K pin, connect the positive lead of the osciloscope to 32K, 
    and set Rtc.Enable32kHzPin(true).

    Other information
    -----------------

    1. ESP32 Datasheet: https://www.espressif.com/sites/default/files/documentation/esp32_datasheet_en.pdf
    2. Rtc library (contains libraries for various RTCs): https://github.com/Makuna/Rtc    

    Created on April 3 2019 by Peter Dalmaris

*/

/* for normal hardware wire use below */
#include <Wire.h> // must be included here so that Arduino library object file references work
#include <RtcDS3231.h>
RtcDS3231<TwoWire> Rtc(Wire);
/* for normal hardware wire use above */


void setup () 
{
    Serial.begin(57600);

    Serial.print("compiled: ");
    Serial.print(__DATE__);
    Serial.println(__TIME__);

    //--------RTC SETUP ------------
    // if you are using ESP-01 then uncomment the line below to reset the pins to
    // the available pins for SDA, SCL
    // Wire.begin(0, 2); // due to limited pins, use pin 0 and 2 for SDA, SCL
    
    Rtc.Begin();

    if (!Rtc.GetIsRunning())
    {
        Serial.println("RTC was not actively running, starting now");
        Rtc.SetIsRunning(true);
    }

    // never assume the Rtc was last configured by you, so
    // just clear them to your needed state
    Rtc.Enable32kHzPin(false);
//    Rtc.SetSquareWavePin(DS3231SquareWavePin_ModeNone); 
    Rtc.SetSquareWavePin(DS3231SquareWavePin_ModeClock); 

    // See https://github.com/Makuna/Rtc/blob/master/src/RtcDS3231.h
    // for available frequencies
    Rtc.SetSquareWavePinClockFrequency(DS3231SquareWaveClock_1Hz); 
    delay(2000);    
}

void loop () 
{

//    Rtc.SetSquareWavePinClockFrequency(DS3231SquareWaveClock_4kHz);
//    delay(2000);
//    Rtc.SetSquareWavePinClockFrequency(DS3231SquareWaveClock_8kHz);
//    delay(2000);
//    Rtc.SetSquareWavePinClockFrequency(DS3231SquareWaveClock_1kHz);
//    delay(2000);
}
