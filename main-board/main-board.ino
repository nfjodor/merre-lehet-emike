#include <SPI.h>
#include <Wire.h>
#include <Adafruit_PN532.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <Adafruit_NeoPixel.h>
#include "variables.h"
#ifdef __AVR__
  #include <avr/power.h>
#endif

#define PN532_SCK   (D5)
#define PN532_MOSI  (D3)
#define PN532_SS    (D2)
#define PN532_MISO  (D4)
#define PN532_IRQ   (D1)
#define PN532_RESET (D0)

#define FEEDBACK_PIN    (D7)
#define WIFI_BROADCAST_DEVICE_PIN (D6)

Adafruit_PN532 nfc(PN532_SCK, PN532_MISO, PN532_MOSI, PN532_SS);
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(1, FEEDBACK_PIN, NEO_GRB + NEO_KHZ800);

String captureUrl = String(baseUrl + "catch=");
String startTimerUrl = String(baseUrl + "start_timer=");
String heartbeatUrl = String(baseUrl + "heartbeat=");
String uuid = String(255); 
unsigned long previousMillis = 0;
long heatbeatTime = 60000;
int delayval = 50;
String stateStartTimer = "start_timer";
String stateSearch = "search";
String stateHeartbeat = "heartbeat";
String mode = stateSearch;

void setUuid(uint8_t uid[], uint8_t uidLength) {
  uuid = "";
  for (uint8_t i=0; i < uidLength; i++) {
      uuid += uid[i];
  }
  Serial.println(uuid);
}

void builtInLedFlash() {
  digitalWrite(LED_BUILTIN, LOW);
  delay(delayval * 2);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(delayval * 2);
}
void errorHandle(String errorMessage, int flashCount) {
  Serial.print(errorMessage);
  for (int i=0; i < flashCount; i++) {
    builtInLedFlash();
  }
}

void setFeedbackLedColor(int r=0, int g=0, int b=0) {
  pixels.setPixelColor(0, pixels.Color(r,g,b));
  pixels.show();
}

void getCall(String url, String nextState) {
  HTTPClient http;
  Serial.println(url + uuid);
  http.begin(url + uuid);

  int isWifiDevicePinEnabled = HIGH;
  
  int httpCode = http.GET();
  if (httpCode > 0) {
    String payload = http.getString();
    Serial.println(payload);
    if (payload == "true") {
      if (nextState == "heartbeat") {
        Serial.println("Heartbeat OK");
        return;
      }
      mode = String(nextState);
      if (mode == stateStartTimer) {
        isWifiDevicePinEnabled = LOW;
        setFeedbackLedColor(0, 255, 0);
      }
      delay(2000);
    } else {
      errorHandle(String("Response payload was not valid: \n" + payload), 2);
    }
  } else {
    errorHandle(String("Fail: \n" + httpCode), 3);
  }
  
  digitalWrite(WIFI_BROADCAST_DEVICE_PIN, isWifiDevicePinEnabled);
}


void setup(void) {
  nfc.begin();
  uint32_t versiondata = nfc.getFirmwareVersion();

  Serial.begin(115200);

  pinMode(WIFI_BROADCAST_DEVICE_PIN, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  
  if (! versiondata) {
    errorHandle("Didn't find PN53x board", 3);
    while (1); // halt
  }

  WiFi.begin(ssid, password);
  Serial.print("Connecting..");

  while (WiFi.status() != WL_CONNECTED) {
    builtInLedFlash();
    Serial.print(".");
    delay(1000);
  }

  Serial.println("\nConnected");

  Serial.print("Found chip PN5"); Serial.println((versiondata>>24) & 0xFF, HEX);
  Serial.print("Firmware ver. "); Serial.print((versiondata>>16) & 0xFF, DEC);
  Serial.print('.'); Serial.println((versiondata>>8) & 0xFF, DEC);

  nfc.setPassiveActivationRetries(10);
  nfc.SAMConfig();

  Serial.println("Waiting for an ISO14443A Card ...");
  digitalWrite(LED_BUILTIN, HIGH);
  digitalWrite(WIFI_BROADCAST_DEVICE_PIN, HIGH);
  pixels.begin();
}

void loop(void) {
  boolean success;
  uint8_t uidLength;
  uint8_t uid[] = {0, 0, 0, 0, 0, 0, 0};
  unsigned long currentMillis = millis();
  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, &uid[0], &uidLength);

  if (uuid && (currentMillis - previousMillis >= heatbeatTime)) {
    getCall(heartbeatUrl, stateHeartbeat);
    previousMillis = currentMillis;
  }
  if (mode == stateSearch || mode == stateStartTimer) {
    String newMode = stateStartTimer;
    String getUrl = captureUrl;
    if (mode == stateStartTimer) {
      setFeedbackLedColor(0, 255, 0);
      delay(delayval);
      setFeedbackLedColor();
      newMode = stateSearch;
      getUrl = startTimerUrl;
    }
    Serial.println("Entering " + mode + " mode");
    if (success) {
      setUuid(uid, uidLength);
      getCall(getUrl, newMode);
    }
    delay(500);
  }
}
