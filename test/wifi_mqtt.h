/*
 * wifi_mqtt.h - The unofficial Sherlocked library.
 *
 * Copyright (c) 2022
 * Interactive Matter, by Serge Offermans
 * serge@interactivematter.nl
 * 
 * Reworked by Kleurbleur, Mark Ridder
 *
 * All rights reserved. 
 * LAST UPDATE: 18-08-2022
*/
#ifndef Sherlocked_h
#define Sherlocked_h

#include <Arduino.h>
#include <PubSubClient.h>
#include <WiFi.h>
#include <Update.h>
#include <Sherlocked.h>
#include <ArduinoJson.h>


// wifi functions
void initWiFi();

// mqtt functions 
void reconnect();
void mqttDisconnect();
void callback(char* topic, byte* payload, unsigned int length);

// ace functions
void pubMsg_kb(const char * method, const char *param1=(char*)'\0', const char *val1=(char*)'\0', const char *param2=(char*)'\0', const char *val2=(char*)'\0');
void pubMsg(char msg[]);void setState(uint8_t s, int trigger);
uint8_t getState();
void stateCallback(int meth, int state, int trigger);
void inputCallback(int meth, int numInputs, int ids[], int vals[], int trigger);
void outputCallback(int meth, int numOutputs, int ids[], int vals[], int trigger);
void jsonCallback(JsonObject & json);
void commandCallback(int meth, int cmd, const char value[], int triggerID);


// ota functions
void setBinVers(const char binfile[]);
String getHeaderValue(String header, String headerName);
void execOTA();
void startOTA();
void doOTA(const char binfile[]);









#endif