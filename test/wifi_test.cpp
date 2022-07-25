#include <Arduino.h>
#include <WiFi.h>

// Replace with your network credentials (STATION)
const char* ssid = "Wireless Funtime Palace";
const char* password = "radiorijnmond";

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

void setup() {
  Serial.begin(9600);
  initWiFi();
}

void loop() {
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
    Serial.print("RRSI: ");
    Serial.println(WiFi.RSSI());
    delay(3000);
}