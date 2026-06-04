from fastapi import APIRouter, Depends
from sqlalchemy.orm import Session
from ..database import get_db
from ..models import Card

router = APIRouter(prefix="/api/cards", tags=["cards"])

@router.get("/")
def get_cards(db: Session = Depends(get_db)):
    return db.query(Card).all()

@router.post("/")
def add_card(uid: str, name: str, role: str, db: Session = Depends(get_db)):
    card = Card(uid=uid, name=name, role=role)
    db.add(card)
    db.commit()
    db.refresh(card)
    return card
