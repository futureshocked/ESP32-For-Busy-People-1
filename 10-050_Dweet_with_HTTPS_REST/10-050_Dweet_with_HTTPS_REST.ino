/*  10.050 - Dweet using HTTPS REST client

   This example sketch reads values from the BME280 and sends it to Dweet.io using secure HTTP.

   From Dweet.io, the data is picked up by freeboard.io and visualised in a dashboard.

   To read your Dweets, point your browser to https://dweet.io/get/dweets/for/peterslab.
   
   To see the dashboard, go to https://freeboard.io/board/8FbEWD. Note, this dashboard may be offline
   when you try to access it.

   This sketch was written by Peter Dalmaris using information from the
   ESP32 datasheet and examples.


   Components
   ----------
    - ESP32 Dev Kit v4
    - BME280

    IDE
    ---
    Arduino IDE with ESP32 Arduino Code
    (https://github.com/espressif/arduino-esp32)


    Libraries
    ---------
    - WiFiClientSecure (comes with the ESP32 Arduino core)


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
    2. WiFiClientSecure library: https://github.com/espressif/arduino-esp32/tree/master/libraries/WiFiClientSecure
    3. Learn about Dweet.io: https://dweet.io
    4. Learn about freeboard.io: https://freeboard.io
    5. Curl tutorial (good for testing Dweets): https://curl.haxx.se/docs/httpscripting.html#GET

    Created on April 8 2019 by Peter Dalmaris

*/

#include <WiFiClientSecure.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#define SEALEVELPRESSURE_HPA (1013.25)

Adafruit_BME280 bme; // I2C

const char* ssid     = "ardwifi";     // change this for your own network
const char* password = "ardwifi987";  // change this for your own network

const char*  server = "dweet.io";      
String url = "/dweet/for/peterslab?";  // Do not remove the "?"


//Dweet root certificate
const char* dweetio_root_ca = \
                              "-----BEGIN CERTIFICATE-----\n" \
                              "MIIDQTCCAimgAwIBAgITBmyfz5m/jAo54vB4ikPmljZbyjANBgkqhkiG9w0BAQsF\n" \
                              "ADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6\n" \
                              "b24gUm9vdCBDQSAxMB4XDTE1MDUyNjAwMDAwMFoXDTM4MDExNzAwMDAwMFowOTEL\n" \
                              "MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv\n" \
                              "b3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj\n" \
                              "ca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvo3bSMgHFzZM\n" \
                              "9O6II8c+6zf1tRn4SWiw3te5djgdYZ6k/oI2peVKVuRF4fn9tBb6dNqcmzU5L/qw\n" \
                              "IFAGbHrQgLKm+a/sRxmPUDgH3KKHOVj4utWp+UhnMJbulHheb4mjUcAwhmahRWa6\n" \
                              "VOujw5H5SNz/0egwLX0tdHA114gk957EWW67c4cX8jJGKLhD+rcdqsq08p8kDi1L\n" \
                              "93FcXmn/6pUCyziKrlA4b9v7LWIbxcceVOF34GfID5yHI9Y/QCB/IIDEgEw+OyQm\n" \
                              "jgSubJrIqg0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMC\n" \
                              "AYYwHQYDVR0OBBYEFIQYzIU07LwMlJQuCFmcx7IQTgoIMA0GCSqGSIb3DQEBCwUA\n" \
                              "A4IBAQCY8jdaQZChGsV2USggNiMOruYou6r4lK5IpDB/G/wkjUu0yKGX9rbxenDI\n" \
                              "U5PMCCjjmCXPI6T53iHTfIUJrU6adTrCC2qJeHZERxhlbI1Bjjt/msv0tadQ1wUs\n" \
                              "N+gDS63pYaACbvXy8MWy7Vu33PqUXHeeE6V/Uq2V8viTO96LXFvKWlJbYK8U90vv\n" \
                              "o/ufQJVtMVT8QtPHRh8jrdkPSHCa2XV4cdFyQzR1bldZwgJcJmApzyMZFo6IQ6XU\n" \
                              "5MsI+yMRQ+hDKXJioaldXgjUkK642M4UwtBV8ob2xJNDd2ZhwLnoQdeXeGADbkpy\n" \
                              "rqXRfboQnoZsG4q5WTP468SQvvG5\n" \
                              "-----END CERTIFICATE-----\n";


WiFiClientSecure client;

void setup() {
  //Initialize serial and wait for port to open:
  Serial.begin(115200);
  delay(100);

  bme.begin(0x76);   // BME280 sensors are usually set to address 0x76 or 0x77
                              // If your BME280 sensor module has an SD0 pin, then:
                              // SD0 unconnected configures the address to 0x77
                              // SD0 to GND configures the address to 0x76
                              // If there is no SD0 pin, try either address to find out which
                              // one works with your sensor.

  Serial.print("Attempting to connect to SSID: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

}

void loop() {
  // attempt to connect to Wifi network:
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    // wait 1 second for re-trying
    delay(1000);
  }

  Serial.print("Connected to ");
  Serial.println(ssid);

  client.setCACert(dweetio_root_ca);

  // Create the GET request string
  String get_request = "GET https://";
  get_request += server;
  get_request += url;
  get_request += "temp=";
  get_request += bme.readTemperature();
  get_request += "&hum=";
  get_request += bme.readHumidity();
  get_request += " HTTP/1.0";

  Serial.println("The GET request will be:");
  Serial.println(get_request);

  Serial.println("\nStarting connection to server...");
  if (!client.connect(server, 443))
    Serial.println("Connection failed!");
  else {
    Serial.println("Connected to server!");
    // Make a HTTPS request:
    client.println(get_request);    
    client.println(String("Host: ") + server);
    client.println("Connection: close");
    client.println();

    while (client.connected()) {
      String line = client.readStringUntil('\n');
      if (line == "\r") {
        Serial.println("headers received");
        break;
      }
    }
    // if there are incoming bytes available
    // from the server, read them and print them:
    while (client.available()) {
      char c = client.read();
      Serial.write(c);
    }

    client.stop();
  }

  delay(20000);  // Update every 20 seconds so that Dweet.io does not complain.
                 // Consider changing this to a timer interrupt.
}
