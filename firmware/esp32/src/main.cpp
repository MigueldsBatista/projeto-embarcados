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
#include <stdlib.h>

// --- CONFIGURAÇÃO ---
char mqtt_server[40] = "192.168.1.20"; // Valor padrão
const int mqtt_port = 1883;
const char* tracking_topic_discovery = "tracking/discovery";
const char* tracking_topic_benchmark_data = "tracking/benchmark/data";
const char* tracking_topic_benchmark_perf = "tracking/benchmark/perf";
const char* tracking_topic_benchmark_summary = "tracking/benchmark/summary";

#ifndef TRACKING_BENCHMARK_ENABLED
#define TRACKING_BENCHMARK_ENABLED 0
#endif

#ifndef TRACKING_BENCHMARK_WINDOW_SIZE
#define TRACKING_BENCHMARK_WINDOW_SIZE 256
#endif

#ifndef TRACKING_BENCHMARK_RING_CAPACITY
#define TRACKING_BENCHMARK_RING_CAPACITY 512
#endif

#ifndef TRACKING_BENCHMARK_BATCH_SIZE
#define TRACKING_BENCHMARK_BATCH_SIZE 10
#endif

#ifndef TRACKING_BENCHMARK_NETWORK_DELAY_MS
#define TRACKING_BENCHMARK_NETWORK_DELAY_MS 1
#endif

#ifndef TRACKING_BENCHMARK_SNAPSHOT_LIMIT
#define TRACKING_BENCHMARK_SNAPSHOT_LIMIT 24
#endif

const uint32_t kTrackingBenchmarkScales[] = {100, 5000, 20000};
const size_t kTrackingBenchmarkScaleCount = sizeof(kTrackingBenchmarkScales) / sizeof(kTrackingBenchmarkScales[0]);

unsigned long lastScanTime = 0;
const unsigned long scanInterval = 5000; 
bool isScanning = false;

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
bool trackingBenchmarkRan = false;

// Global states for LED feedback (Task 1)
bool ledFeedbackActive = false;
unsigned long ledFeedbackStart = 0;
bool ledFeedbackType = false; // true for Green, false for Red blink

// FreeRTOS Task Handles and Queues (Task 2)
TaskHandle_t mqttTaskHandle = NULL;
TaskHandle_t rfidTaskHandle = NULL;
QueueHandle_t rfidQueue = NULL;

struct RfidScan {
  char uid[32];
};

enum class TrackingBenchmarkApproach : uint8_t {
  Inefficient = 0,
  Circular = 1
};

struct TrackingSample {
  char mac[18];
  int rssi;
  char scanner[20];
  uint32_t timestampUs;
};

struct TrackingPerfStats {
  uint32_t targetN = 0;
  uint32_t processed = 0;
  uint32_t minLatencyUs = UINT32_MAX;
  uint32_t maxLatencyUs = 0;
  uint64_t sumLatencyUs = 0;
  uint32_t minHeap = UINT32_MAX;
  uint32_t maxHeap = 0;
  uint32_t dropped = 0;
};

class TrackingInefficientWindow {
 public:
  explicit TrackingInefficientWindow(size_t maxSamples) : maxSamples_(maxSamples) {}

  ~TrackingInefficientWindow() {
    clear();
  }

  bool push(const TrackingSample& sample) {
    size_t nextSize = size_ < maxSamples_ ? size_ + 1 : maxSamples_;
    TrackingSample* next = static_cast<TrackingSample*>(malloc(nextSize * sizeof(TrackingSample)));
    if (!next) {
      return false;
    }

    if (size_ < maxSamples_) {
      for (size_t i = 0; i < size_; ++i) {
        next[i] = data_[i];
      }
      next[size_] = sample;
    } else {
      for (size_t i = 1; i < size_; ++i) {
        next[i - 1] = data_[i];
      }
      next[nextSize - 1] = sample;
    }

    free(data_);
    data_ = next;
    size_ = nextSize;
    return true;
  }

  void clear() {
    free(data_);
    data_ = nullptr;
    size_ = 0;
  }

 private:
  TrackingSample* data_ = nullptr;
  size_t size_ = 0;
  size_t maxSamples_ = 0;
};

