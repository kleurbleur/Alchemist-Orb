
#include <Arduino.h>
#include <NeoPixelBus.h>
#include <Artnet.h>
#include <WiFi.h>


const char* ssid     = "Wireless Funtime Palace";
const char* password = "radiorijnmond";

// Neopixel settings
const int numLeds = 144; // change for your setup
const int channelsPerLed = 4;
const int numberOfChannels = numLeds * channelsPerLed; // Total number of channels you want to receive (1 led = 3 channels)
const uint16_t PixelPin = 2;            // make sure to set this to the correct pin, ignored for Esp8266
NeoPixelBus<NeoGrbwFeature, NeoEsp32I2s1800KbpsMethod> leds(numLeds, PixelPin);
RgbwColor red(255, 0, 0, 0);
RgbwColor green(0, 255, 0, 0);
RgbwColor blue(0, 0, 255, 0);
RgbwColor white(0, 0, 0, 255);


// Artnet settings
Artnet artnet;
const int startUniverse = 0; // CHANGE FOR YOUR SETUP most software this is 1, some software send out artnet first universe as 0.
int previousDataLength = 0;


void onDmxFrame(uint16_t universe, uint16_t length, uint8_t sequence, uint8_t* data, IPAddress remoteIP)
{
  // read universe and put into the right part of the display buffer
  for (int i = 0; i < length / channelsPerLed; i++)
  {
    int led = i + (universe - startUniverse) * (previousDataLength / channelsPerLed);
    if (led < numLeds) {
        RgbwColor color = (data[i * channelsPerLed], data[i * channelsPerLed + 1], data[i * channelsPerLed + 2], data[i * channelsPerLed + 3]);
        leds.SetPixelColor(led, color);

    }
  }
  leds.Show();
  previousDataLength = length;
}

  void initTest()
  {
    for (int i = 0 ; i < numLeds ; i++)
      leds.SetPixelColor(i, red);
    leds.Show();
    delay(500);
    for (int i = 0 ; i < numLeds ; i++)
      leds.SetPixelColor(i, green);
    leds.Show();
    delay(500);
    for (int i = 0 ; i < numLeds ; i++)
      leds.SetPixelColor(i, blue);
    leds.Show();
    delay(500);
    for (int i = 0 ; i < numLeds ; i++)
      leds.SetPixelColor(i, white);

    leds.Show();
  }

void setup()
{
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  //Serial.begin(115200);
  artnet.begin();
  leds.Begin();
  initTest();
  leds.Show();
  


  // this will be called for each packet received
  artnet.setArtDmxCallback(onDmxFrame);

}


void loop()
{
    artnet.read();
    // strip.Show();
}
