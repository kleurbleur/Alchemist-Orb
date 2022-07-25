
char _incomingMessage[MESSAGE_LENGTH];
uint32_t _lastMqttSend = 0;
char _pubBuf[MESSAGE_LENGTH];
/*
   MQTT Callback
*/

void mqttCallback(char* topic, byte* payload, unsigned int length)
{
  uint16_t incMsgLen = 0;
  for (int i = 0; i < length; i++)
  {
    char c = (char)payload[i];
    _incomingMessage[incMsgLen] = (char)payload[i];
    incMsgLen ++;
  }
  _incomingMessage[incMsgLen] = '\0'; // add \0 at the end for str fucntions
  incMsgLen ++;
  //  Serial.print("Message arrived in: ");
  //  Serial.print(topic);
  //  Serial.print(" \t");
  parseCommand(_incomingMessage);
}


/*
   SETUP
*/

bool mqttConnect()
{
  if (!client.connected())
  {
    Serial.print("[MQTT] Attempting MQTT Connection as ");
    Serial.println(_mac);
    StaticJsonBuffer<MESSAGE_LENGTH> jsonBuffer;
    JsonObject& root = jsonBuffer.createObject();
    root["sender"] = _cfg.name;
    root["state"] = "offline";
    root["method"] = getMethodStr(M_INFO);
    root["trigger"] = getEvtStr(E_DISCONNECT);
    char buffer[MESSAGE_LENGTH];
    root.printTo(buffer, sizeof(buffer));
    espClient.setTimeout(3000);

    if (client.connect(_mac, _ownTopic, 1, false, buffer)) // will topic, QoS, retain, message
    {
      Serial.println("[MQTT] connected");
      _mqttConnected = true;

      // ... and resubscribe
      client.subscribe(_ownTopic);
      client.subscribe(_roomTopic);
      client.subscribe(_moduleTopic);
      dbf("\t Subscribed to %s\n", _roomTopic);
      dbf("\t Subscribed to %s\n", _ownTopic);
      dbf("\t Subscribed to %s\n", _moduleTopic);

      dbf("\t Client state now ");
      Serial.println(client.state());
      dbf("\t Client Connected is ");
      Serial.println(client.connected());
      return true;
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.println(client.state());
      return false;
    }
  }
}


void mqttDisConnect()
{
  if (client.connected())
  {
    StaticJsonBuffer<MESSAGE_LENGTH> jsonBuffer;
    JsonObject& root = jsonBuffer.createObject();
    root["state"] = "offline";
    char buffer[MESSAGE_LENGTH];
    root.printTo(buffer, sizeof(buffer));
    pubMsg(buffer);

    dbf("MQTT Disconnect\n");
    _mqttConnected = false;
    client.loop();
    client.disconnect();
  }
}


bool setupMQTT()
{
  dbf("Setup MQTT\n");

  strcpy(_roomTopic, _cfg.room);

  strcpy(_ownTopic, _roomTopic);
  strcat(_ownTopic, "/");
  strcat(_ownTopic, _cfg.puzzle);

  // most specific, just fir us
  strcpy(_moduleTopic, _ownTopic);
  strcat(_moduleTopic, "/");
  strcat(_moduleTopic, _cfg.name);

  client.setBufferSize(MESSAGE_LENGTH);
  client.setServer(_mqtt_server, 1883);
  client.setCallback(mqttCallback);

  if (!mqttConnect())
  {
    dbf("MQTT Connection Failed...\n");
    return false;
  }
  return true;
}

bool pubFullState(int evt)
{
  StaticJsonBuffer<MESSAGE_LENGTH> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root["sender"] = _cfg.name;
  root["state"] = getStateStr(getState());
  
  JsonArray& inputs = root.createNestedArray("inputs");
  for (int i = 0; i < _cfg.numInputs; i++)
  {
    JsonObject& nested = inputs.createNestedObject();
    nested["id"]    = _inputs[i].id;
    nested["value"] = _inputs[i].value;
  }

  JsonArray& outputs = root.createNestedArray("outputs");
  for (int i = 0; i < _cfg.numOutputs; i++)
  {
    JsonObject& nestedOut = outputs.createNestedObject();
    nestedOut["id"]    = _outputs[i].id;
    nestedOut["value"] = _outputs[i].value;
  }
  root["method"] = getMethodStr(M_INFO);
  root["trigger"] = getEvtStr(evt);
  root.printTo(_pubBuf, sizeof(_pubBuf));
  return pubMsg(_pubBuf);
}

bool pubInfo(int evt)
{
  StaticJsonBuffer<MESSAGE_LENGTH> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root["sender"] = _cfg.name;
  root["mac"] = _mac;
  root["ip"] = _myIP;
  root["version"] = _version;
  root["method"] = getMethodStr(M_INFO);
  root["trigger"] = getEvtStr(evt);
  root.printTo(_pubBuf, sizeof(_pubBuf));
  return pubMsg(_pubBuf);
}

bool pubInput(int id, int val)
{
  StaticJsonBuffer<MESSAGE_LENGTH> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root["sender"] = _cfg.name;

  JsonArray& inputs = root.createNestedArray("inputs");
  for (int i = 0; i < _cfg.numInputs; i++)
  {
    if (_inputs[i].id == id)
    {
      JsonObject& nested = inputs.createNestedObject();
      nested["id"]    = _inputs[i].id;
      nested["value"] = _inputs[i].value;
      break;
    }
  }
  root["method"] = getMethodStr(M_INFO);
  root["trigger"] = getEvtStr(E_INPUT);
  //  char buffer[MESSAGE_LENGTH];
  root.printTo(_pubBuf, sizeof(_pubBuf));
  return pubMsg(_pubBuf);
}

