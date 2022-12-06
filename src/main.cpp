// ONGOING DEV: - making two Wire busses -> https://randomnerdtutorials.com/esp32-i2c-communication-arduino-ide/
#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <Sherlocked.h>
#include <NeoPixelBus.h>
#include <Ramp.h> 



// Firmware version
char firmware_version[] = "0.9";                                // more robust wifi reconnect and signal strength indicator

// SETTINGS

// mqtt/ace settings
char hostname[] ="controller";                                  // the hostname for board  <= REPLACE
const char gen_topic[] = "alch";  
const char puzzle_topic[] = "alch/orb"; 
const char module_topic[] = "alch/orb/controller";              // the module name of the board <= REPLACE
const char* ssid = "Wireless Funtime Palace";
const char* password = "radiorijnmond";
IPAddress server(192, 168, 178, 213);                           // ip address of the mqtt/ace server <= REPLACE


// controller settings
// settings
u_int8_t flicker_time = 12;   // off time in between the 3 individual white leds in a smd5050
u_int8_t max_intensity = 50;    // max of 255
u_int8_t min_intensity = 5;     
u_int32_t duration = 3000;       // time it takes to go back and forth between max and min intensity


// amount of pixels we have on the strip
const uint16_t PixelCount = 81; // 6 deksel + 5x15 = 81



// INPUTS and OUTPUTS
// These needs to correspond to the document at https://docs.google.com/document/d/1GiCjMT_ph-NuIOsD4InIvT-H3MmUkSkzBZRMM1L5IsI/edit#heading=h.wqfd6v7o79qu

// how many do we have and do they need to start at a certain id? 
#define NUM_OUTPUTS 10           // amount of outputs
#define START_OUTPUT 1          // the start number of the output
#define NUM_INPUTS 0           // amount of inputs
#define START_INPUT 0          // the start number of the input

// pin assignment
// IN
// - 
// OUT
#define DotClockPin 12              
#define DotDataPin 13            
#define DotChipSelectPin -1   


// lets put these pins in an array
// int _inputsPins[1] = {
//   motor_control_A_top_pin,
// };
// lets calculate the size of this array so we can loop through it later on
// int _inputsPinsArraySize = sizeof(_inputsPins)/sizeof(int);

// naming the IDs and values
// these names are now numbered and can be used when calling an array numbers. Instead of inIDs[2] now inIDs[arm_A_controller_top]
// enum inputs{
//   TOP_CONTROLLER_ARM_A,
// };
enum outputs{
    BREATH_MODE_ENABLE,
    BREATH_FLICKER_TIME,
    BREATH_MAX_INTENSITY,
    BREATH_MIN_INTENSITY,
    BREATH_DURATION,
    FINALE_MODE_ENABLE,
    FINALE_FLICKER_TIME,
    FINALE_MAX_INTENSITY,
    FINALE_MIN_INTENSITY,
    FINALE_DURATION    
};

// a struct to create more overview for the inputs
// struct input {
//     uint8_t id;
//     uint8_t value;
//     uint8_t pin;
//     uint32_t current_debounce;
//     uint32_t last;
//     uint32_t debounce;
// };

// lets create those struct based on the amount of inputs we have
// input _input[NUM_INPUTS];

// let's initiate those inputs and give them id's and connect them to the physival pins
// void initInputs()
// {
//     Serial.println("Setup commencing");
//     for (int i = 0; i < NUM_INPUTS; i++)
//     {
//         Serial.printf("Setup %i \n", i);
//         _input[i].id = i+START_INPUT;
//         if (i < _inputsPinsArraySize)
//         {
//             _input[i].debounce = 10;
//             _input[i].last = millis();
//             _input[i].pin = _inputsPins[i];
//             Serial.printf("input pin found! %i \n", _input[i].pin);
//             pinMode(_input[i].pin, INPUT_PULLUP);
//         } 
//     }
// }


// and here we create the id and the value arrays which are callable by e.g. outIDs[led_hole] (which is 15)
int outIDs[NUM_OUTPUTS];
int outValues[NUM_OUTPUTS] = {0};
// and two helper functions to set the id's of the out- and inputs.
void setOutputsNum()
{
  Serial.print("Outputs[value]: {");
  for(int i = 0; i < NUM_OUTPUTS; i++)
  {
    outIDs[i] = i+START_OUTPUT;
    Serial.print(i+START_OUTPUT);
    Serial.printf("[%i]", outValues[i]);    
    if (i < NUM_OUTPUTS-1)
    {    
      Serial.print(", ");
    }
    if (i >= NUM_OUTPUTS-1)
    {
      Serial.println("}");
    }
  }
}
// void setInputsNum()
// {
//   Serial.print("Inputs: {");
//   for(int i = 0; i < NUM_INPUTS; i++)
//   {
//     inIDs[i] = i+START_INPUT;
//     Serial.print(i+START_INPUT);
//     Serial.printf("[%i]", inValues[i]);
//     if (i < NUM_INPUTS-1)
//     {
//       Serial.print(", ");
//     }
//     if (i >= NUM_INPUTS-1)
//     {
//       Serial.println("}");
//     }    
//   }
// }

