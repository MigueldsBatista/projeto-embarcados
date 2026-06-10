import sys
import os
import unittest
from unittest.mock import MagicMock, patch
import json
import pytest
from sqlalchemy import create_engine
from sqlalchemy.orm import sessionmaker

# Add the project root to sys.path to allow importing from backend
sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), '../..')))

from backend.database import Base
from backend.models import Card, AccessLog
from backend.mqtt_client import RFIDMQTTClient

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

@patch('backend.mqtt_client.SessionLocal')
@patch('paho.mqtt.client.Client')
def test_mqtt_process_scan_authorized(mock_mqtt_client, mock_session_local, db):
    # Setup mock session to return our test db
    mock_session_local.return_value = db
    
    # Setup MQTT client mock
    mqtt_service = RFIDMQTTClient()
    mqtt_service.client = MagicMock()
    
    # Add an active card to the DB
    test_uid = "A1B2C3D4"
    card = Card(uid=test_uid, name="John Doe", is_active=True)
    db.add(card)
    db.commit()
    
    # Process scan
    mqtt_service.process_scan(test_uid)
    
    # Verify response published
    mqtt_service.client.publish.assert_called_once()
    args, kwargs = mqtt_service.client.publish.call_args
    assert args[0] == f"rfid/responses/{test_uid}"
    payload = json.loads(args[1])
    assert payload["authorized"] is True
    assert payload["name"] == "John Doe"
    
    # Verify log entry created
    log = db.query(AccessLog).filter(AccessLog.uid == test_uid).first()
    assert log is not None
    assert log.status == "authorized"

@patch('backend.mqtt_client.SessionLocal')
@patch('paho.mqtt.client.Client')
def test_mqtt_process_scan_unknown(mock_mqtt_client, mock_session_local, db):
    mock_session_local.return_value = db
    mqtt_service = RFIDMQTTClient()
    mqtt_service.client = MagicMock()
    
    test_uid = "UNKNOWN"
    
    # Process scan
    mqtt_service.process_scan(test_uid)
    
    # Verify response
    args, kwargs = mqtt_service.client.publish.call_args
    payload = json.loads(args[1])
    assert payload["authorized"] is False
    assert payload["name"] == "Unknown"
    
    # Verify log
    log = db.query(AccessLog).filter(AccessLog.uid == test_uid).first()
    assert log is not None
    assert log.status == "unknown"

@patch('backend.mqtt_client.SessionLocal')
@patch('paho.mqtt.client.Client')
def test_mqtt_process_scan_inactive(mock_mqtt_client, mock_session_local, db):
    mock_session_local.return_value = db
    mqtt_service = RFIDMQTTClient()
    mqtt_service.client = MagicMock()
    
    test_uid = "INACTIVE"
    card = Card(uid=test_uid, name="Jane Smith", is_active=False)
    db.add(card)
    db.commit()
    
    # Process scan
    mqtt_service.process_scan(test_uid)
    
    # Verify response
    args, kwargs = mqtt_service.client.publish.call_args
    payload = json.loads(args[1])
    assert payload["authorized"] is False
    assert payload["name"] == "Jane Smith"
    
    # Verify log
    log = db.query(AccessLog).filter(AccessLog.uid == test_uid).first()
    assert log is not None
    assert log.status == "inactive"
