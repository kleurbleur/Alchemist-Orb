

void parseCommand(char * incomingMessage)
{
  Serial.print("PARSING: ");
  Serial.println(incomingMessage);
  //   Serial.println();

  // possibly make the size of inc mes len
  StaticJsonBuffer<MESSAGE_LENGTH> jsonBuffer;

  JsonObject &root = jsonBuffer.parseObject(incomingMessage);

  // Test if parsing succeeds.
  if (!root.success()) {
    Serial.println(("MQTT JSON parseObject() failed"));
    //    Serial.println("{\"err\":\"events\"}");
  }
  else
  {
    if (root.containsKey("sender"))
    {
      const char* s = root["sender"];
      if (strcmp(s, _cfg.name) == 0) // We sent this message, dont parse!
      {
        return;
      }
    }
    else
    { // discard messages without teh sender parameter defined
      dbf("No Sender Defined, ignore message\n");
      return;
    }

    if (root.containsKey("recipient"))
    {
      const char* r = root["recipient"];
      if (strcmp(r, _cfg.name) != 0) // This message is addressed to someone else!
      {
        dbf("Recipient defined but not us, ignore message\n");
        return;
      }
    }



    if (!root.containsKey("method"))
    {
      dbf("No method defined, ignore message\n");
      return;
    }

    const char* methodStr = root["method"];
    int meth = getMethodID(methodStr);

    const char* sender = root["sender"];

    if (meth == M_PUT)
    {
      if (root.containsKey("state"))
      {
        const char * s = root["state"];
        int stat = getStateID(s);
        if (stat != UNDEFINED)
        {
          setState(stat, getEvtID(sender));
        }
      }

      if (root.containsKey("outputs"))
      {
        JsonVariant outputs = root["outputs"];
        if (outputs.is<JsonArray>())
        {
          int numOutputs = outputs.size();
          dbf("Got outputs Array with %d outputs\n", numOutputs);
          for (int i = 0; i < numOutputs; i++)
          {
            if (outputs[i].is<JsonObject>())
            {
              if (outputs[i].as<JsonObject>().containsKey("id") && outputs[i].as<JsonObject>().containsKey("value"))
              {
                int id  = outputs[i].as<JsonObject>()["id"];
                int val = outputs[i].as<JsonObject>()["value"];

                setOutputByID(id, val, getEvtID(sender));
              }
            }
          }
        }
      }

      if (root.containsKey("cmd"))
      {
        const char * c = root["cmd"];
        if (strcmp(c, "reset") == 0)
        {
          resetPuzzle();
        }
        if (strcmp(c, "reboot") == 0)
        {
          ESP.restart();
        }
        if (strcmp(c, "clearEEPROM") == 0)
        {
          clearEEPROM();
          ESP.restart();
        }
      }
      
    }
    else if (meth == M_GET)
    {
      if (root.containsKey("outputs"))
      {
        bool outputPubArray[_cfg.numOutputs]; // array holding true or fasle for each of the output array positions
        for (int i = 0; i < _cfg.numOutputs; i++)
        {
          outputPubArray[i] = false; // dont pub by default
        }

        JsonVariant outputs = root["outputs"];
        if (outputs.is<JsonArray>())
        {
          int numOutputs = outputs.as<JsonArray>().size();
          dbf("Got outputs Array with %d outputs\n", numOutputs);
          if (numOutputs == 0) // if none defined, pub all
          {
            pubOutputs(getEvtID(sender));
          }
          else // pub all in list
          {
            for (int i = 0; i < numOutputs; i++)
            {
              int outputid = outputs.as<JsonArray>().get<signed int>(i);
              if (outputs[i].is<signed int>())
              {
                int index = getOutputArrayIndexByID(outputid);
                dbf("%i. Got array index %i for outputid %i\n", i, index, outputid);
                if (index != UNDEFINED)
                {
                  outputPubArray[i] = true;
                }
              } else
              {
                dbf("Output in list is not signed int %i\n",outputid);
              }
            }
            pubOutputs(outputPubArray, _cfg.numOutputs, getEvtID(sender));
          }
        }
      }

      if (root.containsKey("inputs"))
      {
        bool inputPubArray[_cfg.numInputs]; // array holding true or fasle for each of the input array positions
        for (int i = 0; i < _cfg.numInputs; i++)
        {
          inputPubArray[i] = false; // dont pub by default
        }

        JsonVariant inputs = root["inputs"];
        if (inputs.is<JsonArray>())
        {
          int numInputs = inputs.size();
          dbf("Got inputs Array with %d inputs\n", numInputs);
          if (numInputs == 0) // if none defined, pub all
          {
            pubInputs(getEvtID(sender));
          }
          else // pub all in list
          {
            for (int i = 0; i < numInputs; i++)
            {
              if (inputs[i].is<signed int>())
              {
                int index = getInputArrayIndexByID(inputs[i]);
                if (index != UNDEFINED)
                {
                  inputPubArray[i] = true;
                }
              }
            }
            pubInputs(inputPubArray, _cfg.numInputs, getEvtID(sender));
          }
        }
      }

      if (root.containsKey("info"))
      {
        const char * c = root["info"];
        //        }
        if (strcmp(c, "system") == 0)
        {
          pubInfo(E_REQUEST);
        }
        else if (strcmp(c, "state") == 0)
        {
          pubState(E_REQUEST);
        }
        else if (strcmp(c, "full") == 0)
        {
          pubFullState(E_REQUEST);
        }
      }
    }

    if (root.containsKey("cmd"))
    {
      const char * c = root["cmd"];
      if (strcmp(c, "reset") == 0)
      {
        resetPuzzle();
      }
      if (strcmp(c, "info") == 0)
      {
        pubInfo(E_REQUEST);
      }
      if (strcmp(c, "state") == 0)
      {
        pubFullState(E_REQUEST);
      }
      if (strcmp(c, "reboot") == 0)
      {
        ESP.restart();
      }
      if (strcmp(c, "clearEEPROM") == 0)
      {
        clearEEPROM();
        ESP.restart();
      }
    }

    if (root.containsKey("ota"))
    {
      const char* c = root["ota"];
      doOTA(c);
    }


    //    for (auto kv : root)
    //    {
    //        Serial.println(kv.key);
    //        Serial.println(kv.value.as<char*>());
    //    }

  }
}