bool pubInputs(int evt)
{
  StaticJsonBuffer<MESSAGE_LENGTH> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root["sender"] = _cfg.name;

  JsonArray& inputs = root.createNestedArray("inputs");
  for (int i = 0; i < _cfg.numInputs; i++)
  {
    JsonObject& nested = inputs.createNestedObject();
    nested["id"]    = _inputs[i].id;
    nested["value"] = _inputs[i].value;
  }
  
  root["method"] = getMethodStr(M_INFO);
  root["trigger"] = getEvtStr(evt);
  //  char buffer[MESSAGE_LENGTH];
  root.printTo(_pubBuf, sizeof(_pubBuf));
  return pubMsg(_pubBuf);
}

bool pubInputs(bool indexArr[], int len, int evt)
{
  StaticJsonBuffer<MESSAGE_LENGTH> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root["sender"] = _cfg.name;

  JsonArray& inputs = root.createNestedArray("inputs");
  for (int i = 0; i < len; i++)
  {
    if(indexArr[i] && i >= 0 && i < _cfg.numInputs)
    {
      JsonObject& nested = inputs.createNestedObject();
      nested["id"]    = _inputs[i].id;
      nested["value"] = _inputs[i].value;
    }
  }
  
  root["method"] = getMethodStr(M_INFO);
  root["trigger"] = getEvtStr(evt);
  //  char buffer[MESSAGE_LENGTH];
  root.printTo(_pubBuf, sizeof(_pubBuf));
  return pubMsg(_pubBuf);
}

bool pubOutput(int id, int val, int evt)
{
  dbf("pubOutput %i %i (%s, %i)\n", id, val, getEvtStr(evt), evt);
  StaticJsonBuffer<MESSAGE_LENGTH> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root["sender"] = _cfg.name;

  JsonArray& outputs = root.createNestedArray("outputs");
  for (int i = 0; i < _cfg.numOutputs; i++)
  {
    if (_outputs[i].id == id)
    {
      JsonObject& nested = outputs.createNestedObject();
      nested["id"]    = _outputs[i].id;
      nested["value"] = _outputs[i].value;
      break;
    }
  }
  root["method"] = getMethodStr(M_INFO);
  root["trigger"] = getEvtStr(evt);
  //  char buffer[MESSAGE_LENGTH];
  root.printTo(_pubBuf, sizeof(_pubBuf));
  return pubMsg(_pubBuf);
}

bool pubOutputs(int evt)
{
  dbf("pubOutputs %s \n",getEvtStr(evt));
  StaticJsonBuffer<MESSAGE_LENGTH> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root["sender"] = _cfg.name;

  JsonArray& outputs = root.createNestedArray("outputs");
  for (int i = 0; i < _cfg.numOutputs; i++)
  {
    JsonObject& nested = outputs.createNestedObject();
    nested["id"]    = _outputs[i].id;
    nested["value"] = _outputs[i].value;
   }
  root["method"] = getMethodStr(M_INFO);
  root["trigger"] = getEvtStr(evt);
  //  char buffer[MESSAGE_LENGTH];
  root.printTo(_pubBuf, sizeof(_pubBuf));
  return pubMsg(_pubBuf);
}

bool pubOutputs(bool indexArr[], int len, int evt)
{
  dbf("pubOutputs with indexes %s \n",getEvtStr(evt));
  StaticJsonBuffer<MESSAGE_LENGTH> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root["sender"] = _cfg.name;

  JsonArray& outputs = root.createNestedArray("outputs");
  for (int i = 0; i < len; i++)
  {
    if(indexArr[i] && i >= 0 && i < _cfg.numOutputs)
    {
      JsonObject& nested = outputs.createNestedObject();
      nested["id"]    = _outputs[i].id;
      nested["value"] = _outputs[i].value;
    }
  }
  root["method"] = getMethodStr(M_INFO);
  root["trigger"] = getEvtStr(evt);
  //  char buffer[MESSAGE_LENGTH];
  root.printTo(_pubBuf, sizeof(_pubBuf));
  return pubMsg(_pubBuf);
}

bool pubState(int evt)
{
  StaticJsonBuffer<MESSAGE_LENGTH> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root["sender"] = _cfg.name;
  root["state"] = getStateStr(getState());
  root["method"] = getMethodStr(M_INFO);
  root["trigger"] = getEvtStr(evt);
  //  char buffer[MESSAGE_LENGTH];
  root.printTo(_pubBuf, sizeof(_pubBuf));
  pubMsg(_pubBuf);
}


bool pubMsg(JsonObject &root)
{
  root["sender"] = _cfg.name;
  root.printTo(_pubBuf, sizeof(_pubBuf));
  return pubMsg(_pubBuf);
}

bool pubMsg(char * buffer)
{
  return pubMsg(buffer, true); // by default, force the emssage
}

bool pubMsg(char * buffer, bool force)
{
  if (_wifiConnected && _mqttConnected)
  {
    if (millis() - _lastMqttSend > 50 || force)
    {
      bool success = client.publish(_ownTopic, buffer);
      _lastMqttSend = millis();
      dbf("\t MQTT %s: %s\n", success ? "SUCCESS" : "FAIL" , buffer);
      return success;
    }
    else
    {
      dbf("pubMsg(%s) too soon after last message..\n", buffer);
    }
  }
  else
  {
    dbf("pubMsg(%s) without wifi or mqtt..\n", buffer);
  }
  return false;
}
