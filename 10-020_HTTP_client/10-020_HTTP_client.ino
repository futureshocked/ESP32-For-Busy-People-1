/*  10.020 - Simple HTTP client

   This sketch receives data via HTTP GET requests to fetch a text file from a web server.

   To try it our, either use it as-is (in which case, your ESP32 will retrieve the text in my
   sample text file), or upload a text file in your own web server and change the host and url strings.

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
    3. The remote text file is at: https://txplore-downloads.s3.amazonaws.com/esp32/test_file.txt
    
    Created on April 8 2019 by Peter Dalmaris

*/

#include <WiFi.h>

const char* ssid     = "Your_ssid";     // change this for your own network
const char* password = "Your_password";  // change this for your own network

const char* host = "txplore-downloads.s3.amazonaws.com";  // Change this for your own server
String url = "/esp32/test_file.txt";                      // Change this for your own file

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
    delay(5000);    

    Serial.print("connecting to ");
    Serial.println(host);

    // Use WiFiClient class to create TCP connections
    WiFiClient client;
    const int httpPort = 80;
    if (!client.connect(host, httpPort)) {
        Serial.println("connection failed");
        return;
    }

    // We now create a URI for the request
    Serial.print("Requesting URL: ");
    Serial.println(url);

    // This will send the request to the server
    client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                 "Host: " + host + "\r\n" +
                 "Connection: close\r\n\r\n");
    unsigned long timeout = millis();
    while (client.available() == 0) {
        if (millis() - timeout > 5000) {
            Serial.println(">>> Client Timeout !");
            client.stop();
            return;
        }
    }

    // Read all the lines of the reply from server and print them to Serial
    while(client.available()) {
        String line = client.readStringUntil('\r');
        Serial.print(line);
    }

    Serial.println();
    Serial.println("closing connection");
}
