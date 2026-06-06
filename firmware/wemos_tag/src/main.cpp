#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// --- CONFIGURAÇÃO WIFI ---
const char* ssid = "NOME_DO_SEU_WIFI";
const char* password = "SENHA_DO_SEU_WIFI";

// --- CONFIGURAÇÃO MQTT ---
const char* mqtt_server = "IP_DO_SEU_BROKER"; // Ex: 192.168.1.100
const char* mqtt_topic_heartbeat = "tracking/heartbeat";

WiFiClient espClient;
PubSubClient client(espClient);

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Conectando em ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi conectado");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
  Serial.print("MAC Address: ");
  Serial.println(WiFi.macAddress());
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
  setup_wifi();
  client.setServer(mqtt_server, 1883);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Envia um heartbeat a cada 3 segundos
  static unsigned long lastMsg = 0;
  unsigned long now = millis();
  if (now - lastMsg > 3000) {
    lastMsg = now;

    StaticJsonDocument<128> doc;
    doc["mac"] = WiFi.macAddress();
    doc["status"] = "online";
    doc["rssi"] = WiFi.RSSI(); // A tag também reporta como ela vê o roteador

    char buffer[128];
    serializeJson(doc, buffer);
    client.publish(mqtt_topic_heartbeat, buffer);
    
    Serial.print("Heartbeat enviado: ");
    Serial.println(buffer);
  }
}
