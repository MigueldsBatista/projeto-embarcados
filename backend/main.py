from contextlib import asynccontextmanager
from fastapi import FastAPI
from fastapi.middleware.cors import CORSMiddleware
from .database import engine
from . import models
from .mqtt_client import mqtt_client
from .routers import cards, metrics, tracking

# Create database tables
models.Base.metadata.create_all(bind=engine)

@asynccontextmanager
async def lifespan(app: FastAPI):
    # Startup: Start MQTT client
    mqtt_client.start()
    yield
    # Shutdown: Stop MQTT client
    mqtt_client.stop()

app = FastAPI(title="RFID Dashboard API", lifespan=lifespan)

app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

app.include_router(cards.router)
app.include_router(metrics.router)
app.include_router(tracking.router)

@app.get("/api/health")
def health_check():
    return {"status": "ok"}
