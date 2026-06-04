from fastapi import APIRouter, Depends, HTTPException
from sqlalchemy.orm import Session
from typing import List
from ..database import get_db
from ..models import Card
from .. import schemas

router = APIRouter(prefix="/api/cards", tags=["cards"])

@router.get("/", response_model=List[schemas.CardResponse])
def get_cards(db: Session = Depends(get_db)):
    return db.query(Card).all()

@router.post("/", response_model=schemas.CardResponse)
def add_card(card_in: schemas.CardCreate, db: Session = Depends(get_db)):
    # Check if UID already exists
    existing_card = db.query(Card).filter(Card.uid == card_in.uid).first()
    if existing_card:
        raise HTTPException(status_code=400, detail="UID already registered")
    
    card = Card(**card_in.model_dump())
    db.add(card)
    db.commit()
    db.refresh(card)
    return card

@router.put("/{uid}", response_model=schemas.CardResponse)
def update_card(uid: str, card_in: schemas.CardUpdate, db: Session = Depends(get_db)):
    card = db.query(Card).filter(Card.uid == uid).first()
    if not card:
        raise HTTPException(status_code=404, detail="Card not found")
    
    update_data = card_in.model_dump(exclude_unset=True)
    for key, value in update_data.items():
        setattr(card, key, value)
    
    db.commit()
    db.refresh(card)
    return card

@router.delete("/{uid}")
def delete_card(uid: str, db: Session = Depends(get_db)):
    card = db.query(Card).filter(Card.uid == uid).first()
    if not card:
        raise HTTPException(status_code=404, detail="Card not found")
    
    db.delete(card)
    db.commit()
    return {"message": "Card deleted successfully"}
