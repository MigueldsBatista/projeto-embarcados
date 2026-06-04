from pydantic import BaseModel
from datetime import datetime
from typing import Optional

class CardBase(BaseModel):
    uid: str
    name: str
    role: Optional[str] = "user"
    is_active: Optional[bool] = True

class CardCreate(CardBase):
    pass

class CardUpdate(BaseModel):
    name: Optional[str] = None
    role: Optional[str] = None
    is_active: Optional[bool] = None

class CardResponse(CardBase):
    id: int
    created_at: datetime

    class Config:
        from_attributes = True
