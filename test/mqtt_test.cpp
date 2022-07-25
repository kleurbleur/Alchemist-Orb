#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>


// SETTINGS
const char* ssid = "Wireless Funtime Palace";
const char* password = "radiorijnmond";
char hostname[] ="Orb";
char startMessage[] = "Orb is online";
char sub_topic[] = "Centrepiece_in";  // out en in topics are best to seperate
char pub_topic[] = "Centrepiece_out"; // out en in topics are best to seperate
IPAddress server(192, 168, 178, 215);



WiFiClient espClient;
PubSubClient client(espClient);

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

void callback(char* sub_topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(sub_topic);
  Serial.print("] ");
  for (int i=0;i<length;i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
  String msg = (char*)payload;
// INCOMING TEST MESSAGES
  if(msg.indexOf("start") >= 0){
    Serial.println("found start");
    client.publish(pub_topic,"running");
  } 
  if(msg.indexOf("shutdown") >= 0){
    Serial.println("found shutdown");
    client.publish(pub_topic,"halting");
  } 
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(hostname)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish(sub_topic, startMessage);
      // ... and resubscribe
      client.subscribe(sub_topic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}



void setup() {
  Serial.begin(9600);
  digitalWrite(18, LOW);
  initWiFi();

  client.setServer(server, 1883);
  client.setCallback(callback);

}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}