#include <Wire.h>
#include <PN532_I2C.h>
#include <PN532.h>
#include <NfcAdapter.h>

#include <WiFi.h>
#include <HTTPClient.h>

#include "config.h"

PN532_I2C pn532_i2c(Wire);
NfcAdapter nfc = NfcAdapter(pn532_i2c);
String tagId = "None";
byte nuidPICC[4];


void setup() { 
  Serial.begin(9600);
  nfc.begin();
  // Setup WiFi
  initWiFi();

}
 
void loop() {
  readNFC();
}

void readNFC() 
{
 if (nfc.tagPresent())
 {
   NfcTag tag = nfc.read();
   tag.print();
   tagId = tag.getUidString();
   postIDToServer(tagId);
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
  }
  Serial.println(WiFi.localIP());
}

int postIDToServer(String id)
{
  if((WiFi.status() == WL_CONNECTED)) {
    Serial.println("Attempting to POST ID To Server");
    HTTPClient http;

    // configure traged server and url
    //http.begin("https://www.howsmyssl.com/a/check", ca); //HTTPS
    // http.begin(SERVER_IP, SERVER_PORT, "/addUser"); //HTTP
    http.begin(SERVER_IP, SERVER_PORT, "/setPrintWindow"); //HTTP
    http.addHeader("Content-Type", "application/json");
  
    return http.POST("{\"id\":\"" + id + "\", \"secret\":\"" + SECRET + "\"}");
  }

  return -1;
}