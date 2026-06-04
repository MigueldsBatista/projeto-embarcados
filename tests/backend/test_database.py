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