// INPUT OUTPUT END ---
//  that's it, now we can call for example _input[CONTROLLER_ARM_A_TARGET_TOP].pin or _input[CONTROLLER_ARM_A_TARGET_TOP].value


// SETUP LIBS
WiFiClient espClient;
PubSubClient client(espClient);
// Hardware SPI on 20MHz. Most efficient and speedy method available. 
NeoPixelBus<DotStarBgrFeature, DotStarSpi20MhzMethod> strip(PixelCount);

// define the ramps
ramp white_1_ramp;
ramp white_2_ramp;
ramp white_3_ramp;



// variables 
static bool eth_connected = false;
unsigned long previousMicros = 0;
unsigned long currentMicros;
unsigned long currentDebounceMillis;
unsigned long last_debounce_time = 0;
u_int8_t flicker_time_2 = flicker_time * 0.33;
u_int8_t flicker_time_3 = flicker_time * 0.8;

u_int8_t breathing_flicker_time = 12;   // off time in between the 3 individual white leds in a smd5050
u_int8_t breathing_max_intensity = 50;    // max of 255
u_int8_t breathing_min_intensity = 5;     
u_int32_t breathing_duration = 3000;       // time it takes to go back and forth between max and min intensity
u_int8_t finale_flicker_time = 5;   // off time in between the 3 individual white leds in a smd5050
u_int8_t finale_max_intensity = 255;    // max of 255
u_int8_t finale_min_intensity = 25;     
u_int32_t finale_duration = 30000;       // time it takes to go back and forth between max and min intensity



// the ethernet function
char localIP[16];
char macAddress[18];
int getWifiStrength(int points){
    long rssi = 0;
    long averageRSSI = 0; 
    for (int i=0;i < points;i++){
        rssi += WiFi.RSSI();
        delay(20);
    }
    averageRSSI = rssi/points;
    return averageRSSI;
}
void initWiFi() {
    WiFi.disconnect();
    WiFi.mode(WIFI_OFF);
    delay(100);
    WiFi.mode(WIFI_STA);
    WiFi.persistent(false);
    delay(100);
    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi ..");
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print('.');
        delay(1000);
    }
    WiFi.localIP().toString().toCharArray(localIP, 16);
    strncpy( macAddress, WiFi.macAddress().c_str(), sizeof(macAddress) );
    Serial.println(localIP);
    Serial.println(macAddress);
    Serial.print("Wifi strength: ");
    Serial.println(getWifiStrength(10));
}

// the publish function for ACE/mqtt
void pubMsg(char msg[])
{
  Serial.print("pubMsg: ");
  Serial.println(msg);
  client.publish(puzzle_topic, msg);
}
// declare function prototypes 
void pubMsg_kb(const char * method, const char *param1=(char*)'\0', const char *val1=(char*)'\0', const char *param2=(char*)'\0', const char *val2=(char*)'\0' );
// actual function
void pubMsg_kb(const char * method, const char *param1, const char *val1, const char *param2, const char *val2)
{
  char jsonMsg[200], arg1[200], arg2[200];
  if (param1 && val1){
    if (val1[0]=='{' || val1[0]=='['){
      sprintf(arg1, ",\"%s\":%s", param1, val1); 
    } else {
      sprintf(arg1, ",\"%s\":\"%s\"", param1, val1); 
    }
  }
  else if (param1 && !val1){
    Serial.println("function pubMsg: Please supply a value with parameter 1");
  }
  else if (strcmp(param1,nullptr) && strcmp(val1,nullptr) ){
    memset(arg1, 0, 50);
  }  

  if (param2 && val2){
    if (val2[0]=='{' || val2[0]=='['){
      sprintf(arg2, ",\"%s\":%s", param2, val2); 
    } else {
      sprintf(arg2, ",\"%s\":\"%s\"", param2, val2); 
    }  }
  else if (param2 && !val2){
    Serial.println("function pubMsg: Please supply a value with parameter 2");
  }
  else if (strcmp(param2,nullptr) && strcmp(val2,nullptr) ){
    memset(arg2, 0, 50);
  }  

  sprintf(jsonMsg, "{\"sender\":\"%s\",\"method\":\"%s\"%s%s}", hostname, method, arg1, arg2);
  Serial.println(jsonMsg);
  client.publish(puzzle_topic, jsonMsg);
}