class TrackingRingBuffer {
 public:
  bool push(const TrackingSample& sample) {
    if (count_ == TRACKING_BENCHMARK_RING_CAPACITY) {
      tail_ = (tail_ + 1) % TRACKING_BENCHMARK_RING_CAPACITY;
      --count_;
      ++dropped_;
    }
    data_[head_] = sample;
    head_ = (head_ + 1) % TRACKING_BENCHMARK_RING_CAPACITY;
    ++count_;
    return true;
  }

  bool pop(TrackingSample& sample) {
    if (count_ == 0) {
      return false;
    }
    sample = data_[tail_];
    tail_ = (tail_ + 1) % TRACKING_BENCHMARK_RING_CAPACITY;
    --count_;
    return true;
  }

  size_t size() const {
    return count_;
  }

  uint32_t dropped() const {
    return dropped_;
  }

  void clear() {
    head_ = 0;
    tail_ = 0;
    count_ = 0;
    dropped_ = 0;
  }

 private:
  TrackingSample data_[TRACKING_BENCHMARK_RING_CAPACITY];
  size_t head_ = 0;
  size_t tail_ = 0;
  size_t count_ = 0;
  uint32_t dropped_ = 0;
};

TrackingInefficientWindow trackingInefficientWindow(TRACKING_BENCHMARK_WINDOW_SIZE);
TrackingRingBuffer trackingRingBuffer;

// Flag para salvar config
bool shouldSaveConfig = false;

const char* trackingBenchmarkApproachToStr(TrackingBenchmarkApproach approach) {
  return approach == TrackingBenchmarkApproach::Inefficient ? "inefficient" : "circular";
}

void copyToFixedBuffer(char* destination, size_t destinationSize, const String& value) {
  value.toCharArray(destination, destinationSize);
}

TrackingSample buildTrackingSample(const String& mac, int rssi, const char* scannerLabel) {
  TrackingSample sample;
  copyToFixedBuffer(sample.mac, sizeof(sample.mac), mac);
  copyToFixedBuffer(sample.scanner, sizeof(sample.scanner), String(scannerLabel));
  sample.rssi = rssi;
  sample.timestampUs = micros();
  return sample;
}

void publishTrackingBenchmarkPerf(TrackingBenchmarkApproach approach, const TrackingPerfStats& stats, uint32_t latencyUs, uint32_t index) {
  StaticJsonDocument<256> doc;
  doc["approach"] = trackingBenchmarkApproachToStr(approach);
  doc["n_target"] = stats.targetN;
  doc["index"] = index;
  doc["latency_us"] = latencyUs;
  doc["heap_free"] = ESP.getFreeHeap();

  char payload[256];
  size_t len = serializeJson(doc, payload);
  client.publish(tracking_topic_benchmark_perf, payload, len);
}

void publishTrackingBenchmarkData(TrackingBenchmarkApproach approach, uint32_t targetN, TrackingSample* samples, size_t count, uint32_t batchIndex) {
  // Use a larger buffer for batches of 10 samples
  StaticJsonDocument<2048> doc;
  doc["approach"] = trackingBenchmarkApproachToStr(approach);
  doc["n_target"] = targetN;
  doc["batch"] = batchIndex;
  JsonArray rows = doc.createNestedArray("samples");
  for (size_t i = 0; i < count; ++i) {
    JsonObject row = rows.createNestedObject();
    row["mac"] = samples[i].mac;
    row["rssi"] = samples[i].rssi;
    row["scanner"] = samples[i].scanner;
    row["t_us"] = samples[i].timestampUs;
  }

  char payload[2048];
  size_t len = serializeJson(doc, payload);
  if (len > 0) {
    client.publish(tracking_topic_benchmark_data, payload, len);
  }
}

