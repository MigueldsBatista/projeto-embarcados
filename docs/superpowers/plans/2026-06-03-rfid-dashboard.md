# RFID Web Dashboard Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Replace the hardcoded array of RFID cards on the ESP32 with a robust web dashboard and backend to register cards, log entries, and view metrics using a modern interface.

**Architecture:** ESP32 reads cards and publishes the UID to an MQTT broker. A FastAPI backend listens to MQTT, validates the card against an SQLite database, logs the scan, and replies via MQTT so the ESP32 can light the appropriate LED. The FastAPI backend also serves a REST API for a Vue.js frontend dashboard. The architecture uses SQLAlchemy to ensure it can easily transition to AWS RDS (Postgres) and AWS IoT Core later.

**Tech Stack:** 
- Embedded: C++ / PlatformIO
- Backend: Python / FastAPI, SQLAlchemy, Paho-MQTT
- Frontend: Vue 3 (Composition API), TailwindCSS
- Infrastructure: Mosquitto (MQTT Broker), SQLite (Database)

---

### Task 1: Setup Backend Foundation (FastAPI & Database)

**Files:**
- Create: `backend/requirements.txt`
- Create: `backend/main.py`
- Create: `backend/database.py`
- Create: `backend/models.py`

- [ ] **Step 1: Install Python dependencies**
```bash
mkdir backend
cd backend
python -m venv venv
source venv/bin/activate
pip install fastapi "uvicorn[standard]" sqlalchemy paho-mqtt
pip freeze > requirements.txt
```

- [ ] **Step 2: Create Database Config (`backend/database.py`)**
```python
from sqlalchemy import create_engine
from sqlalchemy.orm import sessionmaker, declarative_base

SQLALCHEMY_DATABASE_URL = "sqlite:///./rfid.db"
engine = create_engine(SQLALCHEMY_DATABASE_URL, connect_args={"check_same_thread": False})
SessionLocal = sessionmaker(autocommit=False, autoflush=False, bind=engine)
Base = declarative_base()

def get_db():
    db = SessionLocal()
    try:
        yield db
    finally:
        db.close()
```

- [ ] **Step 3: Define SQLAlchemy Models (`backend/models.py`)**
```python
from sqlalchemy import Column, Integer, String, Boolean, DateTime, ForeignKey
from sqlalchemy.sql import func
from .database import Base

class Card(Base):
    __tablename__ = "cards"
    id = Column(Integer, primary_key=True, index=True)
    uid = Column(String, unique=True, index=True)
    name = Column(String)
    role = Column(String)
    is_active = Column(Boolean, default=True)
    created_at = Column(DateTime(timezone=True), server_default=func.now())

class AccessLog(Base):
    __tablename__ = "access_logs"
    id = Column(Integer, primary_key=True, index=True)
    uid = Column(String, index=True)
    status = Column(String) # "authorized", "unknown", "inactive"
    timestamp = Column(DateTime(timezone=True), server_default=func.now())
```

- [ ] **Step 4: Create FastAPI Entrypoint (`backend/main.py`)**
```python
from fastapi import FastAPI
from fastapi.middleware.cors import CORSMiddleware
from .database import engine, Base
import backend.models as models

Base.metadata.create_all(bind=engine)

app = FastAPI(title="RFID Dashboard API")

app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

@app.get("/api/health")
def health_check():
    return {"status": "ok"}
```

### Task 2: Setup MQTT on Backend

**Files:**
- Create: `backend/mqtt_client.py`
- Modify: `backend/main.py:17-20`

- [ ] **Step 1: Create MQTT handler (`backend/mqtt_client.py`)**
```python
import paho.mqtt.client as mqtt
import json
from .database import SessionLocal
from .models import Card, AccessLog

MQTT_BROKER = "localhost" # Assume mosquitto is running locally for now
MQTT_PORT = 1883

def on_connect(client, userdata, flags, rc):
    print(f"Connected to MQTT broker with result code {rc}")
    client.subscribe("rfid/scans")

def on_message(client, userdata, msg):
    try:
        payload = json.loads(msg.payload.decode())
        uid = payload.get("uid")
        if not uid:
            return
        
        db = SessionLocal()
        card = db.query(Card).filter(Card.uid == uid).first()
        
        status = "unknown"
        authorized = False
        name = "Unknown"
        
        if card:
            if card.is_active:
                status = "authorized"
                authorized = True
                name = card.name
            else:
                status = "inactive"
        
        # Log the access
        log_entry = AccessLog(uid=uid, status=status)
        db.add(log_entry)
        db.commit()
        db.close()
        
        # Publish response
        response = {"authorized": authorized, "name": name}
        client.publish(f"rfid/responses/{uid}", json.dumps(response))
        print(f"Processed scan for {uid}: {status}")
        
    except Exception as e:
        print(f"Error processing MQTT message: {e}")

def start_mqtt():
    client = mqtt.Client()
    client.on_connect = on_connect
    client.on_message = on_message
    client.connect(MQTT_BROKER, MQTT_PORT, 60)
    client.loop_start()
```

