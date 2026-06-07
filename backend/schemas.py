from pydantic import BaseModel
from datetime import datetime
from typing import Optional

class CardBase(BaseModel):
    uid: str
    name: str
    role: Optional[str] = "user"
    is_active: Optional[bool] = True
    tracker_mac: Optional[str] = None

class CardCreate(CardBase):
    pass

class CardUpdate(BaseModel):
    name: Optional[str] = None
    role: Optional[str] = None
    is_active: Optional[bool] = None
    tracker_mac: Optional[str] = None

class CardResponse(CardBase):
    id: int
    created_at: datetime

    class Config:
        from_attributes = True

class TrackingLogResponse(BaseModel):
    id: int
    mac: str
    rssi: int
    zone: str
    scanner: str
    timestamp: datetime

    class Config:
        from_attributes = True

class ScannerBase(BaseModel):
    identifier: str
    name: str

class ScannerCreate(ScannerBase):
    pass

class ScannerUpdate(BaseModel):
    name: Optional[str] = None

class ScannerResponse(ScannerBase):
    id: int
    last_seen: Optional[datetime] = None
    class Config:
        from_attributes = True

class AccessLogResponse(BaseModel):
    id: int
    uid: str
    status: str
    timestamp: datetime
    person_name: Optional[str] = None

    class Config:
        from_attributes = True
