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

Freenove_ESP32_WS2812 strip = Freenove_ESP32_WS2812(LEDS_COUNT, LEDS_PIN, CHANNEL, TYPE_GRB);
int delayval = 100;

static bool eth_connected = false;

hw_timer_t *My_timer = NULL;

IPAddress newIP(10,0,20,150);
IPAddress subnet(255,255,255,0);
IPAddress gateway(10,0,20,1);

int panic_on=0;

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
 if (eth_connected)
  {
    //bool success = testClient("10.20.0.100", 80);
  //testClient("10.20.0.100", 80);

  }
  
  strip.setLedColorData(0, 0, 255, 0);
  strip.show();

}

bool testClient(const char *host, uint16_t port)
{
  Serial.print("\nconnecting to ");
  Serial.println(host);

  WiFiClient client;
  if (!client.connect(host, port))
  {
    Serial.println("connection failed");
    return 1;
  }
  client.printf("GET / HTTP/1.1\r\nHost: %s\r\n\r\n", host);
  while (client.connected() && !client.available())
    ;
  while (client.available())
  {
    Serial.write(client.read());
  }

  Serial.println("closing connection\n");
  client.stop();
  return 0;
}


bool switch_on(const char *host, uint16_t port)
{
  Serial.print("\nconnecting to ");
  Serial.println(host);

  WiFiClient client;
  if (!client.connect(host, port))
  {
    Serial.println("connection failed");
    return 1;
  }
  client.printf("GET /switchon HTTP/1.1\r\nHost: %s\r\n\r\n", host);
  while (client.connected() && !client.available())
    ;
  while (client.available())
  {
    Serial.write(client.read());
  }

  Serial.println("closing connection\n");
  client.stop();
  return 0;
}

bool switch_off(const char *host, uint16_t port)
{
  Serial.print("\nconnecting to ");
  Serial.println(host);

  WiFiClient client;
  if (!client.connect(host, port))
  {
    Serial.println("connection failed");
    return 1;
  }
  client.printf("GET /switchoff HTTP/1.1\r\nHost: %s\r\n\r\n", host);
  while (client.connected() && !client.available())
    ;
  while (client.available())
  {
    Serial.write(client.read());
  }

  Serial.println("closing connection\n");
  client.stop();
  return 0;
}
void setup()
{
  Serial.begin(115200);
  WiFi.onEvent(WiFiEvent);

  ETH.begin(ETH_ADDR, ETH_POWER_PIN, ETH_MDC_PIN, ETH_MDIO_PIN, ETH_TYPE, ETH_CLK_MODE);
  ETH.config(newIP,gateway,subnet);
  
  strip.begin();
  strip.setBrightness(10);  

  pinMode(2, INPUT_PULLUP); // set the pin as input

  pinMode(14, INPUT_PULLUP); // set the pin as input

  pinMode(13, OUTPUT); // set the pin as output
  digitalWrite(13, LOW);

  pinMode(15, OUTPUT); // set the pin as output
  digitalWrite(15, LOW);

  My_timer = timerBegin(0,80,true);
  timerAttachInterrupt(My_timer, &onTimer, true);
  timerAlarmWrite(My_timer, 10000000, true);
  timerAlarmEnable(My_timer);
  
}

void loop()
{

int buttonState_on = digitalRead(2);
int buttonState_off = digitalRead(14);

  if (buttonState_on == LOW) {
    delay (500);

    if (buttonState_on == LOW){  
    Serial.println("On Button pressed\n");
    
      if (panic_on==0){
      switch_on("10.0.20.100", 80);
      panic_on=1;
      }
      
    }
  }
  if (buttonState_off == LOW) {
    delay (500);

    if (buttonState_off == LOW){  
    Serial.println("Off Button pressed\n");
    
    if (panic_on==1){
      switch_off("10.0.20.100", 80);
      panic_on=0;
      }
      
    }
  }
  
}
