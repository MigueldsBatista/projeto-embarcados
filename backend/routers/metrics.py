from fastapi import APIRouter, Depends
from sqlalchemy.orm import Session
from ..database import get_db
from ..models import AccessLog

router = APIRouter(prefix="/api/metrics", tags=["metrics"])

@router.get("/logs")
def get_logs(limit: int = 50, db: Session = Depends(get_db)):
    return db.query(AccessLog).order_by(AccessLog.timestamp.desc()).limit(limit).all()

@router.get("/summary")
def get_summary(db: Session = Depends(get_db)):
    total = db.query(AccessLog).count()
    unknowns = db.query(AccessLog).filter(AccessLog.status == "unknown").count()
    return {"total_scans": total, "unknown_scans": unknowns}
