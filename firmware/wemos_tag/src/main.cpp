#include <LittleFS.h>
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// --- CONFIGURAÇÃO ---
char mqtt_server[40] = "192.168.1.20"; // Valor padrão
const char* mqtt_topic_heartbeat = "tracking/heartbeat";

WiFiClient espClient;
PubSubClient client(espClient);

// Flag para salvar config
bool shouldSaveConfig = false;

// Callback que indica que a configuração deve ser salva
void saveConfigCallback() {
  Serial.println("Configuração precisa ser salva");
  shouldSaveConfig = true;
}

void setup_wifi_manager() {
  // LittleFS para persistência
  if (LittleFS.begin()) {
    Serial.println("LittleFS montado");
    if (LittleFS.exists("/config.json")) {
      File configFile = LittleFS.open("/config.json", "r");
      if (configFile) {
        size_t size = configFile.size();
        std::unique_ptr<char[]> buf(new char[size]);
        configFile.readBytes(buf.get(), size);
        StaticJsonDocument<256> doc;
        if (deserializeJson(doc, buf.get()) == DeserializationError::Ok) {
          strcpy(mqtt_server, doc["mqtt_server"]);
        }
      }
    }
  } else {
    Serial.println("Falha ao montar LittleFS");
  }

  WiFiManagerParameter custom_mqtt_server("server", "IP do Broker MQTT", mqtt_server, 40);
  WiFiManager wifiManager;

  // Callback de salvamento
  wifiManager.setSaveConfigCallback(saveConfigCallback);
  wifiManager.addParameter(&custom_mqtt_server);

  // Tenta conectar, se falhar abre o AP "WemosTag-Config"
  if (!wifiManager.autoConnect("WemosTag-Config")) {
    Serial.println("Falha ao conectar e timeout do portal atingido");
    delay(3000);
    ESP.restart();
  }

  // Se chegou aqui, conectou no WiFi
  Serial.println("WiFi conectado!");
  strcpy(mqtt_server, custom_mqtt_server.getValue());

  // Salva o novo IP se necessário
  if (shouldSaveConfig) {
    StaticJsonDocument<256> doc;
    doc["mqtt_server"] = mqtt_server;
    File configFile = LittleFS.open("/config.json", "w");
    if (configFile) {
      serializeJson(doc, configFile);
      configFile.close();
      Serial.println("Configuração salva com sucesso");
    }
  }

  Serial.print("MQTT Broker IP: ");
  Serial.println(mqtt_server);
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Tentando conexão MQTT...");
    String clientId = "WemosTag-";
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

void setup() {
  Serial.begin(115200);
  setup_wifi_manager();
  client.setServer(mqtt_server, 1883);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  static unsigned long lastMsg = 0;
  unsigned long now = millis();
  if (now - lastMsg > 3000) {
    lastMsg = now;

    StaticJsonDocument<128> doc;
    doc["mac"] = WiFi.macAddress();
    doc["status"] = "online";
    doc["rssi"] = WiFi.RSSI();

    char buffer[128];
    serializeJson(doc, buffer);
    client.publish(mqtt_topic_heartbeat, buffer);
    
    Serial.print("Heartbeat enviado: ");
    Serial.println(buffer);
  }
}
