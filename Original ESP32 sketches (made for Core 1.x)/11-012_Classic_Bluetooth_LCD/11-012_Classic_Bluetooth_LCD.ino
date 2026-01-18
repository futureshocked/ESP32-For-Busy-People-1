/*  11.012 - Classic Bluetooth with LCD

   This example sketch shows how to create a wireless serial channel using
   classic Bluetooth.

   After you upload the sketch to your ESP32, connect to the BT serial port
   on your computer.

   Then, use a serial monitor to interact with the ESP32.

   Type something in the input box, and it will appear in the LCD.

   The readings from the sensor will appear in the serial monitor.

   Type "~" (ASCII 126) to clear the screen manually.

   This sketch was written by Peter Dalmaris using information from the
   ESP32 datasheet and examples.


   Components
   ----------
    - ESP32 Dev Kit v4
    - I2C LCD
    - BME280

    IDE
    ---
    Arduino IDE with ESP32 Arduino Code
    (https://github.com/espressif/arduino-esp32)


    Libraries
    ---------
    - WiFi
    - Adafruit_Sensor
    - Adafruit_BME280
    - LiquidCrystal_I2C

   Connections
   -----------

   Connect the hardware breakout like this:

    ESP32   |    BME280   |   I2C LCD
   ------------------------------------
    5V      |    Vin      |     Vcc
    GND     |    GND      |     GND
    GPIO22  |    SCL      |     SCL
    GPIO21  |    SDA      |     SDA


    Other information
    -----------------

    1. ESP32 Datasheet: https://www.espressif.com/sites/default/files/documentation/esp32_datasheet_en.pdf
    2. For information on the LiquidCrystal library: https://github.com/johnrickman/LiquidCrystal_I2C
    3. Bluetooth Serial: https://github.com/espressif/arduino-esp32/blob/master/libraries/BluetoothSerial/src/BluetoothSerial.h
    4. ASCII code table: http://www.asciitable.com/
    5. Liquid Crystal Display library: https://github.com/johnrickman/LiquidCrystal_I2C/blob/master/LiquidCrystal_I2C.h
    6. Learn about dtostrf: https://www.microchip.com/webdoc/AVRLibcReferenceManual/group__avr__stdlib_1ga060c998e77fb5fc0d3168b3ce8771d42.html
    
    Created on April 8 2019 by Peter Dalmaris

*/

#include "BluetoothSerial.h"
#include <LiquidCrystal_I2C.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#define SEALEVELPRESSURE_HPA (1013.25)

Adafruit_BME280 bme; // I2C

LiquidCrystal_I2C lcd(0x27, 16, 2); // If this address is not working for your I2C backpack,
// run the address scanner sketch to determine the actual
// address.

BluetoothSerial SerialBT;

int lazy_counter = 0; // We'll use this variable to control how often we
// transmit sensor data to the serial monitor
int char_counter = 0;
int row = 0;
int col = 0;

const byte max_rows = 1;  //This represents row id, first row is zero
const byte max_cols = 15; //This represents col id, first col is zero

void setup() {
  Serial.begin(115200);
  SerialBT.begin("ESP32 BT Display"); //Bluetooth device name
  Serial.println("The device started, now you can pair it with bluetooth!");

  bme.begin(0x76);   // BME280 sensors are usually set to address 0x76 or 0x77
  // If your BME280 sensor module has an SD0 pin, then:
  // SD0 unconnected configures the address to 0x77
  // SD0 to GND configures the address to 0x76
  // If there is no SD0 pin, try either address to find out which
  // one works with your sensor.

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Ready to receive");
  delay(5000);
}

void loop() {
  if (SerialBT.available()) {
    if (col == 0 && row == 0)
      reset_lcd();

    int ascii_code_received = SerialBT.read(); // the read() function returns the ASCII code of the letter received
    lcd.setCursor(col, row);
    lcd.print(char(ascii_code_received));
    col++;
    
    if (col > max_cols)  // Reached the edge of the row
    {
      col = 0;
      row++;
    }

    if (row > max_rows)   // Reached the edge of the display
      reset_lcd();    

    if (ascii_code_received == 126) // ASCI 126 is "~"
      reset_lcd();
  }

  // I only want to get an update from the sensor occassionaly.
  // With a lazy counter at 200, the updates will occur 
  // every 200 * 50ms = 10,000 ms approximately 
  if (lazy_counter == 200) // send over the sensor readings
  {
    tx_sensor_values();
    lazy_counter = 0;
  }

  lazy_counter++;
  delay(50);
}

void reset_lcd()
{
  col = 0;
  row = 0;
  lcd.clear();
}

void tx_sensor_values()
{
  const uint8_t temp_label[] = "Temperature: ";
  const uint8_t humi_label[] = " Humidity: ";    // The space at the start of the string is intentional
  char temp[5]; // Buffer big enough for 5-character float
  char humi[5]; // Buffer big enough for 5-character float
  dtostrf(bme.readTemperature(), 4, 2, temp);
  dtostrf(bme.readHumidity(), 4, 2, humi);

  SerialBT.write(10);  // 10 is ASCI code for new line
  SerialBT.write(temp_label, sizeof(temp_label));
  for (int i = 0; i < sizeof(temp); i++)
    SerialBT.write(temp[i]);
  
  SerialBT.write(10);  // 10 is ASCI code for new line
  SerialBT.write(humi_label, sizeof(humi_label));
  
  for (int i = 0; i < sizeof(humi); i++) // The write function needs a single 8-bit int,
    SerialBT.write(humi[i]);
  
  SerialBT.write(10);
}
