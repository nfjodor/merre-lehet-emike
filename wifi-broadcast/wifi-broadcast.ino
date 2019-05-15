#include <ESP8266WiFi.h>
#include <Arduino.h>
#include "../variables.h"
extern "C" {
  #include <user_interface.h>
}
int isAPEnabled;
int switchPin = D1; // GPIO5
void setup() {
  Serial.begin(115200);
  delay(10);
  
  WiFi.mode(WIFI_AP);
  wifi_set_macaddr(1, mac);
  WiFi.softAP("Emike is here", "nothing-to-see-here", 1, true); // false = show ap, true = hide ap
  Serial.println(WiFi.macAddress());
  Serial.println(WiFi.localIP());
  pinMode(switchPin, INPUT);
}
void loop() {
  isAPEnabled = digitalRead(switchPin);
  if (isAPEnabled) {
    WiFi.mode(WIFI_AP);
  } else {
    WiFi.mode(WIFI_STA);
  }
  delay(1000);
}