void publishTrackingBenchmarkSummary(TrackingBenchmarkApproach approach, const TrackingPerfStats& stats) {
  StaticJsonDocument<384> doc;
  doc["approach"] = trackingBenchmarkApproachToStr(approach);
  doc["n_target"] = stats.targetN;
  doc["processed"] = stats.processed;
  doc["latency_min_us"] = stats.minLatencyUs;
  doc["latency_max_us"] = stats.maxLatencyUs;
  doc["latency_avg_us"] = stats.processed ? (stats.sumLatencyUs / stats.processed) : 0;
  doc["heap_min"] = stats.minHeap;
  doc["heap_max"] = stats.maxHeap;
  doc["dropped"] = stats.dropped;

  char payload[384];
  size_t len = serializeJson(doc, payload);
  client.publish(tracking_topic_benchmark_summary, payload, len);

  Serial.printf(
      "[TRACKING BENCH] approach=%s N=%lu processed=%lu min=%luus avg=%lluus max=%luus heapMin=%lu heapMax=%lu dropped=%lu\n",
      trackingBenchmarkApproachToStr(approach),
      static_cast<unsigned long>(stats.targetN),
      static_cast<unsigned long>(stats.processed),
      static_cast<unsigned long>(stats.minLatencyUs),
      static_cast<unsigned long long>(stats.processed ? (stats.sumLatencyUs / stats.processed) : 0),
      static_cast<unsigned long>(stats.maxLatencyUs),
      static_cast<unsigned long>(stats.minHeap),
      static_cast<unsigned long>(stats.maxHeap),
      static_cast<unsigned long>(stats.dropped));
}

void updateTrackingBenchmarkStats(TrackingPerfStats& stats, uint32_t latencyUs) {
  const uint32_t heap = ESP.getFreeHeap();
  if (latencyUs < stats.minLatencyUs) stats.minLatencyUs = latencyUs;
  if (latencyUs > stats.maxLatencyUs) stats.maxLatencyUs = latencyUs;
  stats.sumLatencyUs += latencyUs;
  if (heap < stats.minHeap) stats.minHeap = heap;
  if (heap > stats.maxHeap) stats.maxHeap = heap;
  ++stats.processed;
  Serial.printf("Tracking bench | Latencia: %lu us | Heap Livre: %u bytes\n", latencyUs, heap);
}

TrackingSample sampleFromSnapshot(const TrackingSample* snapshot, size_t snapshotCount, uint32_t index) {
  if (snapshotCount == 0) {
    return buildTrackingSample(WiFi.macAddress(), WiFi.RSSI(), "esp32-central");
  }

  TrackingSample sample = snapshot[index % snapshotCount];
  sample.timestampUs = micros();
  return sample;
}

void drainTrackingRingBuffer(uint32_t targetN, uint32_t& batchIndex) {
  TrackingSample batch[TRACKING_BENCHMARK_BATCH_SIZE];
  while (trackingRingBuffer.size() > 0) {
    size_t consumed = 0;
    while (consumed < TRACKING_BENCHMARK_BATCH_SIZE) {
      TrackingSample sample;
      if (!trackingRingBuffer.pop(sample)) {
        break;
      }
      batch[consumed++] = sample;
    }

    if (consumed == 0) {
      break;
    }

    publishTrackingBenchmarkData(TrackingBenchmarkApproach::Circular, targetN, batch, consumed, batchIndex++);
    client.loop();
    delay(TRACKING_BENCHMARK_NETWORK_DELAY_MS);
  }
}

void runTrackingInefficientBenchmark(const TrackingSample* snapshot, size_t snapshotCount, uint32_t targetN) {
  TrackingPerfStats stats;
  stats.targetN = targetN;
  trackingInefficientWindow.clear();

  uint32_t batchIndex = 0;
  TrackingSample batch[TRACKING_BENCHMARK_BATCH_SIZE];
  size_t batchCount = 0;

  for (uint32_t i = 0; i < targetN; ++i) {
    TrackingSample sample = sampleFromSnapshot(snapshot, snapshotCount, i);
    unsigned long start = micros();
    bool ok = trackingInefficientWindow.push(sample);
    unsigned long duration = micros() - start;

    if (!ok) {
      ++stats.dropped;
      continue;
    }

    updateTrackingBenchmarkStats(stats, duration);
    if ((i % 100) == 0) {
      publishTrackingBenchmarkPerf(TrackingBenchmarkApproach::Inefficient, stats, duration, i);
    }

    // Collect into batch
    batch[batchCount++] = sample;
    if (batchCount == TRACKING_BENCHMARK_BATCH_SIZE) {
      publishTrackingBenchmarkData(TrackingBenchmarkApproach::Inefficient, targetN, batch, batchCount, batchIndex++);
      batchCount = 0;
      client.loop();
      delay(TRACKING_BENCHMARK_NETWORK_DELAY_MS);
    }
    
    if ((i % 50) == 0) client.loop();
  }

  // Send remaining
  if (batchCount > 0) {
    publishTrackingBenchmarkData(TrackingBenchmarkApproach::Inefficient, targetN, batch, batchCount, batchIndex++);
  }

  publishTrackingBenchmarkSummary(TrackingBenchmarkApproach::Inefficient, stats);
}

