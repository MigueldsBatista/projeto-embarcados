#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// --- Configuration ---
const char* ssid = "FAMILIA BATISTA_2G";
const char* password = "ericabatista1601";
const char* mqtt_server = "192.168.1.20";
const int mqtt_port = 1883;

// ----------------------

// Pin definitions
#ifndef RC522_SCK_PIN
#define RC522_SCK_PIN 18
#endif
#ifndef RC522_MOSI_PIN
#define RC522_MOSI_PIN 23
#endif
#ifndef RC522_MISO_PIN
#define RC522_MISO_PIN 19
#endif
#ifndef RC522_SS_PIN
#define RC522_SS_PIN 5
#endif
#ifndef RC522_RST_PIN
#define RC522_RST_PIN 22
#endif

#ifndef LED_RED_PIN
#define LED_RED_PIN 4
#endif
#ifndef LED_GREEN_PIN
#define LED_GREEN_PIN 2
#endif

MFRC522 rfid(RC522_SS_PIN, RC522_RST_PIN);
WiFiClient espClient;
PubSubClient client(espClient);

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void mqtt_callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  
  String message = "";
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.println(message);

  StaticJsonDocument<200> doc;
  DeserializationError error = deserializeJson(doc, message);
  if (error) {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.f_str());
    return;
  }

  bool authorized = doc["authorized"];
  const char* name = doc["name"];

  if (authorized) {
    Serial.printf("Access GRANTED for %s\n", name);
    digitalWrite(LED_GREEN_PIN, HIGH);
    digitalWrite(LED_RED_PIN, LOW);
    delay(2000);
    digitalWrite(LED_GREEN_PIN, LOW);
  } else {
    Serial.println("Access DENIED");
    for (int i = 0; i < 3; i++) {
      digitalWrite(LED_RED_PIN, HIGH);
      delay(200);
      digitalWrite(LED_RED_PIN, LOW);
      delay(200);
    }
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);
    
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

String uidToString(const MFRC522::Uid &uid) {
  String out;
  for (byte i = 0; i < uid.size; i++) {
    if (i) out += ':';
    if (uid.uidByte[i] < 0x10) out += '0';
    out += String(uid.uidByte[i], HEX);
  }
  out.toUpperCase();
  return out;
}

void setup() {
  Serial.begin(9600);
  pinMode(LED_RED_PIN, OUTPUT);
  pinMode(LED_GREEN_PIN, OUTPUT);
  
  digitalWrite(LED_RED_PIN, LOW);
  digitalWrite(LED_GREEN_PIN, LOW);

  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(mqtt_callback);

  SPI.begin(RC522_SCK_PIN, RC522_MISO_PIN, RC522_MOSI_PIN, RC522_SS_PIN);
  rfid.PCD_Init();
  
  Serial.println("RFID Ready");
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) {
    delay(50);
    return;
  }

  String uidStr = uidToString(rfid.uid);
  Serial.print("Scanned UID: ");
  Serial.println(uidStr);

  // Subscribe to the response topic for this specific UID
  String responseTopic = "rfid/responses/" + uidStr;
  client.subscribe(responseTopic.c_str());

  // Publish the scan
  StaticJsonDocument<200> doc;
  doc["uid"] = uidStr;
  char buffer[256];
  serializeJson(doc, buffer);
  client.publish("rfid/scans", buffer);

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
  
  delay(1000); // Prevent spamming
}