void setOutput(int outID, int value)
{
    outID = outID +1;
    outValues[outID] = value;
};
void setOutputWithMessage(int outID, int value, const char* request)
{
    outValues[outID] = value;
    DynamicJsonBuffer  jsonBuffer(200);
    JsonObject& root = jsonBuffer.createObject();
    root["sender"] = hostname;
    root["method"] = "info";
    JsonArray& outputs = root.createNestedArray("outputs");
    JsonObject& outid_val = outputs.createNestedObject();
    outid_val["id"] = outIDs[outID];
    outid_val["value"] = outValues[outID];
    root["trigger"] = request;
    char full_char[250];
    root.prettyPrintTo(full_char, sizeof(full_char));
    pubMsg(full_char);      
}
// void blockMessage(uint8_t requested_id, uint8_t blocking_id)
// {
//     DynamicJsonBuffer  jsonBuffer(200);
//     JsonObject& root = jsonBuffer.createObject();
//     root["sender"] = hostname;
//     root["method"] = "info";
//     JsonArray& outputs = root.createNestedArray("outputs");
//     JsonObject& outid_val = outputs.createNestedObject();
//     outid_val["id"] = outIDs[requested_id];
//     outid_val["value"] = outValues[requested_id];
//     JsonArray& inputs = root.createNestedArray("inputs");
//     JsonObject& inputsid_val = inputs.createNestedObject();
//     inputsid_val["id"] = _input[blocking_id].id;
//     inputsid_val["value"] = _input[blocking_id].value;
//     root["trigger"] = "block";
//     char full_char[250];
//     root.prettyPrintTo(full_char, sizeof(full_char));
//     pubMsg(full_char);  
// };

// OUTPUT FUNCTIONS
// the motor functions
void breathing_mode_enable(int start){
  if (start == 1 )
  {
    flicker_time = breathing_flicker_time;   // off time in between the 3 individual white leds in a smd5050

    white_1_ramp.setGrain(1);                         // set grain to 1 ms
    white_1_ramp.go(breathing_min_intensity);                               // make sure to start at 0
    white_1_ramp.go(breathing_max_intensity, breathing_duration, SINUSOIDAL_INOUT, BACKANDFORTH);    // go to value 30 in 1000ms in a linear line and looping up and down

    white_2_ramp.setGrain(1);                         // set grain to 1 ms
    white_2_ramp.go(breathing_min_intensity);                               // make sure to start at 0
    white_2_ramp.go(breathing_max_intensity, breathing_duration, SINUSOIDAL_INOUT, BACKANDFORTH);    // go to value 30 in 1000ms in a linear line and looping up and down

    white_3_ramp.setGrain(1);                         // set grain to 1 ms
    white_3_ramp.go(breathing_min_intensity);                               // make sure to start at 0
    white_3_ramp.go(breathing_max_intensity, breathing_duration, SINUSOIDAL_INOUT, BACKANDFORTH);    // go to value 30 in 1000ms in a linear line and looping up and down       



    outValues[BREATH_MODE_ENABLE] = 1;
  } 
  else if (start == 0)
  {
    outValues[BREATH_MODE_ENABLE] = 0;
    white_1_ramp.pause();
    white_2_ramp.pause();
    white_3_ramp.pause();
  }
}
void finale_mode_enable(int start){
  if (start == 1 )
  {
    flicker_time = finale_flicker_time;      // off time in between the 3 individual white leds in a smd5050

    white_1_ramp.setGrain(1);                         // set grain to 1 ms
    white_1_ramp.go(finale_min_intensity);                               // make sure to start at 0
    white_1_ramp.go(finale_max_intensity, finale_duration, SINUSOIDAL_INOUT, BACKANDFORTH);    // go to value 30 in 1000ms in a linear line and looping up and down

    white_2_ramp.setGrain(1);                         // set grain to 1 ms
    white_2_ramp.go(finale_min_intensity);                               // make sure to start at 0
    white_2_ramp.go(finale_max_intensity, finale_duration, SINUSOIDAL_INOUT, BACKANDFORTH);    // go to value 30 in 1000ms in a linear line and looping up and down

    white_3_ramp.setGrain(1);                         // set grain to 1 ms
    white_3_ramp.go(finale_min_intensity);                               // make sure to start at 0
    white_3_ramp.go(finale_max_intensity, finale_duration, SINUSOIDAL_INOUT, BACKANDFORTH);    // go to value 30 in 1000ms in a linear line and looping up and down       

    outValues[FINALE_MODE_ENABLE] = 1;
  } 
  else if (start == 0)
  {
    outValues[FINALE_MODE_ENABLE] = 0;
    white_1_ramp.pause();
    white_2_ramp.pause();
    white_3_ramp.pause();
  }
}
// INPUT FUNCTIONS
// function to read, set and send the inputs on the controller
// void digitalReadSetValue(uint8_t name_id)
// {
//     _input[name_id].current_debounce = millis();
//     if (_input[name_id].current_debounce - _input[name_id].last > _input[name_id].debounce)
//     {
//         if (!digitalRead(_input[name_id].pin) && _input[name_id].value == 0)
//         {
//             _input[name_id].value = 1;
//             Sherlocked.sendInput(name_id, _input[name_id].value, T_INPUT);
//             Serial.printf("input %i: 1\n", _input[name_id].pin);
//         }
//         else if (digitalRead(_input[name_id].pin) && _input[name_id].value == 1)
//         {
//             _input[name_id].value = 0;
//             Sherlocked.sendInput(name_id, _input[name_id].value, T_INPUT);
//             Serial.printf("input %i: 0\n", _input[name_id].pin);
//         }
//         _input[name_id].last = _input[name_id].current_debounce;  
//     }
// }



