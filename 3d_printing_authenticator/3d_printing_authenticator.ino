#include <Wire.h>
#include <PN532_I2C.h>
#include <PN532.h>
#include <NfcAdapter.h>

#include <WiFi.h>
#include <HTTPClient.h>

#include <FastLED.h>

#include "config.h"

#define NUM_LEDS 2

PN532_I2C pn532_i2c(Wire);
NfcAdapter nfc = NfcAdapter(pn532_i2c);
String tagId = "None";
byte nuidPICC[4];

#define DATA_PIN 13
CRGB leds[NUM_LEDS];

void setup() {
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);  // GRB ordering is assumed

  Serial.begin(9600);
  nfc.begin();
  // Setup WiFi
  initWiFi();
  pinMode(DATA_PIN, OUTPUT);
}

void loop() {
  checkWiFi();
  readNFC();
}

void checkWiFi(){
   if (WiFi.status() == WL_CONNECTED) return;

   WiFi.reconnect();
   while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    singleBlink(CRGB::Yellow);
    delay(1000);
  }
  Serial.println(WiFi.localIP());
  singleBlink(CRGB::Green); 
}

void singleBlink(const CRGB::HTMLColorCode colour) 
{
    for(int j = 0; j < NUM_LEDS; j++) 
    {
      leds[j] = colour;
      FastLED.show();
      delay(200);
    }
    

    delay(600);
    for(int j = 0; j < NUM_LEDS; j++) 
    {
      leds[j] = CRGB::Black;
    }
    FastLED.show();
}


void blink(const CRGB::HTMLColorCode colour) 
{
  for(int i =0; i < 5; i++) 
  {
    for(int j = 0; j < NUM_LEDS; j++) 
    {
      leds[j] = colour;      
    }
    FastLED.show();

    delay(500);
    for(int j = 0; j < NUM_LEDS; j++) 
    {
      leds[j] = CRGB::Black;
    }
    FastLED.show();

    delay(500);
    
  }

}

void readNFC() {
  if (nfc.tagPresent()) {
    NfcTag tag = nfc.read();
    tag.print();
    tagId = tag.getUidString();
    bool status = postIDToServer(tagId);
    Serial.println(status);
    if (status) 
    {
      blink(CRGB::Green);
    } else 
    {
      blink(CRGB::Red);
    }
    // Serial.println(http)
  }
  delay(50);
}

void initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
    singleBlink(CRGB::Yellow);
  }
  Serial.println(WiFi.localIP());
  singleBlink(CRGB::Green);
}

int postIDToServer(String id) {
  if ((WiFi.status() == WL_CONNECTED)) {
    Serial.println("Attempting to POST ID To Server");
    HTTPClient http;

    // configure traged server and url
    //http.begin("https://www.howsmyssl.com/a/check", ca); //HTTPS
    // http.begin(SERVER_IP, SERVER_PORT, "/addUser"); //HTTP
    http.begin(SERVER_IP, SERVER_PORT, "/setPrintWindow");  //HTTP
    http.addHeader("Content-Type", "application/json");
    http.addHeader("Accept", "*/*");

    http.POST("{\"id\":\"" + id + "\", \"secret\":\"" + SECRET + "\"}");
    // Serial.print("Status: ");
    // Serial.println(http.getString());
    return http.getString() == "SUCCESS";
  }

  return -1;
}
