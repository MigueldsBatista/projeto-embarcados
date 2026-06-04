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
