#include <Arduino.h>
// #include <NeoPixelBus.h>
// #include <Ramp.h> 
// #include <StopWatch.h>
#include <Sherlocked.h>
#include <PubSubClient.h>
#include <WiFi.h>

// general settings
const char* firmware_version = "0.1";


// network settings
const char* ssid = "Wireless Funtime Palace";
const char* password = "radiorijnmond";
const char hostname[] ="controller";
const char gen_topic[] = "alch";  
const char puzzle_topic[] = "alch/centrepiece"; 
const char module_topic[] = "alch/centrepiece/controller";
IPAddress server(192, 168, 178, 214);

WiFiClient espClient;
PubSubClient client(espClient);

// declare function prototypes 
void pubMsg_kb(char * method, char * param1=NULL, char * val1=NULL, char * param2=NULL, char * val2=NULL );


void pubMsg(char* msg)
{
  Serial.println(msg);
  client.publish(puzzle_topic, msg);
}


// actual function
void pubMsg_kb(char * method, char * param1, char * val1, char * param2, char * val2 )
{
  char jsonMsg[200], arg1[50], arg2[50];

  if (param1 && val1)
  {
    sprintf(arg1, ", \"%s\":\"%s\"", param1, val1); 
  }
  else if (param1 && !val1){
    Serial.println("function pubMsg: Please supply a value with the parameter 1");
  }
  else if (param1==NULL && val1==NULL ){
    sprintf(arg1, ""); // this can be cleaner I guess but without it arg1 would be filled with \xa5\xa5\xa5 etcetera 
  }  

  if (param2 && val2)
  {
    sprintf(arg2, ", \"%s\":\"%s\"", param2, val2); 
  }
  else if (param2 && !val2)
  {
    Serial.println("function pubMsg: Please supply a value with the parameter 2");
  }
  else if (param2==NULL && val2==NULL){
    sprintf(arg2, ""); // this can be cleaner I guess but without it arg2 would be filled with \xa5\xa5\xa5 etcetera 
  }  

  sprintf(jsonMsg, "{\"sender\":\"%s\" , \"method\":\"%s\" %s %s}", hostname, method, arg1, arg2);
  Serial.print("arg2: ");
  Serial.println(arg2);
  Serial.println(jsonMsg);
  client.publish(puzzle_topic, jsonMsg);
}



void resetPuzzle(){
  pubMsg_kb("info", "state", "reset");
};




// set up the wifi



void initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println(WiFi.localIP());
}


// Here starts the ACE/MQTT implementation --- this is rather long
#define dbf Serial.printf
char _incomingMessage[MESSAGE_LENGTH];
uint32_t _lastMqttSend = 0;
char _pubBuf[MESSAGE_LENGTH];

