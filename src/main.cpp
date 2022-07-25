#include <Arduino.h>
#include <NeoPixelBus.h>
#include <Ramp.h> 
#include <StopWatch.h>
// #include <Sherlocked.h>


// settings
const int max_intensity = 10;    // max of 255
const int min_intensity = 1;     
const int duration = 1000;       // time it takes to go back and forth between max and min intensity
const int inbetween_time = 30;   // off time in between the 3 individual white leds in a smd5050


// let strip settings
const uint16_t PixelCount = 144; 
const uint8_t DotClockPin = 14;
const uint8_t DotDataPin = 13;  
const int8_t DotChipSelectPin = -1;
// Hardware SPI on 20MHz. Most efficient and speedy method available. 
NeoPixelBus<DotStarBgrFeature, DotStarSpi20MhzMethod> strip(PixelCount);

// setup the ramps
ramp white_1_ramp;
ramp white_2_ramp;
ramp white_3_ramp;

// define the timing
StopWatch sw;

// Here starts the ACE/MQTT implementation

// #define dbf Serial.printf

// * The commandCallback function is called (activated) when a new 'cmd' or 'info' command is received */
// void commandCallback(int meth, int cmd, const char * value, int triggerID)
// {
//   switch (cmd)
//   {
//     case CMD_RESET:
//       dbf("Received Puzzle RESET from Sherlocked\n");
//         // resetPuzzle();
//       break;

//     case CMD_REBOOT:
//       dbf("Received Reboot from Sherlocked\n");
// //      ESP.restart();
//       break;

//     case CMD_SYNC:
//       dbf("Sync Command not implemented for this board\n");
//       break;

//     case CMD_OTA:
//     dbf("OTA Firmware update command\n");
//       // value contains the file URL for the OTA command
//       // doOTA(value);
//       break;

//     case INFO_SYSTEM:
//         // Expects to receive back system info such as local IP ('ip'), Mac address ('mac') and Firmware Version ('fw')
//       break;

//     case INFO_STATE:
//         pubMsg(Sherlocked.sendState(getState(), T_REQUEST));
//       break;

//     case INFO_FULLSTATE:
//       // Expects to receive back a full state with all relevant inputs and outputs
//       break;
//   }
// }


void setup()
{

    // /* Set the name for this controller, this should be unqiue within */
    // Sherlocked.setName("orb");
    // /* Set callback functions for the various messages that can be received by the Sherlocked ACE system */    
    // /* Puzzle State Changes are handled here */
    // Sherlocked.setStateCallback(stateCallback);
    // /* General Command and Info Messages */
    // Sherlocked.setCommandCallback(commandCallback);
    // /* Inputs and Outputs */
    // Sherlocked.setInputCallback(inputCallback);
    // Sherlocked.setOutputCallback(outputCallback);
    // /* Catch-all callback for json messages that were not handled by other callbacks */
    // Sherlocked.setJSONCallback(jsonCallback);

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

    // start the timer
    sw.start();    
}


void loop()
{

    // ACE/MQTT Stuff
    // if (Serial.available()) {
    //     String incomingMqttMessage = Serial.readStringUntil('\n');
    //     // The parser accepts a char * (c string)
    //     char msgBuf[1024];
    //     strcpy(msgBuf, incomingMqttMessage.c_str());
    //     Sherlocked.parse(msgBuf);
    // }
    // /* Keep server up to date about input and output changes*/
    // static uint32_t lastSend = 0;
    // if(millis() - lastSend > 30000)
    // {
    //     lastSend = millis();
    //     int inputId = 1;
    //     int inputValue = random(0, 255);
    //     pubMsg(Sherlocked.sendInput(inputId, inputValue, T_INPUT));
    // }





    if (sw.elapsed() >= inbetween_time) {
        int white_1_i = white_1_ramp.update();
        Serial.print("white_1_i: ");
        Serial.println(white_1_i);
        strip.ClearTo(RgbColor(0, white_1_i, 0));
        strip.Show();
    }

    if (sw.elapsed() >= (inbetween_time * 2)) {
        int white_2_i = white_2_ramp.update();
        Serial.print("white_2_i: ");
        Serial.println(white_2_i);
        strip.ClearTo(RgbColor(white_2_i, 0, 0));
        strip.Show();
    }

    if (sw.elapsed() >= (inbetween_time * 3)) {
        int white_3_i = white_3_ramp.update();
        Serial.print("white_3_i: ");
        Serial.println(white_3_i);
        strip.ClearTo(RgbColor(0, 0, white_3_i));
        strip.Show();
        sw.reset();
        sw.start();
    }   

}

