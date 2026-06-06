#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>
#include <WiFi.h>
#include <DNSServer.h>
#include <WebServer.h>
#include <WiFiManager.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <Preferences.h>

// --- CONFIGURAÇÃO ---
char mqtt_server[40] = "192.168.1.20"; // Valor padrão
const int mqtt_port = 1883;
const char* tracking_topic_discovery = "tracking/discovery";

unsigned long lastScanTime = 0;
const unsigned long scanInterval = 5000; 

// Pin definitions
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
Preferences preferences;

// Flag para salvar config
bool shouldSaveConfig = false;

void saveConfigCallback() {
  Serial.println("Configuração precisa ser salva");
  shouldSaveConfig = true;
}

void setup_wifi_manager() {
  preferences.begin("mqtt-config", false);
  String saved_mqtt = preferences.getString("server", "192.168.1.20");
  strcpy(mqtt_server, saved_mqtt.c_str());

  WiFiManagerParameter custom_mqtt_server("server", "IP do Broker MQTT", mqtt_server, 40);
  WiFiManager wifiManager;

  wifiManager.setSaveConfigCallback(saveConfigCallback);
  wifiManager.addParameter(&custom_mqtt_server);

  // Configura timeout para não travar se o WiFi cair depois
  wifiManager.setConfigPortalTimeout(180); // 3 minutos

  if (!wifiManager.autoConnect("ESP32-Scanner-Config")) {
    Serial.println("Falha ao conectar, reiniciando...");
    delay(3000);
    ESP.restart();
  }

  Serial.println("WiFi Conectado!");
  strcpy(mqtt_server, custom_mqtt_server.getValue());

  if (shouldSaveConfig) {
    preferences.putString("server", mqtt_server);
    Serial.println("Configuração de MQTT salva nas Preferences");
  }
  preferences.end();
}

void scan_tracking_tags() {
  unsigned long now = millis();
  if (now - lastScanTime < scanInterval) return;
  lastScanTime = now;

  Serial.println("Iniciando varredura WiFi para tags...");
  int n = WiFi.scanNetworks(false, false, false, 100); 
  
  if (n > 0) {
    for (int i = 0; i < n; ++i) {
      String bssid = WiFi.BSSIDstr(i);
      int rssi = WiFi.RSSI(i);
      
      StaticJsonDocument<128> doc;
      doc["mac"] = bssid;
      doc["rssi"] = rssi;
      doc["scanner"] = "esp32-central";

      char buffer[128];
      serializeJson(doc, buffer);
      client.publish(tracking_topic_discovery, buffer);
    }
  }
  WiFi.scanDelete();
}

void mqtt_callback(char* topic, byte* payload, unsigned int length) {
  String message = "";
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  StaticJsonDocument<200> doc;
  if (deserializeJson(doc, message) == DeserializationError::Ok) {
    bool authorized = doc["authorized"];
    const char* name = doc["name"];

    if (authorized) {
      Serial.printf("Acesso GARANTIDO para %s\n", name);
      digitalWrite(LED_GREEN_PIN, HIGH);
      delay(2000);
      digitalWrite(LED_GREEN_PIN, LOW);
    } else {
      Serial.println("Acesso NEGADO");
      for (int i = 0; i < 3; i++) {
        digitalWrite(LED_RED_PIN, HIGH);
        delay(200);
        digitalWrite(LED_RED_PIN, LOW);
        delay(200);
      }
    }
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Tentando conexão MQTT...");
    String clientId = "ESP32Scanner-";
    clientId += String(random(0xffff), HEX);
    
    if (client.connect(clientId.c_str())) {
      Serial.println("conectado");
    } else {
      Serial.print("falhou, rc=");
      Serial.print(client.state());
      Serial.println(" tentando novamente em 5 segundos");
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

  setup_wifi_manager();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(mqtt_callback);

  SPI.begin(18, 19, 23, 5); // SCK, MISO, MOSI, SS
  rfid.PCD_Init();
  
  Serial.println("Scanner Pronto");
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    // Se o WiFi cair, tenta reconectar sem travar o loop infinitamente
    // O WiFiManager cuida da reconexão em background se já foi configurado
  }

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  scan_tracking_tags();

  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) {
    return;
  }

  String uidStr = uidToString(rfid.uid);
  String responseTopic = "rfid/responses/" + uidStr;
  client.subscribe(responseTopic.c_str());

  StaticJsonDocument<200> doc;
  doc["uid"] = uidStr;
  char buffer[256];
  serializeJson(doc, buffer);
  client.publish("rfid/scans", buffer);

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
  
  delay(1000); 
}
