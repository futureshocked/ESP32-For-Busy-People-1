/*  05.052 - BME280 environment sensor using I2C and timer interrupt

   This sketch shows you how to read temperature, humidity and barometric pressure
   values from the BME280 sensor. This sketch also show the calculated altitude.

   Instead of using the delay function (bad, blocking!), this example show how to
   use a timer interrupt. This makes more efficient use of the hardware.

   After uploading the sketch to your ESP32, open the serial monitor to see the
   values from the sensor.

   This sketch was written by Peter Dalmaris using information from the
   ESP32 datasheet and the Adafruit example.


   Components
   ----------
    - ESP32 Dev Kit v4
    - BME280 sensor breakout

    IDE
    ---
    Arduino IDE with ESP32 Arduino Code
    (https://github.com/espressif/arduino-esp32)


    Libraries
    ---------
    - Adafruit_Sensor
    - Adafruit_BME280

   Connections
   -----------

    Connect the sensor breakout like this:

    ESP32   |    BME280
    -------------------
     3.3V   |    Vin
     GND    |    GND
     GPIO22 |    SCL
     GPIO21 |    SDA


    Other information
    -----------------

    1. ESP32 Datasheet: https://www.espressif.com/sites/default/files/documentation/esp32_datasheet_en.pdf
    2. Sensor product page: https://www.bosch-sensortec.com/bst/products/all_products/bme280
    3. Sensor datasheet: https://ae-bst.resource.bosch.com/media/_tech/media/datasheets/BST-BME280-DS002.pdf
    3. Library Adafruit_Sensor: https://github.com/adafruit/Adafruit_Sensor
    4. Library Adafruit_BME280: https://github.com/adafruit/Adafruit_BME280_Library
    5. ESP32 Arduino core esp32-hal-timer.h: https://github.com/espressif/arduino-esp32/blob/master/cores/esp32/esp32-hal-timer.h
    6. Espressif Docs about ESP32 timers: https://docs.espressif.com/projects/esp-idf/en/latest/api-reference/peripherals/timer.html
    
    Created on March 28 2019 by Peter Dalmaris

*/

/***************************************************************************
  This is a library for the BME280 humidity, temperature & pressure sensor

  Designed specifically to work with the Adafruit BME280 Breakout
  ----> http://www.adafruit.com/products/2650

  These sensors use I2C or SPI to communicate, 2 or 4 pins are required
  to interface. The device's I2C address is either 0x76 or 0x77.

  Adafruit invests time and resources providing this open source code,
  please support Adafruit andopen-source hardware by purchasing products
  from Adafruit!

  Written by Limor Fried & Kevin Townsend for Adafruit Industries.
  BSD license, all text above must be included in any redistribution
 ***************************************************************************/

#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#define SEALEVELPRESSURE_HPA (1013.25)

Adafruit_BME280 bme; // I2C

volatile int interruptCounter;  // When this is not zero, we'll take a reading from the sensor
// The interrupt service routine will increment it.
// When the sensor is read, this variable is decremented.

// The hardware timer pointer
hw_timer_t * timer = NULL;      

// This variable is used for syncronisation
// We use it to ensure that the ISR and the loop
// do not try to access the interruptCounter variable
// at the same time.
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;  



void IRAM_ATTR onTimer() {
  portENTER_CRITICAL_ISR(&timerMux);
  interruptCounter++;
  portEXIT_CRITICAL_ISR(&timerMux);
}

void setup() {
  Serial.begin(9600);
  Serial.println(F("BME280 test"));
  
  bool status;

  // default settings
  status = bme.begin(0x76);  // BME280 sensors are usually set to address 0x76 or 0x77
  // If your BME280 sensor module has an SD0 pin, then:
  // SD0 unconnected configures the address to 0x77
  // SD0 to GND configures the address to 0x76
  // If there is no SD0 pin, try either address to find out which
  // one works with your sensor.
  if (!status) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1);
  }

  Serial.println("-- Default Test --");

  Serial.println();

  // Initilise the timer.
  // Parameter 1 is the timer we want to use. Valid: 0, 1, 2, 3 (total 4 timers)
  // Parameter 2 is the prescaler. The ESP32 default clock is at 80MhZ. The value "80" will 
  // divide the clock by 80, giving us 1,000,000 ticks per second.
  // Parameter 3 is true means this counter will count up, instead of down (false).
  timer = timerBegin(0, 80, true);

  // Attach the timer to the interrupt service routine named "onTimer".
  // The 3rd parameter is set to "true" to indicate that we want to use the "edge" type (instead of "flat").
  timerAttachInterrupt(timer, &onTimer, true);

  // This is where we indicate the frequency of the interrupts.
  // The value "1000000" (because of the prescaler we set in timerBegin) will produce 
  // one interrupt every second. 
  // The 3rd parameter is true so that the counter reloads when it fires an interrupt, and so we
  // can get periodic interrupts (instead of a single interrupt).
  timerAlarmWrite(timer, 1000000, true);

  // Start the timer
  timerAlarmEnable(timer);
}

void loop() {
  if (interruptCounter > 0) {
    
    portENTER_CRITICAL(&timerMux);
    interruptCounter--;
    portEXIT_CRITICAL(&timerMux);

    printValues();     // Access the sensor and print the values
  }
}

void printValues() {
  Serial.print("Temperature = ");
  Serial.print(bme.readTemperature());
  Serial.println(" *C");

  Serial.print("Pressure = ");

  Serial.print(bme.readPressure() / 100.0F);
  Serial.println(" hPa");

  Serial.print("Approx. Altitude = ");
  Serial.print(bme.readAltitude(SEALEVELPRESSURE_HPA));
  Serial.println(" m");

  Serial.print("Humidity = ");
  Serial.print(bme.readHumidity());
  Serial.println(" %");

  Serial.println();
}