// set the state depending on the output
void outputStateMachine(int id, int value)
{
  id = id -1; 
  dbf("outputStateMachine received id: %i with value: %i\n", id, value);
  if (id == BREATH_MODE_ENABLE ){
    Serial.println("BREATH_MODE_ENABLE");
    breathing_mode_enable(value);    
  }
  if (id == BREATH_FLICKER_TIME ){
    Serial.println("BREATH_FLICKER_TIME");
    outValues[BREATH_FLICKER_TIME] = value;    
  }
  if (id == BREATH_MAX_INTENSITY ){
    Serial.println("BREATH_MAX_INTENSITY");
    outValues[BREATH_MAX_INTENSITY] = value;    
  }
  if (id == BREATH_MIN_INTENSITY ){
    Serial.println("BREATH_MIN_INTENSITY");
    outValues[BREATH_MIN_INTENSITY] = value;    
  }
  if (id == BREATH_DURATION ){
    Serial.println("BREATH_DURATION");
    outValues[BREATH_DURATION] = value;    
  }
  if (id == FINALE_MODE_ENABLE ){
    Serial.println("FINALE_MODE_ENABLE");
    finale_mode_enable(value);    
  }
  if (id == FINALE_FLICKER_TIME ){
    Serial.println("FINALE_FLICKER_TIME");
    outValues[FINALE_FLICKER_TIME] = value;    
  }
  if (id == FINALE_MAX_INTENSITY ){
    Serial.println("FINALE_MAX_INTENSITY");
    outValues[FINALE_MAX_INTENSITY] = value;    
  }
  if (id == FINALE_MIN_INTENSITY ){
    Serial.println("FINALE_MIN_INTENSITY");
    outValues[FINALE_MIN_INTENSITY] = value;    
  }
  if (id == FINALE_DURATION ){
    Serial.println("FINALE_DURATION");
    outValues[FINALE_DURATION] = value;    
  }
  if (id == 98){
    Serial.print("Wifi strengt: ");
    Serial.println(getWifiStrength(10));
    DynamicJsonBuffer  wifi_jsonBuffer(200);
    JsonObject& root = wifi_jsonBuffer.createObject();
    root["sender"] = hostname;
    root["method"] = "info";
    root["signal strength"] = getWifiStrength(10);
    root["trigger"] = "request";
    char wifi_char[250];
    root.prettyPrintTo(wifi_char, sizeof(wifi_char));
    pubMsg(wifi_char);     
    }
}
// void inputStateMachine()
// {
//   digitalReadSetValue(TOP_CONTROLLER_ARM_A);   
// }

// functions to set the in and outputs at the right starting position


// functions to send all out- and inputs
void sendAllOutputs()
{
  char * msg = Sherlocked.sendOutputs(NUM_OUTPUTS, outIDs, outValues, T_REQUEST);
  pubMsg(msg);
}
char * getAllOutputs()
{
  char * msg = Sherlocked.sendOutputs(NUM_OUTPUTS, outIDs, outValues, T_REQUEST);
  return(msg);
}
// void sendAllInputs()
// {
//   int allids[NUM_INPUTS]; // actual ids
//   int allvals[NUM_INPUTS]; // actual values
//   for (int i = 0; i < NUM_INPUTS; i++)
//   {
//       allids[i]  = _input[i].id;
//       allvals[i] = _input[i].value;
//   }
//   char * msg = Sherlocked.sendInputs(NUM_INPUTS, allids, allvals, T_REQUEST);
//   pubMsg(msg);
// }
// char * getAllInputs()
// {
//   int allids[NUM_INPUTS]; // actual ids
//   int allvals[NUM_INPUTS]; // actual values
//   for (int i = 0; i < NUM_INPUTS; i++)
//   {
//       allids[i]  = _input[i].id;
//       allvals[i] = _input[i].value;
//   }
//   char * msg = Sherlocked.sendInputs(NUM_INPUTS, allids, allvals, T_REQUEST);
//   return(msg);
// }
// helper functions for getting the right value for the right id
// int getOutputValueByID(int id)
// {
//   for(int i = 0; i < NUM_OUTPUTS; i++)
//   {
//     if(outIDs[i] == id)
//     {
//       return outValues[i];
//     }
//   }
// }
// int getInputValueByID(int id)
// {
//   for(int i = 0; i < NUM_INPUTS; i++)
//   {
//     if(inIDs[i] == id)
//     {
//       return inValues[i];
//     }
//   }
// }
int getOutputArrayIndexByID(int id)
{
  for (int i = 0; i < NUM_OUTPUTS; i++)
  {
    if (outIDs[i] == id)
    {
      return i;
    }
  }
  return UNDEFINED;
}
// int getInputArrayIndexByID(int id)
// {
//   for (int i = 0; i < NUM_INPUTS; i++)
//   {
//     if (_input[i].id == id)
//     {
//       return i;
//     }
//   }
//   return UNDEFINED;
// }

