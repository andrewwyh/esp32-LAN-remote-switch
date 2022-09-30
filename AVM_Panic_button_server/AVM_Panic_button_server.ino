#include <Arduino.h>
#include <ETH.h>
#include <SPI.h>
#include <SD.h>
#include "config.h"
#include "Freenove_WS2812_Lib_for_ESP32.h"
#include <WebServer.h>

#define LEDS_COUNT  8
#define LEDS_PIN  12
#define CHANNEL   0
#define RELAYPIN 14
#define REDLED 2
#define GREENLED 13
#define BLINK_INTERVAL 1000

unsigned long previousMillis = 0;
bool greenledState=0;

bool online=0;

Freenove_ESP32_WS2812 strip = Freenove_ESP32_WS2812(LEDS_COUNT, LEDS_PIN, CHANNEL, TYPE_GRB);

int delayval = 100;

static bool eth_connected = false;

WebServer server(80);  // Object of WebServer(HTTP port, 80 is default)
hw_timer_t *My_timer = NULL;

IPAddress newIP(10,0,20,100);
IPAddress subnet(255,255,255,0);
IPAddress gateway(10,0,20,1);

void WiFiEvent(WiFiEvent_t event)
{
#if ESP_IDF_VERSION_MAJOR > 3
    switch (event)
    {
    case ARDUINO_EVENT_ETH_START:
        Serial.println("ETH Started");
        // set eth hostname here
        ETH.setHostname("esp32-ethernet");
        break;
    case ARDUINO_EVENT_ETH_CONNECTED:
        Serial.println("ETH Connected");
        break;
    case ARDUINO_EVENT_ETH_GOT_IP:
        Serial.print("ETH MAC: ");
        Serial.print(ETH.macAddress());
        Serial.print(", IPv4: ");
        Serial.print(ETH.localIP());
        if (ETH.fullDuplex())
        {
            Serial.print(", FULL_DUPLEX");
        }
        Serial.print(", ");
        Serial.print(ETH.linkSpeed());
        Serial.println("Mbps");
        eth_connected = true;
        break;
    case ARDUINO_EVENT_ETH_DISCONNECTED:
        Serial.println("ETH Disconnected");
        eth_connected = false;
        break;
    case ARDUINO_EVENT_ETH_STOP:
        Serial.println("ETH Stopped");
        eth_connected = false;
        break;
    default:
        break;
    }
#elif
    switch (event)
    {
    case SYSTEM_EVENT_ETH_START:
        Serial.println("ETH Started");
        // set eth hostname here
        ETH.setHostname("esp32-ethernet");
        break;
    case SYSTEM_EVENT_ETH_CONNECTED:
        Serial.println("ETH Connected");
        break;
    case SYSTEM_EVENT_ETH_GOT_IP:
        Serial.print("ETH MAC: ");
        Serial.print(ETH.macAddress());
        Serial.print(", IPv4: ");
        Serial.print(ETH.localIP());
        if (ETH.fullDuplex())
        {
            Serial.print(", FULL_DUPLEX");
        }
        Serial.print(", ");
        Serial.print(ETH.linkSpeed());
        Serial.println("Mbps");
        eth_connected = true;
        break;
    case SYSTEM_EVENT_ETH_DISCONNECTED:
        Serial.println("ETH Disconnected");
        eth_connected = false;
        break;
    case SYSTEM_EVENT_ETH_STOP:
        Serial.println("ETH Stopped");
        eth_connected = false;
        break;
    default:
        break;
    }
#endif
}

void IRAM_ATTR onTimer(){
  strip.setLedColorData(0, 255, 0, 0);
  strip.show();
  online = 0;
  //timerEnd(My_timer);
}

void setup()
{
  Serial.begin(115200);
  WiFi.onEvent(WiFiEvent);

  ETH.begin(ETH_ADDR, ETH_POWER_PIN, ETH_MDC_PIN, ETH_MDIO_PIN, ETH_TYPE, ETH_CLK_MODE);
  ETH.config(newIP,gateway,subnet);
  

  strip.begin();
  strip.setBrightness(10);  

  pinMode(RELAYPIN, OUTPUT); // set the pin as output
  digitalWrite(RELAYPIN, LOW);

  pinMode(REDLED, OUTPUT); // set the pin as output
  digitalWrite(REDLED, HIGH);

  pinMode(GREENLED, OUTPUT); // set the pin as output
  digitalWrite(GREENLED, HIGH);
  
  server.on("/", handle_root);
  server.on("/switchon", handle_switchon);
  server.on("/switchoff", handle_switchoff);

  server.begin();
  Serial.println("HTTP server started");
  delay(100); 

   strip.setLedColorData(0, 255, 0, 0);
   strip.show();
   online = 0;
   
   My_timer = timerBegin(0,80,true);
   timerAttachInterrupt(My_timer, &onTimer, true);
   timerAlarmWrite(My_timer, 15000000, true);
   timerAlarmEnable(My_timer);


 
}

void loop()
{

unsigned long currentMillis = millis();

if (online == 1) {
  digitalWrite(GREENLED,0);
}

if (online == 0) {
  if (currentMillis - previousMillis >= BLINK_INTERVAL) {
      greenledState = (greenledState == LOW) ? HIGH : LOW;
      digitalWrite(GREENLED,greenledState);
      previousMillis = currentMillis;
  }
}

server.handleClient();

}

// HTML & CSS contents which display on web server
String HTMLROOT = "<!DOCTYPE html>\
<html>\
<body>\
<h1>My First Web Server with ESP32 - Station Mode &#128522;</h1>\
</body>\
</html>";


// HTML & CSS contents which display on web server
String HTMLSWITCHON = "<!DOCTYPE html>\
<html>\
<body>\
<h1>SWITCH TRIGGERED ON &#128522;</h1>\
</body>\
</html>";

// HTML & CSS contents which display on web server
String HTMLSWITCHOFF = "<!DOCTYPE html>\
<html>\
<body>\
<h1>SWITCH TRIGGERED OFF &#128522;</h1>\
</body>\
</html>";

// Handle root url (/)
void handle_root() {
  server.send(200, "text/html", HTMLROOT);
  strip.setLedColorData(0, 0, 255, 0);
  strip.show();
  online = 1;
  timerRestart(My_timer);

}

// Handle switch url (/switchon)
void handle_switchon() {
  server.send(200, "text/html", HTMLSWITCHON);
  digitalWrite(RELAYPIN, HIGH); 
}

// Handle switch url (/switchoff)
void handle_switchoff() {
  server.send(200, "text/html", HTMLSWITCHOFF);
  digitalWrite(RELAYPIN, LOW);
}
