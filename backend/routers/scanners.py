from fastapi import APIRouter, Depends, HTTPException
from sqlalchemy.orm import Session
from typing import List
from ..database import get_db
from ..models import Scanner
from .. import schemas

router = APIRouter(prefix="/api/scanners", tags=["scanners"])

@router.get("/", response_model=List[schemas.ScannerResponse])
def get_scanners(db: Session = Depends(get_db)):
    return db.query(Scanner).all()

@router.put("/{identifier}", response_model=schemas.ScannerResponse)
def update_scanner(identifier: str, scanner_in: schemas.ScannerUpdate, db: Session = Depends(get_db)):
    scanner = db.query(Scanner).filter(Scanner.identifier == identifier).first()
    if not scanner:
        raise HTTPException(status_code=404, detail="Scanner not found")
    
    if scanner_in.name is not None:
        scanner.name = scanner_in.name
    
    db.commit()
    db.refresh(scanner)
    return scanner
