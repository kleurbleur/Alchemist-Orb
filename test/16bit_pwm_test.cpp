#include <Arduino.h>
#include <Ramp.h>

const int ledPin = 19;  // 16 corresponds to GPIO16
// setting PWM properties
const int freq = 300;
const int ledChannel = 0;
const int resolution = 16;

// setup the ramps
rampInt pwm_ramp;

void setup(){
  Serial.begin(115200);
  // configure LED PWM functionalitites
  ledcSetup(ledChannel, freq, resolution);
  // attach the channel to the GPIO to be controlled
  ledcAttachPin(ledPin, ledChannel);

    pwm_ramp.setGrain(1);                                       // set grain to 1 ms
    pwm_ramp.go(0);                                             // make sure to start at 0
    pwm_ramp.go(65535, 40000, LINEAR, FORTHANDBACK);    // go to value 30 in 1000ms in a linear line and looping up and down


}

void loop(){
    int dc = pwm_ramp.update();
    Serial.print("duty cycle: ");
    Serial.println(dc);
    ledcWrite(ledChannel, dc);
//    delay(15);
  }