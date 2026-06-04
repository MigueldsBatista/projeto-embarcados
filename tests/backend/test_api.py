import pytest
from fastapi.testclient import TestClient
from sqlalchemy import create_engine
from sqlalchemy.orm import sessionmaker
from backend.main import app
from backend.database import Base, get_db
from backend.models import Card, AccessLog

# Setup test database
SQLALCHEMY_DATABASE_URL = "sqlite:///./test_api.db"
engine = create_engine(SQLALCHEMY_DATABASE_URL, connect_args={"check_same_thread": False})
TestingSessionLocal = sessionmaker(autocommit=False, autoflush=False, bind=engine)

def override_get_db():
    try:
        db = TestingSessionLocal()
        yield db
    finally:
        db.close()

app.dependency_overrides[get_db] = override_get_db

@pytest.fixture(autouse=True)
def setup_db():
    Base.metadata.create_all(bind=engine)
    yield
    Base.metadata.drop_all(bind=engine)

client = TestClient(app)

def test_get_cards_empty():
    response = client.get("/api/cards/")
    assert response.status_code == 200
    assert response.json() == []

def test_create_card():
    response = client.post(
        "/api/cards/",
        params={"uid": "123456", "name": "Test Card", "role": "admin"}
    )
    assert response.status_code == 200
    data = response.json()
    assert data["uid"] == "123456"
    assert data["name"] == "Test Card"
    assert data["role"] == "admin"
    assert "id" in data

def test_get_cards_with_data():
    # Pre-populate
    client.post("/api/cards/", params={"uid": "111", "name": "C1", "role": "user"})
    client.post("/api/cards/", params={"uid": "222", "name": "C2", "role": "user"})
    
    response = client.get("/api/cards/")
    assert response.status_code == 200
    assert len(response.json()) == 2

def test_get_metrics_logs_empty():
    response = client.get("/api/metrics/logs")
    assert response.status_code == 200
    assert response.json() == []

def test_get_metrics_summary_empty():
    response = client.get("/api/metrics/summary")
    assert response.status_code == 200
    assert response.json() == {"total_scans": 0, "unknown_scans": 0}

def test_get_metrics_with_data():
    db = TestingSessionLocal()
    db.add(AccessLog(uid="123", status="authorized"))
    db.add(AccessLog(uid="456", status="unknown"))
    db.add(AccessLog(uid="789", status="unknown"))
    db.commit()
    db.close()

    # Test logs
    response = client.get("/api/metrics/logs")
    assert response.status_code == 200
    assert len(response.json()) == 3

    # Test summary
    response = client.get("/api/metrics/summary")
    assert response.status_code == 200
    assert response.json() == {"total_scans": 3, "unknown_scans": 2}
