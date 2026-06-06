from sqlalchemy import Column, Integer, String, Boolean, DateTime, func
from .database import Base

class Card(Base):
    __tablename__ = "cards"

    id = Column(Integer, primary_key=True, index=True)
    uid = Column(String, unique=True, index=True, nullable=False)
    name = Column(String, nullable=False)
    role = Column(String, default="user")
    is_active = Column(Boolean, default=True)
    tracker_mac = Column(String, unique=True, index=True, nullable=True) # MAC da Tag WiFi
    created_at = Column(DateTime(timezone=True), server_default=func.now())

class AccessLog(Base):
    __tablename__ = "access_logs"

    id = Column(Integer, primary_key=True, index=True)
    uid = Column(String, index=True, nullable=False)
    status = Column(String, nullable=False) # e.g., "granted", "denied"
    timestamp = Column(DateTime(timezone=True), server_default=func.now())

class TrackingLog(Base):
    __tablename__ = "tracking_logs"

    id = Column(Integer, primary_key=True, index=True)
    mac = Column(String, index=True, nullable=False)
    rssi = Column(Integer, nullable=False)
    zone = Column(String, nullable=False) # "Very Near", "Near", "Far"
    scanner = Column(String, nullable=False)
    timestamp = Column(DateTime(timezone=True), server_default=func.now())
