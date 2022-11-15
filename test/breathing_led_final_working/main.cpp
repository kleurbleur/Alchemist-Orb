#include <Arduino.h>
#include <NeoPixelBus.h>
#include <Ramp.h> 
#include <WiFi.h>

// settings
const int max_intensity = 50;    // max of 255
const int min_intensity = 5;     
const int duration = 3000;       // time it takes to go back and forth between max and min intensity
const int inbetween_time = 12;   // off time in between the 3 individual white leds in a smd5050


// amount of pixels we have on the strip
const uint16_t PixelCount = 81; // 6 deksel + 5x15 = 81

// make sure to set this to the correct pins
const uint8_t DotClockPin = 12;
const uint8_t DotDataPin = 13;  
const int8_t DotChipSelectPin = -1;

// Hardware SPI on 20MHz. Most efficient and speedy method available. 
NeoPixelBus<DotStarBgrFeature, DotStarSpi20MhzMethod> strip(PixelCount);

// define the ramps
ramp white_1_ramp;
ramp white_2_ramp;
ramp white_3_ramp;


void setup()
{
    Serial.begin(115200);
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
    white_1_ramp.go(min_intensity);                               // make sure to start at 0
    white_1_ramp.go(max_intensity, duration, SINUSOIDAL_INOUT, BACKANDFORTH);    // go to value 30 in 1000ms in a linear line and looping up and down

    white_2_ramp.setGrain(1);                         // set grain to 1 ms
    white_2_ramp.go(min_intensity);                               // make sure to start at 0
    white_2_ramp.go(max_intensity, duration, SINUSOIDAL_INOUT, BACKANDFORTH);    // go to value 30 in 1000ms in a linear line and looping up and down

    white_3_ramp.setGrain(1);                         // set grain to 1 ms
    white_3_ramp.go(min_intensity);                               // make sure to start at 0
    white_3_ramp.go(max_intensity, duration, SINUSOIDAL_INOUT, BACKANDFORTH);    // go to value 30 in 1000ms in a linear line and looping up and down       

}


void loop()
{

        int white_1_i = white_1_ramp.update();
        Serial.print("white_1_i: ");
        Serial.println(white_1_i);
        strip.ClearTo(RgbColor(0, white_1_i, 0));
        strip.Show();

        delay(inbetween_time);

        int white_2_i = white_2_ramp.update();
        Serial.print("white_2_i: ");
        Serial.println(white_2_i);
        strip.ClearTo(RgbColor(white_2_i, 0, 0));
        strip.Show();

        delay(inbetween_time);

        int white_3_i = white_3_ramp.update();
        Serial.print("white_3_i: ");
        Serial.println(white_3_i);
        strip.ClearTo(RgbColor(0, 0, white_3_i));
        strip.Show();

        delay(inbetween_time);

}