- [ ] **Step 2: Start MQTT client in FastAPI lifecycle (`backend/main.py`)**
```python
# Add this near the top
from contextlib import asynccontextmanager
from .mqtt_client import start_mqtt

@asynccontextmanager
async def lifespan(app: FastAPI):
    # Startup
    start_mqtt()
    yield
    # Shutdown (can add cleanup here later)

# Update FastAPI init
app = FastAPI(title="RFID Dashboard API", lifespan=lifespan)
```

### Task 3: Backend REST API for Cards and Logs

**Files:**
- Create: `backend/routers/cards.py`
- Create: `backend/routers/metrics.py`
- Modify: `backend/main.py:25-30`

- [ ] **Step 1: Create Cards router (`backend/routers/cards.py`)**
```python
from fastapi import APIRouter, Depends
from sqlalchemy.orm import Session
from ..database import get_db
from ..models import Card, AccessLog

router = APIRouter(prefix="/api/cards", tags=["cards"])

@router.get("/")
def get_cards(db: Session = Depends(get_db)):
    return db.query(Card).all()

@router.post("/")
def add_card(uid: str, name: str, role: str, db: Session = Depends(get_db)):
    card = Card(uid=uid, name=name, role=role)
    db.add(card)
    db.commit()
    db.refresh(card)
    return card
```

- [ ] **Step 2: Create Metrics router (`backend/routers/metrics.py`)**
```python
from fastapi import APIRouter, Depends
from sqlalchemy.orm import Session
from ..database import get_db
from ..models import AccessLog

router = APIRouter(prefix="/api/metrics", tags=["metrics"])

@router.get("/logs")
def get_logs(limit: int = 50, db: Session = Depends(get_db)):
    return db.query(AccessLog).order_by(AccessLog.timestamp.desc()).limit(limit).all()

@router.get("/summary")
def get_summary(db: Session = Depends(get_db)):
    total = db.query(AccessLog).count()
    unknowns = db.query(AccessLog).filter(AccessLog.status == "unknown").count()
    return {"total_scans": total, "unknown_scans": unknowns}
```

- [ ] **Step 3: Include routers in `backend/main.py`**
```python
from .routers import cards, metrics

app.include_router(cards.router)
app.include_router(metrics.router)
```

### Task 4: Setup Vue Frontend

**Files:**
- Create: `frontend/` (via npm/vite)

- [ ] **Step 1: Scaffold Vue project**
```bash
npm create vite@latest frontend -- --template vue
cd frontend
npm install
npm install tailwindcss postcss autoprefixer axios
npx tailwindcss init -p
```

- [ ] **Step 2: Configure Tailwind (`frontend/tailwind.config.js`)**
```javascript
/** @type {import('tailwindcss').Config} */
export default {
  content: [
    "./index.html",
    "./src/**/*.{vue,js,ts,jsx,tsx}",
  ],
  theme: {
    extend: {},
  },
  plugins: [],
}
```

- [ ] **Step 3: Include Tailwind CSS (`frontend/src/style.css`)**
```css
@tailwind base;
@tailwind components;
@tailwind utilities;
```

### Task 5: Build Dashboard UI (Frontend)

**Files:**
- Modify: `frontend/src/App.vue`
- Create: `frontend/src/components/Dashboard.vue`

