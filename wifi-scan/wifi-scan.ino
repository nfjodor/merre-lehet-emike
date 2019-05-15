#include <Wire.h>
#include "ESP8266WiFi.h"
#include <math.h>
#include <Adafruit_NeoPixel.h>
#include "../variables.h"
#ifdef __AVR__
  #include <avr/power.h>
#endif

String chipId = String(ESP.getChipId());
#define LEDCOUNT 6
#define LedPin D2
#define LEDBRIGHTNESS 200
int ledArray[LEDCOUNT][6];
Adafruit_NeoPixel strip = Adafruit_NeoPixel(LEDCOUNT, LedPin, NEO_GRB + NEO_KHZ800);
int missedSearch = 0;
int displayPeak = 0;

void setup() {
  Serial.begin(115200);

  // Set WiFi to station mode and disconnect from an AP if it was previously connected
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  strip.begin();
  strip.show(); // Initialize all pixels to 'off'

  // fill led array with colors
  for (int i = 0; i < LEDCOUNT; i++) {
    if (i == 0) {
      ledArray[i][0] = 0;
      ledArray[i][1] = 0;
      ledArray[i][2] = LEDBRIGHTNESS;
    } else if (i == 1) {
      ledArray[i][0] = LEDBRIGHTNESS / 3;
      ledArray[i][1] = 0;
      ledArray[i][2] = LEDBRIGHTNESS;
    } else if (i == 2) {
      ledArray[i][0] = LEDBRIGHTNESS;
      ledArray[i][1] = 0;
      ledArray[i][2] = LEDBRIGHTNESS;
    } else if (i == 3) {
      ledArray[i][0] = LEDBRIGHTNESS;
      ledArray[i][1] = 0;
      ledArray[i][2] = LEDBRIGHTNESS / 2;
    } else if (i == 4) {
      ledArray[i][0] = LEDBRIGHTNESS;
      ledArray[i][1] = 0;
      ledArray[i][2] = LEDBRIGHTNESS / 5;
    } else {
      ledArray[i][0] = LEDBRIGHTNESS;
      ledArray[i][1] = 0;
      ledArray[i][2] = 0;
    }
  }

  Serial.println(chipId);
}

void loop() {
  scanNetwotrk();
}

void scanNetwotrk() {
  int strength = 0;
  String ssid;
  uint8_t encryptionType;
  int32_t RSSI;
  int32_t STRONGEST_RSSI = -100;
  uint8_t * BSSID;
  int32_t channel;
  bool isHidden;
  int netcount = WiFi.scanNetworks(false, true, 1);
  for (int n = 0; n < netcount; n++) {
    WiFi.getNetworkInfo(n, ssid, encryptionType, RSSI, BSSID, channel, isHidden);
    if (WiFi.BSSIDstr(n) == findMacAddress) {
      missedSearch = 0;
      if (RSSI > -55) {
        strength = 6;
      } else if (RSSI > -58) {
        strength = 5;
      } else if (RSSI > -62) {
        strength = 4;
      } else if (RSSI > -70) {
        strength = 3;
      } else if (RSSI > -80) {
        strength = 2;
      } else if (RSSI > -90) {
        strength = 1;
      } else {
        strength = 0;
      }
      for (int i = 0; i < LEDCOUNT; i++) // update ledstrip
      {
        if (strength <= i) {
          strip.setPixelColor(i, 0, 0, 0);
        } else {
          strip.setPixelColor(i, ledArray[i][0], ledArray[i][1], ledArray[i][2]);
        }
      }
      strip.show();
      break; // exit for loop, because we found a good AP
    }
  }
  if (strength == 0) {
      missedSearch += 1;

      if (missedSearch >= 5) {
        missedSearch = 0;
        for (int i = 0; i < LEDCOUNT; i++) // update ledstrip
        {
          strip.setPixelColor(i, 0, 0, 0);
        }
        strip.show();
      }
  }
}

