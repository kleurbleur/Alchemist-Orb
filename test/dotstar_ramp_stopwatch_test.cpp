#include <Arduino.h>
#include <NeoPixelBus.h>
#include <Ramp.h> 
#include <StopWatch.h>

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

// define the ramps
ramp white_1_ramp;
ramp line_1;
ramp white_2_ramp;
ramp white_3_ramp;
ramp white_all_ramp;

// define the timing
StopWatch sw;

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
    strip.ClearTo(0);
    strip.Show();

    Serial.println();
    Serial.println("Running...");

    white_1_ramp.setGrain(1);                         // set grain to 1 ms
    white_1_ramp.go(0);                               // make sure to start at 0
    white_1_ramp.go(30, 1000, LINEAR, BACKANDFORTH);    // go to value 30 in 1000ms in a linear line and looping up and down
    
    // start the timer
    sw.start();    


}

void line_animation_1(){
        int white_1_i = white_1_ramp.update();
        int line_1_i = line_1.update();
        Serial.println(white_1_i);

        // Set up the whites for the strip
        // RgbColor white_1(0, white_1_i, 0);
        // RgbColor white_2(white_2_i, 0, 0);
        // RgbColor white_3(0, 0, white_3_i);
        // RgbColor white(intensity);
        // RgbColor black(0);

        strip.ClearTo(white_1_ramp.update());
        strip.Show();
}

void loop()
{

    if (sw.elapsed() >= 2000) {
        line_animation_1();
    }


}