- [ ] **Step 1: Create Dashboard Component (`frontend/src/components/Dashboard.vue`)**
```vue
<template>
  <div class="p-8 max-w-6xl mx-auto">
    <h1 class="text-3xl font-bold mb-6">RFID Access Dashboard</h1>
    
    <div class="grid grid-cols-2 gap-4 mb-8">
      <div class="bg-blue-100 p-6 rounded shadow">
        <h2 class="text-xl font-semibold">Total Scans</h2>
        <p class="text-4xl mt-2">{{ summary.total_scans || 0 }}</p>
      </div>
      <div class="bg-red-100 p-6 rounded shadow">
        <h2 class="text-xl font-semibold">Unknown Attempts</h2>
        <p class="text-4xl mt-2">{{ summary.unknown_scans || 0 }}</p>
      </div>
    </div>

    <div class="mb-8">
      <h2 class="text-2xl font-bold mb-4">Recent Logs</h2>
      <table class="w-full text-left bg-white shadow rounded">
        <thead>
          <tr class="bg-gray-200">
            <th class="p-3">Time</th>
            <th class="p-3">UID</th>
            <th class="p-3">Status</th>
          </tr>
        </thead>
        <tbody>
          <tr v-for="log in logs" :key="log.id" class="border-t">
            <td class="p-3">{{ new Date(log.timestamp).toLocaleString() }}</td>
            <td class="p-3 font-mono">{{ log.uid }}</td>
            <td class="p-3">
              <span :class="{'text-green-600': log.status==='authorized', 'text-red-600': log.status==='unknown'}">
                {{ log.status }}
              </span>
            </td>
          </tr>
        </tbody>
      </table>
    </div>
  </div>
</template>

<script setup>
import { ref, onMounted } from 'vue'
import axios from 'axios'

const summary = ref({})
const logs = ref([])

const API_URL = 'http://localhost:8000/api'

const fetchData = async () => {
  try {
    const sumRes = await axios.get(`${API_URL}/metrics/summary`)
    summary.value = sumRes.data
    
    const logsRes = await axios.get(`${API_URL}/metrics/logs`)
    logs.value = logsRes.data
  } catch (error) {
    console.error("API fetch failed", error)
  }
}

onMounted(() => {
  fetchData()
  // Auto refresh every 5s
  setInterval(fetchData, 5000)
})
</script>
```

- [ ] **Step 2: Replace App.vue (`frontend/src/App.vue`)**
```vue
<template>
  <div class="min-h-screen bg-gray-50 text-gray-900 font-sans">
    <Dashboard />
  </div>
</template>

<script setup>
import Dashboard from './components/Dashboard.vue'
</script>
```

### Task 6: Refactor ESP32 C++ Code for MQTT

**Files:**
- Modify: `platformio.ini`
- Modify: `src/main.cpp`

- [ ] **Step 1: Add MQTT Library (`platformio.ini`)**
```ini
; ... existing content ...
lib_deps =
  miguelbalboa/MFRC522@^1.4.12
  knolleary/PubSubClient@^2.8
  bblanchon/ArduinoJson@^6.21.3
```

- [ ] **Step 2: Update `src/main.cpp`**
Replace the hardcoded array with Wi-Fi and MQTT logic.
```cpp
#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// --- Configs ---
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASS";
const char* mqtt_server = "192.168.x.x"; // Replace with PC IP
// ---------------

#define RC522_SS_PIN 5
#define RC522_RST_PIN 22
#define LED_RED_PIN 4
#define LED_GREEN_PIN 2

MFRC522 rfid(RC522_SS_PIN, RC522_RST_PIN);
WiFiClient espClient;
PubSubClient client(espClient);

String currentUid = "";

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
  Serial.println("\nWiFi connected");
}

void mqtt_callback(char* topic, byte* payload, unsigned int length) {
  String message = "";
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  
  StaticJsonDocument<200> doc;
  DeserializationError error = deserializeJson(doc, message);
  if (error) return;
  
  bool authorized = doc["authorized"];
  if (authorized) {
    digitalWrite(LED_GREEN_PIN, HIGH);
    digitalWrite(LED_RED_PIN, LOW);
    delay(1000);
    digitalWrite(LED_GREEN_PIN, LOW);
  } else {
    digitalWrite(LED_RED_PIN, HIGH);
    delay(1000);
    digitalWrite(LED_RED_PIN, LOW);
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP32_RFID_Client")) {
      Serial.println("connected");
      // We will subscribe dynamically per UID before publishing
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
  
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(mqtt_callback);

  SPI.begin();
  rfid.PCD_Init();
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

  currentUid = uidToString(rfid.uid);
  Serial.println("Scanned: " + currentUid);
  
  // Subscribe to the specific response topic
  String topic = "rfid/responses/" + currentUid;
  client.subscribe(topic.c_str());
  
  // Publish scan event
  StaticJsonDocument<200> doc;
  doc["uid"] = currentUid;
  String payload;
  serializeJson(doc, payload);
  client.publish("rfid/scans", payload.c_str());

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
  delay(1000); // Prevent spamming
}
```