// puzzle controller specific functions
void resetPuzzle(){
    outValues[BREATH_MODE_ENABLE] = 0;
    outValues[FINALE_MODE_ENABLE] = 0;
    // pubMsg_kb("info", "state", "reset");
};






// Here starts the ACE/MQTT implementation --- this is rather long
#define dbf Serial.printf
char lastWillMsg[110];
char _incomingMessage[MESSAGE_LENGTH];
uint32_t _lastMqttSend = 0;

void reconnect() {
  sprintf(lastWillMsg, "{\"sender\":\"%s\",\"method\":\"info\",\"state\":\"offline\",\"trigger\":\"disconnect\"}", hostname);
  // Loop until we're reconnected
  while (!client.connected()) {
    if ((WiFi.status() != WL_CONNECTED))
    {
        initWiFi();
    }
    else if ((WiFi.status() == WL_CONNECTED))
    {
        Serial.print("Attempting MQTT connection to ");
        Serial.print(server);
        Serial.print("...");
        // Attempt to connect
        if (client.connect(hostname, "", "", puzzle_topic, 0, true, lastWillMsg)) {
        Serial.println("connected");
        // Once connected, publish an announcement...
        pubMsg_kb("info", "connected", "true", "trigger", "startup");
        // ... and resubscribe
        client.subscribe(gen_topic);
        client.subscribe(puzzle_topic);
        client.subscribe(module_topic);
        } else {
        Serial.print("failed, rc=");
        Serial.print(client.state());
        Serial.println(" try again in 5 seconds");
        // Wait 5 seconds before retrying
        delay(5000);
        }
    }    
  }
}

void mqttDisconnect(){
  client.disconnect();
}

void callback(char* topic, byte* payload, unsigned int length){
    uint16_t incMsgLen = 0;
    for (int i = 0; i < length; i++)
    {
        char c = (char)payload[i];
        _incomingMessage[incMsgLen] = c;
        incMsgLen ++;
    }
    _incomingMessage[incMsgLen] = '\0'; // add 0 at the end for str functions
    incMsgLen ++;
    Sherlocked.parse(_incomingMessage);
}



/* Always keep track of the puzzle state locally */
uint8_t _state = S_IDLE;

void setState(uint8_t s, int trigger)
{
  _state = s;
  dbf("State now %s (%u), triggered by %s\n", Sherlocked.getStateStr(s), s, Sherlocked.getTriggerStr(trigger));
}

uint8_t getState()
{
  return _state;
}

/* State callback is triggered whenever a 'state' is received */
void stateCallback(int meth, int state, int trigger)
{
  if (meth == M_PUT)
  {
    setState(state, trigger);
  }
  else if (meth == M_GET)
  {
    pubMsg(Sherlocked.sendState(getState(), T_REQUEST));
  }
}

/* input callback is triggered whenever 'inputs' is received; this is always an array */
void inputCallback(int meth, int numInputs, int ids[], int vals[], int trigger)
{
  dbf("inputCallback() method : %s, numInputs : %i, trigger : %s\n", Sherlocked.getMethodStr(meth), numInputs, Sherlocked.getTriggerStr(trigger) );
  if (meth == M_GET)
  {
    DynamicJsonBuffer  inputcalbackBuffer(200);
    inputcalbackBuffer.clear();
    JsonObject& root = inputcalbackBuffer.createObject();
    root["sender"] = hostname;
    JsonArray& outputs = root.createNestedArray("inputs");
    root["method"] 	= Sherlocked.getMethodStr(M_INFO);
    root["trigger"] = Sherlocked.getTriggerStr(T_REQUEST);
    char inputChar[400];
    root.prettyPrintTo(inputChar, sizeof(inputChar));

  }
  else if (meth == M_INFO) // listen to other input info
  {
    for (int i = 0; i < numInputs; i++)
    {
      // possibly do something with info received from other controllers within your own puzzle
    }
  }
}