/*  Phony publish function; replace this with an actual MQTT pub function
    Make sure to publish in the 
*/

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection to ");
    Serial.print(server);
    Serial.print("...");
    // Attempt to connect
    if (client.connect(hostname)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      pubMsg_kb("info", "connected", "true", "trigger", "startup");
      // pubMsg_kb("info", ", \"connected\":true,\", \"trigger\":\"startup\"");
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

void callback(char* topic, byte* payload, unsigned int length){
    uint16_t incMsgLen = 0;
    for (int i = 0; i < length; i++)
    {
        char c = (char)payload[i];
        _incomingMessage[incMsgLen] = (char)payload[i];
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

/* The commandCallback function is called (activated) when a new 'cmd' or 'info' command is received */
void commandCallback(int meth, int cmd, const char * value, int triggerID)
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
      dbf("Sync Command not implemented for this board\n");
      break;

    case CMD_OTA:
    dbf("OTA Firmware update command\n");
      // value contains the file URL for the OTA command
      // doOTA(value);
      break;

    case INFO_SYSTEM:
        // Expects to receive back system info such as local IP ('ip'), Mac address ('mac') and Firmware Version ('fw')
      break;

    case INFO_STATE:
        pubMsg(Sherlocked.sendState(getState(), T_REQUEST));
      break;

    case INFO_FULLSTATE:
      // Expects to receive back a full state with all relevant inputs and outputs
      break;
  }
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
    // The get method only has ids; fill in the values with the values that correspond to the input ID
    if (numInputs > 0)
    {
      for (int i = 0; i < numInputs; i++)
      {
        vals[i] = random(0, 8);
      }
      char * msg = Sherlocked.sendInputs(numInputs, ids, vals, T_REQUEST);
      pubMsg(msg);
    }
    else  // If no input id's are provided, feed them all back
    {
      numInputs = 10;
      // store ids and values in new arrays so we have sufficient space
      int aids[numInputs];
      int avals[numInputs];
      for (int i = 0; i < numInputs; i++)
      {
        aids[i]   =  i + 1;
        avals[i]  =  random(0,255);
      }
      char * msg = Sherlocked.sendInputs(numInputs, aids, avals, T_REQUEST);
      pubMsg(msg);
    }
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
      // set value
      dbf("\timplement: setOutput(int id: %i, int val: %i)\n", ids[i], vals[i]);
    }

    // And compile a reply to let the server know the new outputs state
    int numFBOutputs = 0;
    int fbids[numOutputs];
    int fbvals[numOutputs];
    for (int i = 0; i < numOutputs; i++)
    {
      // Make sure to filter out any ID's that are not on this controller
      // int val = getValueByOutputID(ids[i]);
      int val = vals[i];
      if (val != UNDEFINED)
      {
        fbids[numFBOutputs]   = ids[i];
        fbvals[numFBOutputs]  = val;
        numFBOutputs ++;
      }
    }
    if (numFBOutputs > 0)
    {
      pubMsg(Sherlocked.sendOutputs(numFBOutputs, fbids, fbvals, trigger));
    }
  }
  else if (meth == M_GET)
  {
    // The get method only has ids; fill in the values with the values that correspond to the output ID
    if (numOutputs > 0)
    {
      for (int i = 0; i < numOutputs; i++)
      {
        vals[i] = random(0, 8);
      }
      pubMsg(Sherlocked.sendOutputs(numOutputs, ids, vals, T_REQUEST));
    }
    else  // If no output id's are provided, feed them all back
    {
      numOutputs = 10;
      // store ids and values in new arrays so we have sufficient space
      int allids[numOutputs];
      int allvals[numOutputs];
      for (int i = 0; i < numOutputs; i++)
      {
        allids[i]   =  i + 1;
        allvals[i]  =  random(0,255);
      }
      pubMsg(Sherlocked.sendOutputs(numOutputs, allids, allvals, T_REQUEST));
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
     uint8_t r = json["r"];
     uint8_t g = json["g"];
     uint8_t b = json["b"];
     // And use it in a suitable function
     // setLEDColor(r, g, b); 
  }
  // Or use it to turn a motor in a certain direction
  // {"direction":"left"}
  else if(json.containsKey("direction")) 
  {
    // Extract the value from the JSON object
    const char* pos = json["direction"];
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










void setup()
{
    Serial.begin(115200);
    while (!Serial); // wait for serial attach

    initWiFi();

    delay(1000);

    client.setServer(server, 1883);
    client.setCallback(callback);

    delay(1000);

    /* Set the name for this controller, this should be unqiue within */
    Sherlocked.setName("orb");
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

    delay(1000);

    Serial.println();
    Serial.println("Initializing...");
    Serial.flush();
    while (Serial.available()){ 
        Serial.read();
    }


}


void loop()
{

    // ACE/MQTT Stuff
    if (!client.connected()) {
        reconnect();
    }
    client.loop();

    if (Serial.available()) {
        String incomingMqttMessage = Serial.readStringUntil('\n');
        // The parser accepts a char * (c string)
        char msgBuf[1024];
        strcpy(msgBuf, incomingMqttMessage.c_str());
        Sherlocked.parse(msgBuf);
    }
    /* Keep server up to date about input and output changes*/
    static uint32_t lastSend = 0;
    if(millis() - lastSend > 30000)
    {
        lastSend = millis();
        int inputId = 1;
        int inputValue = random(0, 255);
        pubMsg(Sherlocked.sendInput(inputId, inputValue, T_INPUT));
    }

}

