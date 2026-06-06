from contextlib import asynccontextmanager
from fastapi import FastAPI
from fastapi.middleware.cors import CORSMiddleware
from .database import engine
from . import models
from .mqtt_client import mqtt_client
from .routers import cards, metrics, tracking

# ... (tables metadata)

# ... (lifespan)

app = FastAPI(title="RFID Dashboard API", lifespan=lifespan)

# ... (cors middleware)

app.include_router(cards.router)
app.include_router(metrics.router)
app.include_router(tracking.router)

@app.get("/api/health")
def health_check():
    return {"status": "ok"}
