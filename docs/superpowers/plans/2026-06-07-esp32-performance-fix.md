# ESP32 Non-Blocking Performance & RFID Reliability Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Eliminate ESP32 "stuck" behavior and improve RFID responsiveness by moving blocking operations (WiFi scan, MQTT reconnection) to separate FreeRTOS tasks and removing synchronous delays.

**Architecture:** Use FreeRTOS tasks to isolate the time-sensitive RFID reader from blocking network operations. Implement a non-blocking MQTT state machine and replace `delay()` with `millis()`-based logic.

**Tech Stack:** ESP32 (Arduino core), FreeRTOS, PubSubClient, MFRC522.

---

### Task 1: Refactor Reconnection & LED Feedback to be Non-Blocking

**Files:**
- Modify: `src/main.cpp`

- [ ] **Step 1: Replace blocking `reconnect()` with non-blocking logic**

Remove the `while(!client.connected())` loop. Instead, check the connection status and use a timer to avoid spamming reconnection attempts.

```cpp
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
      // Re-subscribe to necessary topics here if needed
      return true;
    } else {
      Serial.print("falhou, rc=");
      Serial.println(client.state());
    }
  }
  return false;
}
```

- [ ] **Step 2: Refactor `mqtt_callback` to remove `delay()`**

Introduce a state machine for LED feedback to avoid blocking the CPU for 2 seconds during authorization.

```cpp
// Global states for LED feedback
bool ledFeedbackActive = false;
unsigned long ledFeedbackStart = 0;
bool ledFeedbackType = false; // true for Green, false for Red blink

void mqtt_callback(char* topic, byte* payload, unsigned int length) {
  StaticJsonDocument<200> doc;
  if (deserializeJson(doc, payload, length) == DeserializationError::Ok) {
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
```

### Task 2: Implement FreeRTOS Tasks for Concurrency

**Files:**
- Modify: `src/main.cpp`

- [ ] **Step 1: Define Task Handles and Queues**

```cpp
TaskHandle_t mqttTaskHandle = NULL;
TaskHandle_t rfidTaskHandle = NULL;
QueueHandle_t rfidQueue = NULL;

struct RfidScan {
  char uid[32];
};
```

- [ ] **Step 2: Create the MQTT/Network Task**

This task will handle WiFi scanning, MQTT loops, and publishing.

```cpp
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
```

- [ ] **Step 3: Create the RFID Task**

This task will focus solely on the MFRC522 reader.

```cpp
void rfidTask(void *pvParameters) {
  for (;;) {
    updateLedFeedback();
    
    if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
      RfidScan scan;
      String uidStr = uidToString(rfid.uid);
      strncpy(scan.uid, uidStr.c_str(), sizeof(scan.uid));
      
      xQueueSend(rfidQueue, &scan, portMAX_DELAY);
      
      rfid.PICC_HaltA();
      rfid.PCD_StopCrypto1();
    }
    vTaskDelay(pdMS_TO_TICKS(50));
  }
}
```

- [ ] **Step 4: Update `setup()` to initialize tasks**

```cpp
void setup() {
  // ... existing initialization ...
  
  rfidQueue = xQueueCreate(10, sizeof(RfidScan));
  
  xTaskCreatePinnedToCore(mqttTask, "MQTTTask", 8192, NULL, 1, &mqttTaskHandle, 0);
  xTaskCreatePinnedToCore(rfidTask, "RFIDTask", 4096, NULL, 2, &rfidTaskHandle, 1);
}
```

- [ ] **Step 5: Clean up `loop()`**

The `loop()` function should be empty or just handle watchdog resets if necessary, as tasks handle everything.

```cpp
void loop() {
  // Empty as we use FreeRTOS tasks
  vTaskDelete(NULL); 
}
```

### Task 3: Optimize WiFi Scanning

**Files:**
- Modify: `src/main.cpp`

- [ ] **Step 1: Use Asynchronous WiFi Scan**

Instead of blocking for 1.4 seconds, trigger the scan and check for results later.

```cpp
bool isScanning = false;

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
  
  if (n >= 0) {
    // Process results as before ...
    for (int i = 0; i < n; ++i) {
        // ... publish to MQTT ...
    }
    WiFi.scanDelete();
    isScanning = false;
    lastScanTime = now;
  }
}
```

### Task 4: Verification and Logging Improvements

**Files:**
- Modify: `src/main.cpp`

- [ ] **Step 1: Add serial logs for task performance**

Monitor heap and task execution to ensure stability.

- [ ] **Step 2: Test RFID responsiveness during WiFi scan**

Verify that a card scan is detected immediately even if `scan_tracking_tags` is in the middle of processing results.

---
