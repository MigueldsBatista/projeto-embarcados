from datetime import datetime, timedelta
from typing import Annotated
from typing import Dict

from fastapi import APIRouter, Depends
from sqlalchemy.orm import Session

from ..database import get_db
from ..models import Card, TrackingLog
from ..mqtt_client import mqtt_client

router = APIRouter(prefix="/api/tracking", tags=["tracking"])

DbSession = Annotated[Session, Depends(get_db)]

@router.get("/discovery", response_model=Dict[str, dict])
def get_discovered_tags():
    """Retorna os MAC addresses descobertos recentemente (último 1 min)"""
    return mqtt_client.discovered_tags

@router.get("/live")
def get_live_tracking(db: DbSession):
    """Retorna o status atual de localização de todos os alunos com rastreador"""
    # Busca cards que possuem tracker_mac
    cards = db.query(Card).filter(Card.tracker_mac != None).all()
    
    results = []
    for card in cards:
        # Busca o último log desse MAC nos últimos 5 minutos
        last_log = db.query(TrackingLog)\
            .filter(TrackingLog.mac == card.tracker_mac)\
            .filter(TrackingLog.timestamp > datetime.now() - timedelta(minutes=5))\
            .order_by(TrackingLog.timestamp.desc())\
            .first()
        
        results.append({
            "student_name": card.name,
            "mac": card.tracker_mac,
            "status": "online" if last_log else "offline",
            "rssi": last_log.rssi if last_log else None,
            "zone": last_log.zone if last_log else "Desconhecida",
            "scanner": last_log.scanner if last_log else None,
            "last_seen": last_log.timestamp if last_log else None
        })
    
    return results

@router.get("/benchmark")
def get_tracking_benchmark():
    """Retorna a telemetria mais recente do benchmark de rastreamento."""
    return mqtt_client.get_benchmark_state()