/* output callback is triggered whenever 'outputs' is received; this is always an array */
void outputCallback(int meth, int numOutputs, int ids[], int vals[], int trigger)
{
  dbf("outputCallback() method : %s, numOutputs : %i, trigger : %s\n", Sherlocked.getMethodStr(meth), numOutputs, Sherlocked.getTriggerStr(trigger) );
  if (meth == M_PUT)
  {
    // set all desired outputs
    for (int i = 0; i < numOutputs; i++)
    {
      // set value SET HERE YOUR STATE OR START YOUR ACTION NEEDED.
      // dbf("\timplement: setOutput(int id: %i, int val: %i)\n", ids[i], vals[i]);
      outputStateMachine(ids[i], vals[i]);
    }

    // And compile a reply to let the server know the new outputs state
    int numOnBoard = 0;   // count the actual number on this board
    int aids[numOutputs]; // actual ids
    int avals[numOutputs]; // actual values
    for (int i = 0; i < numOutputs; i++)
    {
      int idx = getOutputArrayIndexByID(ids[i]); // find the array index of the output id on this board
      if (idx != UNDEFINED) // if found store it
      {
        aids[numOnBoard]  = outIDs[idx];
        avals[numOnBoard] = outValues[idx];
        numOnBoard++;
      }
    }
    if (numOnBoard > 0)
    {
      char * msg = Sherlocked.sendOutputs(numOnBoard, aids, avals, T_REQUEST);
      pubMsg(msg);
    }
  }
  else if (meth == M_GET)
  {
    // The get method only has ids; fill in the values with the values that correspond to the output ID
    if (numOutputs > 0)
    {
      int numOnBoard = 0;   // count the actual number on this board
      int aids[numOutputs]; // actual ids
      int avals[numOutputs]; // actual values
      for (int i = 0; i < numOutputs; i++)
      {
        int idx = getOutputArrayIndexByID(ids[i]); // find the array index of the output id on this board
        if (idx != UNDEFINED) // if found store it
        {
          aids[numOnBoard]  = outIDs[idx];
          avals[numOnBoard] = outValues[idx];
          numOnBoard++;
        }
      }
      if (numOnBoard > 0)
      {
        char * msg = Sherlocked.sendOutputs(numOnBoard, aids, avals, T_REQUEST);
        pubMsg(msg);
      }
    }
    else  // If no output id's are provided, feed them all back
    {
      pubMsg(Sherlocked.sendOutputs(NUM_OUTPUTS, outIDs, outValues, T_REQUEST));
    }
  }
  else if (meth == M_INFO) // listen to other output info
  {
    for (int i = 0; i < numOutputs; i++)
    {
      // possibly do something with info received from other controllers within your own puzzle
    }
  }
}

/*  The JSON Handler is optional. This handler can be used for custom JSON messages that are not covered by the MessageHandler
    This function provides a JSON object from the ArduinoJson Library
 */
void jsonCallback(JsonObject & json)
{
  dbf("jsonCallback() : ");
  json.printTo(Serial);

  if (json.containsKey("r") && json.containsKey("g") && json.containsKey("b"))
  {
    // Extract the values from the JSON object
    //  uint8_t r = json["r"];
    //  uint8_t g = json["g"];
    //  uint8_t b = json["b"];
     // And use it in a suitable function
     // setLEDColor(r, g, b); 
  }
  // Or use it to turn a motor in a certain direction
  // {"direction":"left"}
  else if(json.containsKey("direction")) 
  {
    // Extract the value from the JSON object
    const char * pos = json["direction"];
    // Compare the value with expected values
    if (strcmp(pos, "left") == 0)
    {
      // turnMotorLeft();
    } 
    else if (strcmp(pos, "right") == 0)
    {
      // turnMotorRight();
    } 
  }
  
}

// end of the ACE/MQTT implementation



// START OF THE OTA CODE
#include <Update.h>
uint16_t _otaTimeout = 15000;
int contentLength = 0;
bool isValidContentType = false;
String host = server.toString();
int port = 8888; // Non https. For HTTPS 443.  HTTPS doesn't work yet <= REPLACE for 80
String bin; // bin file name with a slash in front.

void setBinVers(const char binfile[])
{
  // bin = "/ota/";
  String bf = String(binfile);
  bin = bf;
  Serial.print("Setting bin file to: ");
  Serial.println(bin);
}

// Utility to extract header value from headers
String getHeaderValue(String header, String headerName)
{
  return header.substring(strlen(headerName.c_str()));
}

