# Backend Database Foundation Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Complete the database configuration and models for the RFID Dashboard.

**Architecture:** SQLite database with SQLAlchemy ORM. Models for `Card` and `AccessLog`. Tables are created on application startup.

**Tech Stack:** FastAPI, SQLAlchemy, SQLite, Pytest.

---

### Task 1: Create Database Configuration

**Files:**
- Create: `backend/database.py`

- [ ] **Step 1: Implement `backend/database.py`**

```python
from sqlalchemy import create_engine
from sqlalchemy.ext.declarative import declarative_base
from sqlalchemy.orm import sessionmaker

SQLALCHEMY_DATABASE_URL = "sqlite:///./rfid.db"

engine = create_engine(
    SQLALCHEMY_DATABASE_URL, connect_args={"check_same_thread": False}
)
SessionLocal = sessionmaker(autocommit=False, autoflush=False, bind=engine)

Base = declarative_base()

def get_db():
    db = SessionLocal()
    try:
        yield db
    finally:
        db.close()
```

---

### Task 2: Define Models

**Files:**
- Create: `backend/models.py`

- [ ] **Step 1: Implement `backend/models.py`**

```python
from sqlalchemy import Column, Integer, String, Boolean, DateTime, func
from .database import Base

class Card(Base):
    __tablename__ = "cards"

    id = Column(Integer, primary_key=True, index=True)
    uid = Column(String, unique=True, index=True, nullable=False)
    name = Column(String, nullable=False)
    role = Column(String, default="user")
    is_active = Column(Boolean, default=True)
    created_at = Column(DateTime(timezone=True), server_default=func.now())

class AccessLog(Base):
    __tablename__ = "access_logs"

    id = Column(Integer, primary_key=True, index=True)
    uid = Column(String, index=True, nullable=False)
    status = Column(String, nullable=False) # e.g., "granted", "denied"
    timestamp = Column(DateTime(timezone=True), server_default=func.now())
```

---

### Task 3: Initialize Database in Main

**Files:**
- Modify: `backend/main.py`

- [ ] **Step 1: Update `backend/main.py` to create tables**

```python
from fastapi import FastAPI
from fastapi.middleware.cors import CORSMiddleware
from .database import engine
from . import models

# Create database tables
models.Base.metadata.create_all(bind=engine)

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

---

### Task 4: Verify with Tests

**Files:**
- Create: `tests/backend/test_database.py`

- [ ] **Step 1: Write the failing test**

```python
import sys
import os
import pytest
from sqlalchemy import create_engine
from sqlalchemy.orm import sessionmaker

# Add the project root to sys.path to allow importing from backend
sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), '../..')))

from backend.database import Base
from backend.models import Card, AccessLog

# Use an in-memory SQLite database for testing
SQLALCHEMY_DATABASE_URL = "sqlite:///:memory:"
engine = create_engine(SQLALCHEMY_DATABASE_URL, connect_args={"check_same_thread": False})
TestingSessionLocal = sessionmaker(autocommit=False, autoflush=False, bind=engine)

@pytest.fixture
def db():
    Base.metadata.create_all(bind=engine)
    session = TestingSessionLocal()
    yield session
    session.close()
    Base.metadata.drop_all(bind=engine)

def test_create_card(db):
    new_card = Card(uid="12345678", name="Test User", role="admin")
    db.add(new_card)
    db.commit()
    db.refresh(new_card)
    
    card = db.query(Card).filter(Card.uid == "12345678").first()
    assert card is not None
    assert card.name == "Test User"
    assert card.role == "admin"
    assert card.is_active is True
    assert card.created_at is not None

def test_create_access_log(db):
    new_log = AccessLog(uid="12345678", status="granted")
    db.add(new_log)
    db.commit()
    db.refresh(new_log)
    
    log = db.query(AccessLog).filter(AccessLog.uid == "12345678").first()
    assert log is not None
    assert log.status == "granted"
    assert log.timestamp is not None
```

- [ ] **Step 2: Run test to verify it fails**

Run: `pytest tests/backend/test_database.py -v`
Expected: FAIL (or error because files don't exist yet)

- [ ] **Step 3: Run full TDD cycle by implementing Tasks 1-3 and then running this test**

- [ ] **Step 4: Verify all tests pass**

Run: `pytest tests/backend/ -v`
Expected: PASS
