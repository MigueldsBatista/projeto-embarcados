from fastapi import APIRouter, Depends
from sqlalchemy.orm import Session
from ..database import get_db
from ..models import AccessLog, Card
from ..schemas import AccessLogResponse
from typing import List

router = APIRouter(prefix="/api/metrics", tags=["metrics"])

@router.get("/logs", response_model=List[AccessLogResponse])
def get_logs(limit: int = 50, db: Session = Depends(get_db)):
    # Perform an outer join to include logs without a matching card (unknown UIDs)
    results = db.query(
        AccessLog.id,
        AccessLog.uid,
        AccessLog.status,
        AccessLog.timestamp,
        Card.name.label("person_name")
    ).outerjoin(
        Card, AccessLog.uid == Card.uid
    ).order_by(AccessLog.timestamp.desc()).limit(limit).all()
    
    return results

@router.get("/summary")
def get_summary(db: Session = Depends(get_db)):
    total = db.query(AccessLog).count()
    unknowns = db.query(AccessLog).filter(AccessLog.status == "unknown").count()
    return {"total_scans": total, "unknown_scans": unknowns}