// OTA Logic
void execOTA()
{
  // otaClient.setTimeout(_otaTimeout);
  Serial.println("Connecting to: " + String(host));
  if (espClient.connect(host.c_str(), port)) {
    // Connection Succeed.
    Serial.println("Fetching Bin: " + String(bin));
    // Get the contents of the bin file
    espClient.print(String("GET ") + bin + " HTTP/1.1\r\n" +
                    "Host: " + host + "\r\n" +
                    "Cache-Control: no-cache\r\n" +
                    "Connection: close\r\n\r\n");

    // Check what is being sent
    //    Serial.print(String("GET ") + bin + " HTTP/1.1\r\n" +
    //                 "Host: " + host + "\r\n" +
    //                 "Cache-Control: no-cache\r\n" +
    //                 "Connection: close\r\n\r\n");

    unsigned long timeout = millis();
    while (espClient.available() == 0) {
      if (millis() - timeout > _otaTimeout) {
        Serial.println("Client Timeout !");
        espClient.stop();
        return;
      }
    }
    // Once the response is available,
    // check stuff

    /*
       Response Structure
        HTTP/1.1 200 OK
        x-amz-id-2: NVKxnU1aIQMmpGKhSwpCBh8y2JPbak18QLIfE+OiUDOos+7UftZKjtCFqrwsGOZRN5Zee0jpTd0=
        x-amz-request-id: 2D56B47560B764EC
        Date: Wed, 14 Jun 2017 03:33:59 GMT
        Last-Modified: Fri, 02 Jun 2017 14:50:11 GMT
        ETag: "d2afebbaaebc38cd669ce36727152af9"
        Accept-Ranges: bytes
        Content-Type: application/octet-stream
        Content-Length: 357280
        Server: AmazonS3

        {{BIN FILE CONTENTS}}

    */
    while (espClient.available()) {
      // read line till /n
      String line = espClient.readStringUntil('\n');
      // remove space, to check if the line is end of headers
      line.trim();

      // if the the line is empty,
      // this is end of headers
      // break the while and feed the
      // remaining `espClient` to the
      // Update.writeStream();
      if (!line.length()) {
        //headers ended
        break; // and get the OTA started
      }

      // Check if the HTTP Response is 200
      // else break and Exit Update
      if (line.startsWith("HTTP/1.1")) {
        if (line.indexOf("200") < 0) {
          Serial.println("Got a non 200 status code from server. Exiting OTA Update.");
          break;
        }
      }

      // extract headers here
      // Start with content length
      if (line.startsWith("Content-Length: ")) {
        contentLength = atoi((getHeaderValue(line, "Content-Length: ")).c_str());
        Serial.println("Got " + String(contentLength) + " bytes from server");
      }

      // Next, the content type
      if (line.startsWith("Content-Type: ")) {
        String contentType = getHeaderValue(line, "Content-Type: ");
        Serial.println("Got " + contentType + " payload.");
        if (contentType == "application/octet-stream") {
          isValidContentType = true;
        }
      }
    }
  } else {
    // Connect to failed
    // May be try?
    // Probably a choppy network?
    Serial.println("Connection to " + String(host) + " failed. Please check your setup");
    // retry??
    // execOTA();
  }

  // Check what is the contentLength and if content type is `application/octet-stream`
  Serial.println("contentLength : " + String(contentLength) + ", isValidContentType : " + String(isValidContentType));

  // check contentLength and content type
  if (contentLength && isValidContentType) {
    // Check if there is enough to OTA Update
    bool canBegin = Update.begin(contentLength);

    // If yes, begin
    if (canBegin) {
      Serial.println("Begin OTA. This may take 2 - 5 mins to complete. Things might be quite for a while.. Patience!");
      // No activity would appear on the Serial monitor
      // So be patient. This may take 2 - 5mins to complete
      size_t written = Update.writeStream(espClient);

      if (written == contentLength) {
        Serial.println("Written : " + String(written) + " successfully");
      } else {
        Serial.println("Written only : " + String(written) + "/" + String(contentLength) + ". Retry?" );
        // retry??
        // execOTA();
      }

      if (Update.end()) {
        Serial.println("OTA done!");
        if (Update.isFinished()) {
          Serial.println("Update successfully completed. Rebooting.");
          ESP.restart();
        } else {
          Serial.println("Update not finished? Something went wrong!");
        }
      } else {
        Serial.println("Error Occurred. Error #: " + String(Update.getError()));
      }
    } else {
      // not enough space to begin OTA
      // Understand the partitions and
      // space availability
      Serial.println("Not enough space to begin OTA");
      espClient.flush();
    }
  } else {
    Serial.println("There was no content in the response");
    espClient.flush();
  }
}
void startOTA()
{
  dbf("StartOTA\n");
  if (bin != NULL && !bin.equals(""))
  {
    char temp [MESSAGE_LENGTH];
    bin.toCharArray(temp, bin.length() + 1);
    dbf("Bin is set to %s\n", temp);
    if (WiFi.status() != WL_CONNECTED)
    {
      Serial.println("WiFi is down...");
      initWiFi();
    }
    // while (WiFi.status() == WL_CONNECTED) {
    //   delay(1);  // wait for a bit
    // }
    char ota_info[200];
    sprintf(ota_info, "{ \"event\": \"OTA\", \"URI\": \"%s\", \"current_firmware\": \"%s\" }", temp, firmware_version);
    pubMsg_kb("info", "info", ota_info, "trigger", "disconnect");
    delay(100);
    mqttDisconnect();
    execOTA();
  }
  else
  {
    dbf("No file for OTA, check for updates?\n");
//    checkForUpdates();
  }
}
void doOTA(const char binfile[])
{
  setBinVers(binfile);
  startOTA();
}
// END OF THE OTA CODE

