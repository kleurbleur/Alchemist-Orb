// simple test for usage of the dotstart strip plus high strobe to see if we can manage to get the power usage down 
// seems to 

#include <NeoPixelBus.h>
#include <Arduino.h>

const uint16_t PixelCount = 144; // amount of pixels we have on the strip

// make sure to set this to the correct pins
const uint8_t DotClockPin = 14;
const uint8_t DotDataPin = 13;  
const int8_t DotChipSelectPin = -1;

#define intensity 255 //255 is max

// Hardware SPI on 20MHz. Most efficient and speedy method available. 
NeoPixelBus<DotStarBgrFeature, DotStarSpi20MhzMethod> strip(PixelCount);

const int on_time = 1500;
const int off_time = 1500;


RgbColor white1(0, intensity, 0);
RgbColor white2(intensity, 0, 0);
RgbColor white3(0, 0, intensity);
RgbColor white(intensity);
RgbColor black(0);

void setup()
{
    // Serial.begin(115200);
    // while (!Serial); // wait for serial attach

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
    Serial.println("WHITE1 - top");
    strip.ClearTo(white1);                  // set the whole strip to a RGB color, in this case WWW. 
    strip.Show();                           // show the current led strip color
    delayMicroseconds(on_time);

    Serial.println("Off ...");
    strip.ClearTo(black);                   // turn the strip off
    strip.Show();
    delayMicroseconds(off_time);    

    Serial.println("WHITE2 - middle");    
    strip.ClearTo(white2);
    strip.Show();
    delayMicroseconds(on_time);   

    Serial.println("Off ...");
    strip.ClearTo(black);                   // turn the strip off
    strip.Show();
    delayMicroseconds(off_time);     

    Serial.println("WHITE3 - bottom");    
    strip.ClearTo(white3);
    strip.Show();
    delayMicroseconds(on_time);

    Serial.println("Off ...");
    strip.ClearTo(black);                   // turn the strip off
    strip.Show();
    delayMicroseconds(off_time);    

    Serial.println("WHITE - all");    
    strip.ClearTo(white);
    strip.Show();
    delayMicroseconds(on_time);

    Serial.println("Off ...");
    strip.ClearTo(black);                   // turn the strip off
    strip.Show();
    delayMicroseconds(off_time);

}