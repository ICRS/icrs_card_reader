#include <SPI.h>
#include <MFRC522.h>

#include <WiFi.h>
#include <WiFiMulti.h>

#include <HTTPClient.h>

#define SS_PIN 5
#define RST_PIN 0

#define SERVER_IP server_ip_address
#define SERVER_PORT server_port

#define SECRET some_secret_key
#define PASSWORD password
#define SSID ssid

MFRC522 rfid(SS_PIN, RST_PIN); // Instance of the class

MFRC522::MIFARE_Key key; 

WiFiMulti wifiMulti;

void setup() { 
  Serial.begin(9600);
  SPI.begin(); // Init SPI bus
  rfid.PCD_Init(); // Init MFRC522 

  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }

  Serial.println(F("This code scan the MIFARE Classsic NUID."));
  Serial.print(F("Using the following key:"));
  printHex(key.keyByte, MFRC522::MF_KEY_SIZE);

  // Setup WiFi
  wifiMulti.addAP(SSID, PASSWORD);

}
 
void loop() {

  // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
  if ( ! rfid.PICC_IsNewCardPresent())
    return;

  // Verify if the NUID has been readed
  rfid.PICC_ReadCardSerial();

  Serial.print(F("PICC type: "));
  MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
  Serial.println(rfid.PICC_GetTypeName(piccType));

  // Check is the PICC of Classic MIFARE type
  if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI &&  
    piccType != MFRC522::PICC_TYPE_MIFARE_1K &&
    piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
    Serial.println(F("Your tag is not of type MIFARE Classic."));
    return;
  }
  
  Serial.println(F("The NUID tag is:"));
  Serial.print(F("In hex: "));
  printHex(rfid.uid.uidByte, rfid.uid.size);

  Serial.println();
  Serial.print(F("In dec: "));
  printDec(rfid.uid.uidByte, rfid.uid.size);
  Serial.println();

  Serial.println(idToString(rfid.uid.uidByte, rfid.uid.size));

  // POST
  Serial.println(postIDToServer(idToString(rfid.uid.uidByte, rfid.uid.size)));

  // Halt PICC
  rfid.PICC_HaltA();

  // Stop encryption on PCD
  rfid.PCD_StopCrypto1();

  delay(100);
}

/**
 *  Helper function for converting nid to a string
 */
const String idToString(const byte buffer[], byte bufferSize)
{
  String s = "";
  for(byte i=0; i<bufferSize; i++)
    s += String(buffer[i], HEX);
  
  return s;
} 

/**
 * Helper routine to dump a byte array as hex values to Serial. 
 */
void printHex(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}

/**
 * Helper routine to dump a byte array as dec values to Serial.
 */
void printDec(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], DEC);
  }
}


int postIDToServer(String id)
{
  if((wifiMulti.run() == WL_CONNECTED)) {
    Serial.println("Attempting to POST ID To Server");
    HTTPClient http;

    // configure traged server and url
    //http.begin("https://www.howsmyssl.com/a/check", ca); //HTTPS
    // http.begin(SERVER_IP, SERVER_PORT, "/addUser"); //HTTP
    http.begin(SERVER_IP, SERVER_PORT, "/setCanPrint"); //HTTP
    http.addHeader("Content-Type", "application/json");
  
    return http.POST("{\"id\":\"" + id + "\", \"secret\":\"" + SECRET + "\"}");
  }

  return -1;
}