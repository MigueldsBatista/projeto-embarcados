# RFID Dashboard - Implementation Master Plan

This document serves as the single source of truth for the RFID Dashboard project. It tracks the implementation status of the backend, frontend, and infrastructure.

## 🎯 Project Goal
A professional web dashboard to manage RFID access control. ESP32 devices scan cards, send UIDs via MQTT to a FastAPI backend, which validates access and logs events. A Vue 3 dashboard provides real-time monitoring and card management.

---

## ✅ Completed Foundations
- [x] **Backend Foundation**: FastAPI + SQLAlchemy (SQLite) setup.
- [x] **Database Schema**: `Card` and `AccessLog` models.
- [x] **MQTT Integration**: Backend listener for `rfid/scans` and responder on `rfid/responses/{uid}`.
- [x] **Frontend Architecture**: Vue 3 + Bun + Tailwind CSS + TanStack Query + Axios.
- [x] **Design Patterns**: Composables for logic, dumb components for UI, < 200 lines per file.
- [x] **Core Dashboard**: Real-time (polling) stats and logs table.
- [x] **Embedded System**: ESP32 refactored to use WiFi/MQTT instead of hardcoded arrays.

---

## 🛠️ Remaining Roadmap

### Phase 1: Card Management & CRUD
**Goal:** Allow users to register, edit, and deactivate RFID cards from the web interface.

- [ ] **Frontend: Card Management View**
  - [ ] Create `CardsView.vue` container.
  - [ ] Create `CardList.vue` (dumb component) to display cards.
  - [ ] Create `AddCardModal.vue` for new card registration.
  - [ ] Implement `useCards` composable for CRUD operations.
- [ ] **Backend: Enhanced Card Router**
  - [ ] Add `PUT /api/cards/{uid}` for updates.
  - [ ] Add `DELETE /api/cards/{uid}` for removals.
  - [ ] Implement validation to prevent duplicate UIDs.

### Phase 2: Security & Authentication
**Goal:** Secure the dashboard and MQTT communication.

- [ ] **Dashboard Auth**
  - [ ] Implement JWT-based login for the FastAPI backend.
  - [ ] Add `LoginView.vue` and protected routes in Vue Router.
- [ ] **MQTT Security**
  - [ ] Configure Mosquitto with username/password authentication.
  - [ ] Update ESP32 and Backend MQTT Client to use credentials.
  - [ ] (Optional) Transition to AWS IoT Core for production.

### Phase 3: Real-time UX & Notifications
**Goal:** Improve the "live" feel of the dashboard.

- [ ] **WebSocket Integration**
  - [ ] Implement FastAPI WebSockets to push logs to the frontend immediately.
  - [ ] Update `useDashboard` to listen for WebSocket events instead of polling.
- [ ] **UI Polish**
  - [ ] Add "Toast" notifications for successful card scans or errors.
  - [ ] Implement responsive design for mobile monitoring.

### Phase 4: Deployment & Infrastructure
**Goal:** Easy deployment and persistent data.

- [ ] **Dockerization**
  - [ ] Create `Dockerfile` for Backend and Frontend.
  - [ ] Create `docker-compose.yml` including:
    - FastAPI app
    - Vite (built/nginx)
    - Mosquitto Broker
- [ ] **Persistence**
  - [ ] Ensure SQLite volume persistence in Docker.

---

## 📐 Source of Truth: Tech Stack & Conventions

| Layer | Technology | Key Patterns |
| :--- | :--- | :--- |
| **Frontend** | Vue 3, TanStack Query, Axios, Lucide | Composables, Container/Presenter |
| **Backend** | FastAPI, SQLAlchemy, Paho-MQTT | Dependency Injection, Modular Routers |
| **Embedded** | C++, PlatformIO, ArduinoJson | MQTT Pub/Sub |
| **DB** | SQLite (dev) / Postgres (prod) | Repository Pattern via SQLAlchemy |
| **Broker** | Mosquitto | Topic-based communication |
