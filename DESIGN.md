# SafeKid - Design Document

## Architecture Overview
The project follows a decoupled architecture consisting of an ESP32 embedded system, a FastAPI backend, and a Vue.js frontend.

### Backend
- **Framework**: FastAPI
- **Database**: SQLite with SQLAlchemy ORM
- **Communication**: 
  - **MQTT**: Used for real-time communication with the ESP32.
  - **REST API**: Used by the frontend for data management and metrics.
- **Patterns**: Router-based modularization, Dependency Injection for DB sessions.

### Frontend
- **Framework**: Vue 3 (Composition API)
- **State Management**: TanStack Vue Query (Server State)
- **HTTP Client**: Axios
- **Icons**: Lucide Vue
- **Styling**: Tailwind CSS
- **Design Patterns**:
  - **Atomic/Modular Components**: Components are kept small (< 200 lines).
  - **Composables**: Business logic and data fetching are encapsulated in composables.
  - **Container/Presenter Pattern**: 
    - **Views/Containers**: Handle data fetching via composables and pass data to dumb components.
    - **Dumb Components**: Focus on UI/UX and emit events; they do not hold business logic.
  - **API Layer**: Centralized Axios instance for interceptors and configuration.

## Frontend Directory Structure
```
frontend/src/
├── api/          # Axios instance and service definitions
├── components/   # Dumb components (UI only)
├── composables/  # Business logic and query hooks
├── router/       # Vue Router configuration
├── types/        # TypeScript interfaces/types
├── views/        # Page components (Containers)
├── App.vue       # Main application component
└── main.ts       # Application entry point
```

## Security Considerations
- RFID UIDs are validated against the database.
- ESP32 communication is handled via a private MQTT broker (Mosquitto).
- Future versions will implement AWS IoT Core for secure device connection.
