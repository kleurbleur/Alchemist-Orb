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
 * LAST UPDATE: 17-08-2022
*/
#ifndef Sherlocked_h
#define Sherlocked_h

#include "Arduino.h"
#include <PubSubClient.h>
#include <WiFi.h>

void pubMsg_kb(const char * method, const char *param1=(char*)'\0', const char *val1=(char*)'\0', const char *param2=(char*)'\0', const char *val2=(char*)'\0' );
