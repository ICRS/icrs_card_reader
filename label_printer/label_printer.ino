#include <Wire.h>
#include <PN532_I2C.h>
#include <PN532.h>
#include <NfcAdapter.h>
#include <HTTPClient.h>

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ArduinoJson.h>

#include <Fonts/FreeMono9pt7b.h>

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
      ; 
  }
  delay(1000);  

  display.clearDisplay();
  display.display();

  nfc.begin();
  initWiFi();
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
    
    String status = postIDToServer(tagId);
    display.println("------------");
  }
  delay(50);
}

void initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi ..");

  displayText("Connecting");

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  displayText("ready");
}

String postIDToServer(String id) {
  if ((WiFi.status() == WL_CONNECTED)) {
    Serial.println("Attempting to POST ID To Server");
    HTTPClient http;

    id.replace(" ", "");
    http.begin(host, port, "/project-box/assign/uuid?uuid=" + id);  //HTTP
    http.setAuthorization(USERNAME, PASSWORD);

    int response_code = http.GET();
    Serial.println(response_code);
    String s = "Failed to Query Server";

    if (response_code > 400) {
      displayText("Not Inducted");
      return;
    }

    if (response_code = 200) {
      s = http.getString();
      DynamicJsonDocument doc(1024);
      deserializeJson(doc, s);
      JsonObject obj = doc.as<JsonObject>();
      String name = obj["name"];
      String image = obj["image"];

      String s = "Welcome " + name;

      displayText(s);  
      printLabel(image);
    }
    http.end();
    return s;
  }

  return "";
}     

void displayText(String text){
  Serial.println(text);
  
  display.setTextSize(1);
  display.setFont();
  display.setTextColor(WHITE);
  display.setCursor(0, 11);

  display.println(text);
  display.display();
}

void printLabel(String text){
  // todo once ESC POS printer arrives
  // https://www.arduino.cc/reference/en/libraries/escposprinter/

}