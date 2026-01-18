/*  11.010 - Classic Bluetooth

   This example sketch shows how to create a wireless serial channel using
   classic Bluetooth.

   After you upload the sketch to your ESP32, connect to the BT serial port
   on your computer.

   Then, use a serial monitor to interact with the ESP32.

   When you type something in the USB serial monitor, it will bounce back
   via the Bluetooth serial monitor.

   This sketch was written by Peter Dalmaris using information from the
   ESP32 datasheet and examples.


   Components
   ----------
    - ESP32 Dev Kit v4

    IDE
    ---
    Arduino IDE with ESP32 Arduino Code
    (https://github.com/espressif/arduino-esp32)


    Libraries
    ---------


   Connections
   -----------

   There is no external hardware. 


    Other information
    -----------------

    1. ESP32 Datasheet: https://www.espressif.com/sites/default/files/documentation/esp32_datasheet_en.pdf
    2. Bluetooth Serial: https://github.com/espressif/arduino-esp32/blob/master/libraries/BluetoothSerial/src/BluetoothSerial.h
    3. ASCII code table: http://www.asciitable.com/   
    4. Bluetooth article on Wikipedia: https://en.wikipedia.org/wiki/Bluetooth#Bluetooth_4.0 

    Created on April 8 2019 by Peter Dalmaris

*/

//This example code is in the Public Domain (or CC0 licensed, at your option.)
//By Evandro Copercini - 2018
//
//This example creates a bridge between Serial and Classical Bluetooth (SPP)
//and also demonstrate that SerialBT have the same functionalities of a normal Serial

#include "BluetoothSerial.h"

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

BluetoothSerial SerialBT;

void setup() {
  Serial.begin(115200);
  SerialBT.begin("ESP32test"); //Bluetooth device name
  Serial.println("The device started, now you can pair it with bluetooth!");
}

void loop() {
  if (Serial.available()) {
    SerialBT.write(Serial.read());
  }
  if (SerialBT.available()) {
    Serial.write(SerialBT.read());
  }
  delay(20);
}
