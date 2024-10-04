#include <Wire.h>
#include <PN532_I2C.h>
#include <PN532.h>
#include <NfcAdapter.h>
#include <HTTPClient.h>

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ArduinoJson.h>

#include <Fonts/FreeMono9pt7b.h>

#include "config.h"

// #include <Arduino_JSON.h>
Adafruit_SSD1306 display = Adafruit_SSD1306(128, 64, &Wire);
// #include "config.h"

#define SCREEN_ADDRESS 0x3C

PN532_I2C pn532_i2c(Wire);
NfcAdapter nfc = NfcAdapter(pn532_i2c);
String tagId = "None";
byte nuidPICC[4];



void setup() {
  Serial.begin(9600);
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ;  // Don't proceed, loop forever
  }
  display.display();
  delay(1000);  // Pause for 2 seconds

  // Clear the buffer
  display.clearDisplay();
  display.display();

  // display.setFont(&FreeMono9pt7b);
  display.setTextSize(1);
  display.setTextColor(WHITE);

  nfc.begin();
  // Setup WiFi
  initWiFi();
}

void displayText(String text) {
  display.setTextSize(1);
  display.setFont();
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println(text);
  display.display();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    initWiFi();
  }
  delay(100);
  readNFC();
}

void readNFC() {
  if (nfc.tagPresent()) {
    display.clearDisplay();
    NfcTag tag = nfc.read();
    tag.print();
    
    // tagId = tag.getUidString();
    int uidLength = tag.getUidLength();
    byte uid[uidLength];
    tag.getUid(uid, uidLength);
    
    String tagId = "";
    for(size_t i = 0; i < uidLength; i++)
    {
      if(uid[i] < 16)
      {
        tagId += "0";
      }
      tagId += String(uid[i], HEX);
    }
    tagId.toUpperCase();
    
    display.setFont();
    displayText(tagId);


    String status = postIDToServer(tagId);
    display.println("------------");

    display.setCursor(0, 40);
    display.setFont(&FreeMono9pt7b);

    Serial.println(status);
    // Serial.println(http)

    // display.setTextSize(1.5);
    display.setTextColor(WHITE);
    display.println(status);
    display.display();
  }
  delay(50);
}

void initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi ..");

  display.clearDisplay();  
  display.setFont(&FreeMono9pt7b);
  display.setCursor(0, 11);
  display.println("Connecting");
  display.display();

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println(WiFi.localIP());
  display.clearDisplay();
  display.setCursor(0, 11);
  display.setFont(&FreeMono9pt7b);

  display.println("Ready");
  display.display();


}

String postIDToServer(String id) {
  if ((WiFi.status() == WL_CONNECTED)) {
    Serial.println("Attempting to POST ID To Server");
    HTTPClient http;

    id.replace(" ", "%20");
    http.begin(host, port, "/member/permissions/uuid?uuid=" + id);  //HTTP
    http.setAuthorization(USERNAME, PASSWORD);

    int response_code = http.GET();
    Serial.println(response_code);
    String s = "Failed to Query Server";

    if (response_code > 0) {
      s = http.getString();
      DynamicJsonDocument doc(1024);
      deserializeJson(doc, s);
      JsonObject obj = doc.as<JsonObject>();
      s = "";
      display.println("SC: " + obj["shortcode"].as<String>());
      s += obj["inducted"].as<String>() != "null" ? "Inducted" : "Nope! F OFF";
    }
    Serial.println(s);
    http.end();
    return s;
  }

  return "";
}