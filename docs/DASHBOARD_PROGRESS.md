# SafeKid Project: Plan & Progress

## Project Overview
The goal is to transition from a hardcoded RFID registry on the ESP32 to a robust, scalable web dashboard. The system tracks card entries with timestamps, allows for management of registered cards, and provides real-time metrics.

## High-Level Architecture
- **Embedded**: ESP32 + MFRC522 RFID Reader.
- **Communication**: MQTT (Mosquitto) for real-time scan events and authorization responses.
- **Backend**: FastAPI (Python) + SQLAlchemy + SQLite (AWS RDS/Postgres ready).
- **Frontend**: Vue 3 (Composition API) + Tailwind CSS.
- **Deployment Strategy**: Local notebook initially, then AWS (EC2/Amplify/IoT Core).

## Key Decisions
- **MQTT Over HTTP**: Chosen for real-time responsiveness and scalability with multiple readers.
- **FastAPI + Vue**: Modern, fast, and highly flexible stack.
- **Relational DB**: SQLite for development, easily portable to PostgreSQL for production.

---

## Implementation Plan

### 1. Backend Foundation (COMPLETED)
- [x] Create project structure.
- [x] Configure SQLAlchemy and SQLite database.
- [x] Define `Card` and `AccessLog` models.
- [x] Initialize FastAPI with CORS and health check.

### 2. MQTT Backend Integration (COMPLETED)
- [x] Implement MQTT client in `backend/mqtt_client.py`.
- [x] Logic for processing scans, checking database, and logging entries.
- [x] Logic for publishing authorization responses to `rfid/responses/<uid>`.
- [x] Integration into FastAPI lifecycle (lifespan).

### 3. REST API for Management (COMPLETED)
- [x] `GET /api/cards/`: List all registered cards.
- [x] `POST /api/cards/`: Register a new card.
- [x] `GET /api/metrics/logs`: Retrieve recent access logs.
- [x] `GET /api/metrics/summary`: Retrieve scan statistics.

### 4. Vue Frontend Scaffolding (PENDING)
- [ ] Initialize Vite/Vue 3 project.
- [ ] Install and configure Tailwind CSS.
- [ ] Configure Axios for API communication.

### 5. Dashboard UI Development (PENDING)
- [ ] Build `Dashboard.vue` with metrics cards and real-time logs table.
- [ ] Implement auto-refresh mechanism (long-polling or WebSockets later).
- [ ] Layout polish with Tailwind.

### 6. ESP32 Firmware Refactor (PENDING)
- [ ] Integrate `PubSubClient` and `ArduinoJson` libraries.
- [ ] Implement Wi-Fi and MQTT connection logic.
- [ ] Implement RFID scan publication and authorization response subscription.
- [ ] Update LED feedback logic based on MQTT response.

---

## Technical Progress Details

### Backend Files Created:
- `backend/database.py`: DB connection and SessionLocal.
- `backend/models.py`: SQLAlchemy models for Cards and Logs.
- `backend/main.py`: Main FastAPI entrypoint with router integration.
- `backend/mqtt_client.py`: MQTT background listener and processor.
- `backend/routers/cards.py`: Card management endpoints.
- `backend/routers/metrics.py`: Metrics and logs endpoints.

### Verification Status:
- **API**: Verified via `TestClient`. `GET /api/health` and `GET /api/cards/` are functional.
- **Database**: Verified manually. Models persist and query correctly.
- **MQTT Logic**: Implemented and integrated; requires a running Mosquitto broker for full E2E testing.

---
**Last Updated**: 2026-06-03
