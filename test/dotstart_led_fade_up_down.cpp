#include <Arduino.h>
#include <NeoPixelBus.h>

// amount of pixels we have on the strip
const uint16_t PixelCount = 144; 

// make sure to set this to the correct pins
const uint8_t DotClockPin = 14;
const uint8_t DotDataPin = 13;  
const int8_t DotChipSelectPin = -1;

// set the intesity of the strip
#define intensity 40 //255 is max

// Hardware SPI on 20MHz. Most efficient and speedy method available. 
NeoPixelBus<DotStarBgrFeature, DotStarSpi20MhzMethod> strip(PixelCount);


RgbColor white1(0, intensity, 0);
RgbColor white2(intensity, 0, 0);
RgbColor white3(0, 0, intensity);
RgbColor white(intensity);
RgbColor black(0);

void setup()
{
    Serial.begin(9600);
    while (!Serial); // wait for serial attach

    Serial.println();
    Serial.println("Initializing...");
    Serial.flush();

    // begin the strip communication with the desired pins, since it's an ESP32 we can use all we want. 
    strip.Begin(DotClockPin, DotDataPin, DotDataPin, DotChipSelectPin);
    
    // this resets all the neopixels to an off state
    strip.ClearTo(black);
    strip.Show();

    Serial.println();
    Serial.println("Running...");
}


void loop()
{

int x = 1;
for (int i = 1; i > 0; i = i + x){
    Serial.print("intensity: ");  
    Serial.println(i);
    strip.ClearTo(RgbColor(i, i, i));
    strip.Show();
    if (i == 5) {
        x = -1;
        }
    delay(150); // 40ms is the lowest smooth grade
 }

}