void runTrackingCircularBenchmark(const TrackingSample* snapshot, size_t snapshotCount, uint32_t targetN) {
  TrackingPerfStats stats;
  stats.targetN = targetN;
  trackingRingBuffer.clear();

  uint32_t batchIndex = 0;
  for (uint32_t i = 0; i < targetN; ++i) {
    TrackingSample sample = sampleFromSnapshot(snapshot, snapshotCount, i);
    unsigned long start = micros();
    trackingRingBuffer.push(sample);
    unsigned long duration = micros() - start;

    updateTrackingBenchmarkStats(stats, duration);
    if ((i % 100) == 0) {
      publishTrackingBenchmarkPerf(TrackingBenchmarkApproach::Circular, stats, duration, i);
    }

    if (((i + 1) % TRACKING_BENCHMARK_BATCH_SIZE) == 0) {
      drainTrackingRingBuffer(targetN, batchIndex);
    }
    client.loop();
  }

  drainTrackingRingBuffer(targetN, batchIndex);
  stats.dropped = trackingRingBuffer.dropped();
  publishTrackingBenchmarkSummary(TrackingBenchmarkApproach::Circular, stats);
}

void runTrackingBenchmark(const TrackingSample* snapshot, size_t snapshotCount) {
  Serial.printf("Iniciando benchmark de rastreamento com %u amostras visiveis\n", static_cast<unsigned>(snapshotCount));
  for (size_t i = 0; i < kTrackingBenchmarkScaleCount; ++i) {
    const uint32_t targetN = kTrackingBenchmarkScales[i];
    Serial.printf("\n=== Tracking Benchmark Ineficiente | N=%lu ===\n", static_cast<unsigned long>(targetN));
    runTrackingInefficientBenchmark(snapshot, snapshotCount, targetN);

    Serial.printf("\n=== Tracking Benchmark Circular | N=%lu ===\n", static_cast<unsigned long>(targetN));
    runTrackingCircularBenchmark(snapshot, snapshotCount, targetN);
  }
  Serial.println("Benchmark de rastreamento concluido.");
}

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

  // Tenta conectar com as credenciais padrão primeiro
  WiFi.begin("uaifai-brum", "bemvindoaocesar");

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

  if (!isScanning) {
    Serial.println("Iniciando varredura WiFi assíncrona...");
    WiFi.scanNetworks(true, false, false, 100);
    isScanning = true;
    return;
  }

  int n = WiFi.scanComplete();
  if (n == WIFI_SCAN_FAILED) {
    isScanning = false;
    lastScanTime = now;
    return;
  }
  
  if (n < 0) return; // Scanning still in progress

  Serial.printf("Varredura concluída: %d redes encontradas\n", n);
  TrackingSample benchmarkSnapshot[TRACKING_BENCHMARK_SNAPSHOT_LIMIT];
  size_t benchmarkSnapshotCount = 0;
  
  if (n > 0) {
    for (int i = 0; i < n; ++i) {
      String bssid = WiFi.BSSIDstr(i);
      int rssi = WiFi.RSSI(i);

      if (benchmarkSnapshotCount < TRACKING_BENCHMARK_SNAPSHOT_LIMIT) {
        benchmarkSnapshot[benchmarkSnapshotCount++] = buildTrackingSample(bssid, rssi, "esp32-central");
      }
      
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
  isScanning = false;
  lastScanTime = now;

#if TRACKING_BENCHMARK_ENABLED
  if (!trackingBenchmarkRan) {
    runTrackingBenchmark(benchmarkSnapshot, benchmarkSnapshotCount);
    trackingBenchmarkRan = true;
  }
#endif
}

void updateLedFeedback() {
  if (!ledFeedbackActive) return;
  
  unsigned long now = millis();
  if (ledFeedbackType) { // Green constant
    if (now - ledFeedbackStart > 2000) {
      digitalWrite(LED_GREEN_PIN, LOW);
      ledFeedbackActive = false;
    }
  } else { // Red blinking
    unsigned long elapsed = now - ledFeedbackStart;
    if (elapsed > 1200) {
      digitalWrite(LED_RED_PIN, LOW);
      ledFeedbackActive = false;
    } else {
      // Toggle every 200ms
      digitalWrite(LED_RED_PIN, (elapsed / 200) % 2 == 0);
    }
  }
}

void mqtt_callback(char* topic, byte* payload, unsigned int length) {
  String message = "";
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  StaticJsonDocument<200> doc;
  if (deserializeJson(doc, message) == DeserializationError::Ok) {
    bool authorized = doc["authorized"];
    ledFeedbackActive = true;
    ledFeedbackStart = millis();
    ledFeedbackType = authorized;
    
    if (authorized) {
      Serial.println("Acesso GARANTIDO");
      digitalWrite(LED_GREEN_PIN, HIGH);
    } else {
      Serial.println("Acesso NEGADO");
      digitalWrite(LED_RED_PIN, HIGH);
    }
  }
}

unsigned long lastReconnectAttempt = 0;

bool reconnect() {
  if (client.connected()) return true;
  
  unsigned long now = millis();
  if (now - lastReconnectAttempt > 5000) {
    lastReconnectAttempt = now;
    Serial.print("Tentando conexão MQTT...");
    String clientId = "ESP32Scanner-";
    clientId += String(random(0xffff), HEX);
    
    if (client.connect(clientId.c_str())) {
      Serial.println("conectado");
      return true;
    } else {
      Serial.print("falhou, rc=");
      Serial.println(client.state());
    }
  }
  return false;
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

void mqttTask(void *pvParameters) {
  for (;;) {
    if (WiFi.status() == WL_CONNECTED) {
      if (!client.connected()) {
        reconnect();
      }
      client.loop();
      scan_tracking_tags();
      
      RfidScan scan;
      if (xQueueReceive(rfidQueue, &scan, 0) == pdTRUE) {
        Serial.printf("Enviando UID %s para o broker...\n", scan.uid);
        StaticJsonDocument<200> doc;
        doc["uid"] = scan.uid;
        char buffer[256];
        serializeJson(doc, buffer);
        client.publish("rfid/scans", buffer);
        
        String responseTopic = String("rfid/responses/") + scan.uid;
        client.subscribe(responseTopic.c_str());
      }
    }
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

void rfidTask(void *pvParameters) {
  for (;;) {
    updateLedFeedback();
    
    if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
      Serial.print("Cartão detectado! UID: ");
      RfidScan scan;
      String uidStr = uidToString(rfid.uid);
      Serial.println(uidStr);
      strncpy(scan.uid, uidStr.c_str(), sizeof(scan.uid));
      
      xQueueSend(rfidQueue, &scan, portMAX_DELAY);
      
      rfid.PICC_HaltA();
      rfid.PCD_StopCrypto1();
    }
    vTaskDelay(pdMS_TO_TICKS(50));
  }
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
  client.setBufferSize(2048); // Increase buffer for large benchmark payloads

  SPI.begin(18, 19, 23, 5); // SCK, MISO, MOSI, SS
  rfid.PCD_Init();
  
  rfidQueue = xQueueCreate(10, sizeof(RfidScan));
  
  xTaskCreatePinnedToCore(mqttTask, "MQTTTask", 8192, NULL, 1, &mqttTaskHandle, 0);
  xTaskCreatePinnedToCore(rfidTask, "RFIDTask", 4096, NULL, 2, &rfidTaskHandle, 1);

  Serial.println("Scanner Pronto");
}

void loop() {
  vTaskDelete(NULL); 
}