// The function that actually does the incoming commands
/* The commandCallback function is called (activated) when a new 'cmd' or 'info' command is received */
void commandCallback(int meth, int cmd, const char value[], int triggerID)
{
  switch (cmd)
  {
    case CMD_RESET:
      dbf("Received Puzzle RESET from Sherlocked\n");
      resetPuzzle();
      break;

    case CMD_REBOOT:
      dbf("Received Reboot from Sherlocked\n");
      ESP.restart();
      break;

    case CMD_SYNC:
      dbf("Sync Command not implemented for this board\n");  // en is ook niet nodig, alleen voor files belangrijk
      break;

    case CMD_OTA:
      dbf("OTA Firmware update command\n"); // deze gaan we wel doen, Serge stuurt me voorbeeld code
      // value contains the file URL for the OTA command
      doOTA(value);
      break;

    case INFO_SYSTEM:
      dbf("system info requested\n");
      DynamicJsonBuffer  jsonBuffer(400);
      JsonObject& root = jsonBuffer.createObject();
      root["sender"] = hostname;
      root["method"] = M_INFO;
      root["ip"] = localIP;
      root["mac"] = macAddress;
      root["rssi"] = getWifiStrength(10);
      root["version"] = firmware_version;
      root["trigger"] = T_REQUEST;
      char full_char[1200];
      root.prettyPrintTo(full_char, sizeof(full_char));
      pubMsg(full_char);
      break;

    case INFO_STATE:
      dbf("state requested\n");
      pubMsg(Sherlocked.sendState(getState(), T_REQUEST));
      break;

    case INFO_FULLSTATE:
      dbf("full state requested\n");
      // {"sender":"controller","state":"idle","connected":true,"inputs":[{"id":1,"value":1}],"outputs":[{"id":1,"value":0},{"id":3,"value":1}],"method":"info","trigger":"request"}
      DynamicJsonBuffer  jsonBuffer(400);
      JsonObject& root = jsonBuffer.createObject();
      root["sender"] = hostname;
      root["state"] = getState();
      root["connected"] = client.connected();
      // geen inputs dus geen array terug
    //   JsonArray& inputs = root.createNestedArray("inputs");
    //   for (int i = 0; i < NUM_INPUTS; i++)
    //   {
    //     JsonObject& ind_val = inputs.createNestedObject();
    //     ind_val["id"] = _input[i].id;
    //     ind_val["value"] = _input[i].value;
    //   }
      JsonArray& outputs = root.createNestedArray("outputs");
      for (int i = 0; i < NUM_OUTPUTS; i++)
      {
        JsonObject& ond_val = outputs.createNestedObject();
        ond_val["id"] = outIDs[i];
        ond_val["value"] = outValues[i];
      }
      root["trigger"] = "request";
      char full_char[1200];
      root.prettyPrintTo(full_char, sizeof(full_char));
      pubMsg(full_char);
      break;
  }
}


void setup() {
    Serial.begin(115200);
    Serial.println("Conductive MQTT Sensor");

    //   initInputs();
    setOutputsNum();

    // begin the strip communication with the desired pins, since it's an ESP32 we can use all we want. 
    strip.Begin(DotClockPin, DotDataPin, DotDataPin, DotChipSelectPin);

    // this resets all the neopixels to an off state
    strip.ClearTo(0);
    strip.Show();

    // Allow the hardware to sort itself out
    delay(3500);

    // start the ethernet client
    initWiFi();

    // start the mqtt client
    client.setServer(server, 1883);
    client.setCallback(callback);
    client.setBufferSize(1200);


    /* Set the name for this controller, this should be unqiue within */
    Sherlocked.setName(hostname);
    /* Set callback functions for the various messages that can be received by the Sherlocked ACE system */    
    /* Puzzle State Changes are handled here */
    Sherlocked.setStateCallback(stateCallback);
    /* General Command and Info Messages */
    Sherlocked.setCommandCallback(commandCallback);
    /* Inputs and Outputs */
    Sherlocked.setInputCallback(inputCallback);
    Sherlocked.setOutputCallback(outputCallback);
    /* Catch-all callback for json messages that were not handled by other callbacks */
    Sherlocked.setJSONCallback(jsonCallback);

}





void loop() {

    // if the mqtt client does not connect, try it again later
    if (!client.connected()) {
        reconnect();
    }
    client.loop();  

    // setup for the non blocking functions
    currentMicros = micros();

    //   inputStateMachine();

    if (outValues[BREATH_MODE_ENABLE] || outValues[FINALE_MODE_ENABLE])
    {
        // if (currentMicros - previousMicros >= flicker_time) 
        // {
            previousMicros = currentMicros;   
            int white_1_i = white_1_ramp.update();
            // Serial.print("white_1_i: ");
            // Serial.println(white_1_i);
            strip.ClearTo(RgbColor(0, white_1_i, 0));
            strip.Show();
        // }
            delay(flicker_time);
        // if (currentMicros - previousMicros >= flicker_time_2) 
        // {
            previousMicros = currentMicros;   
            int white_2_i = white_2_ramp.update();
            // Serial.print("white_2_i: ");
            // Serial.println(white_2_i);
            strip.ClearTo(RgbColor(white_2_i, 0, 0));
            strip.Show();
        // }
            delay(flicker_time);
        // if (currentMicros - previousMicros >= flicker_time_3) 
        // {
            previousMicros = currentMicros;   
            int white_3_i = white_3_ramp.update();
            // Serial.print("white_3_i: ");
            // Serial.println(white_3_i);
            strip.ClearTo(RgbColor(0, 0, white_3_i));
            strip.Show();
        // }
            delay(flicker_time);
    }
    else if (!outValues[BREATH_MODE_ENABLE] && !outValues[FINALE_MODE_ENABLE])
    {
        strip.ClearTo(0);
        strip.Show();
    }
    
    

}