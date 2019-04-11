/*  10.010 - Connection to a Wifi network

   This sketch simply connects the ESP32 to a Wifi network.

   Once the connection is made, nothing else happens.

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
    - WiFi (comes with the ESP32 Arduino core)


   Connections
   -----------

    No connections needed, the ESP32 is the only hardware.


    Other information
    -----------------

    1. ESP32 Datasheet: https://www.espressif.com/sites/default/files/documentation/esp32_datasheet_en.pdf
    2. Wifi library: https://github.com/espressif/arduino-esp32/tree/master/libraries/WiFi

    Created on April 8 2019 by Peter Dalmaris

*/

#include <WiFi.h>

const char* ssid     = "ardwifi";     // change this for your own network
const char* password = "ardwifi987";  // change this for your own network

void setup()
{
  Serial.begin(115200);
  delay(10);

  // We start by connecting to a WiFi network

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void loop()
{

}
