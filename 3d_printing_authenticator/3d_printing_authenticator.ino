#include <Wire.h>
#include <PN532_I2C.h>
#include <PN532.h>
#include <NfcAdapter.h>

#include <WiFi.h>
#include <HTTPClient.h>
#include <ESP32Time.h>

#include <FastLED.h>

#include "config.h"

#define NUM_LEDS 2

ESP32Time rtc(3600);
int loopCounter = 0;
#define LOOP_RESET = 10 * 60 * 2; // ~ 2 min


PN532_I2C pn532_i2c(Wire);
NfcAdapter nfc = NfcAdapter(pn532_i2c);
String tagId = "None";
byte nuidPICC[4];

#define LED_PIN 13
CRGB leds[NUM_LEDS];

void setup() {
  FastLED.addLeds<NEOPIXEL, LED_PIN>(leds, NUM_LEDS);  // GRB ordering is assumed

  Serial.begin(9600);
  nfc.begin();
  // Setup WiFi
  initWiFi();
  pinMode(LED_PIN, OUTPUT);
}

void loop() {
  checkWiFi();
  readNFC();

  delay(100);
}

void checkWiFi(){
  // Only check wifi every ~ 2min
  // check by WiFi status and time server ping

  loopCounter++;
  if(loopCounter <= LOOP_RESET) return;
  loopCounter = 0;

  if (WiFi.status() != WL_CONNECTED) reconnectWiFi();
  if (!rtc.getEpoch()) reconnectWiFi();
}

void reconnectWiFi(){
  singleBlink(CRGB::Orange);
  WiFi.reconnect();
  waitForWiFi();
}

void waitForWiFi(){
  Serial.print("Connecting to WiFi ..");
  
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
  if (!nfc.tagPresent()) return;

  singleBlink(CRGB::Blue);
  
  NfcTag tag = nfc.read();
  tag.print();
  tagId = tag.getUidString();
  bool status = postIDToServer(tagId);
  Serial.println(status);
  if (status) blink(CRGB::Green);
  else blink(CRGB::Red);
}

void initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  waitForWiFi();
}

int postIDToServer(String id) {
  if ((WiFi.status() != WL_CONNECTED)) reconnectWiFi();

  Serial.println("Attempting to POST ID To Server");
  HTTPClient http;

  http.begin(SERVER_IP, SERVER_PORT, "/setPrintWindow");  //HTTP
  http.setHttpResponseTimeout(2000);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Accept", "*/*");

  http.POST("{\"id\":\"" + id + "\", \"secret\":\"" + SECRET + "\"}");
  
  if(http.status != 408) return http.getString() == "SUCCESS";

  reconnectWiFi();
}
