/*  10.060 - Web server

   This example sketch creates a web server that is running on the ESP32.

   Use a browser to connect to the ESP32 webserver. The landing page shows
   the temperature and humidity. You can click on the hyperlinks to control the
   state of the built-in LED (GPIO2)

   This sketch was written by Peter Dalmaris using information from the
   ESP32 datasheet and examples. You can find the original example notes below. I have
   extended the example to show the BME280 readings.


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
    - WiFi
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
    2. WiFi library: https://github.com/espressif/arduino-esp32/tree/master/libraries/WiFi
    3. Web server library: https://github.com/espressif/arduino-esp32/tree/master/libraries/WebServer
    4. Learn about HTTP: https://en.wikipedia.org/wiki/Hypertext_Transfer_Protocol

    Created on April 8 2019 by Peter Dalmaris

*/

/*  ORIGINAL EXAMPLE NOTES

  WiFi Web Server LED Blink

  A simple web server that lets you blink an LED via the web.
  This sketch will print the IP address of your WiFi Shield (once connected)
  to the Serial monitor. From there, you can open that address in a web browser
  to turn on and off the LED on pin 5.

  If the IP address of your shield is yourAddress:
  http://yourAddress/H turns the LED on
  http://yourAddress/L turns it off

  This example is written for a network using WPA encryption. For
  WEP or WPA, change the Wifi.begin() call accordingly.

  Circuit:
   WiFi shield attached
   LED attached to pin 5

  created for arduino 25 Nov 2012
  by Tom Igoe

  ported for sparkfun esp32
  31.01.2017 by Jan Hendrik Berlin

*/

#include <WiFi.h>

#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#define SEALEVELPRESSURE_HPA (1013.25)

Adafruit_BME280 bme; // I2C

const int led_gpio = 2;

const char* ssid     = "ardwifi";
const char* password = "ardwifi987";

WiFiServer server(80);

void setup()
{
  Serial.begin(115200);

  bool status;

  status = bme.begin(0x76);   // BME280 sensors are usually set to address 0x76 or 0x77
  // If your BME280 sensor module has an SD0 pin, then:
  // SD0 unconnected configures the address to 0x77
  // SD0 to GND configures the address to 0x76
  // If there is no SD0 pin, try either address to find out which
  // one works with your sensor.

  pinMode(led_gpio, OUTPUT);      // set the LED pin mode

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
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  server.begin();

}

int value = 0;

void loop() {
  WiFiClient client = server.available();   // listen for incoming clients

  if (client) {                             // if you get a client,
    Serial.println("New Client.");           // print a message out the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        if (c == '\n') {                    // if the byte is a newline character

          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Refresh: 5");
            client.println();
            client.println("<!DOCTYPE HTML>");
            client.println("<html>");

            // the content of the HTTP response follows the header:
            client.print("<p>Click <a href=\"/H\">here</a> to turn the LED on pin ");
            client.print(led_gpio);
            client.print(" on.</p>");
            client.print("<p>Click <a href=\"/L\">here</a> to turn the LED on pin ");
            client.print(led_gpio);
            client.print(" off.</p>");
            client.print("<hl>");
            client.print("<p>Temperature:");
            client.print(bme.readTemperature());
            client.print(" &deg;C</p>");
            client.print("<p>Humidity:");
            client.print(bme.readHumidity());
            client.println(" %</p>");
            client.println("</html>");

            // The HTTP response ends with another blank line:
            client.println();
            // break out of the while loop:
            break;
          } else {    // if you got a newline, then clear currentLine:
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }

        // Check to see if the client request was "GET /H" or "GET /L":
        if (currentLine.endsWith("GET /H")) {
          digitalWrite(led_gpio, HIGH);               // GET /H turns the LED on
        }
        if (currentLine.endsWith("GET /L")) {
          digitalWrite(led_gpio, LOW);                // GET /L turns the LED off
        }
      }
    }
    // close the connection:
    client.stop();
    Serial.println("Client Disconnected.");
  }
}
