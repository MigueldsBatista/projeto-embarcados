from fastapi import APIRouter, Depends
from sqlalchemy.orm import Session
from typing import List, Dict
from ..database import get_db, SessionLocal
from ..models import Card, TrackingLog, Scanner
from ..mqtt_client import mqtt_client
from datetime import datetime, timedelta, timezone
import collections
import statistics

# RSSI to Meters constants
MEASURED_POWER = -62
N_ENV = 2.5

def rssi_to_meters(rssi: int) -> float:
    """Estimates distance in meters based on RSSI"""
    if not rssi:
        return 0.0
    return 10 ** ((MEASURED_POWER - rssi) / (10 * N_ENV))

router = APIRouter(prefix="/api/tracking", tags=["tracking"])

@router.get("/discovery", response_model=Dict[str, dict])
def get_discovered_tags():
    """Returns recently discovered MAC addresses (last 1 min)"""
    return mqtt_client.discovered_tags

@router.get("/benchmark")
def get_tracking_benchmark():
    """Returns the current state of the tracking benchmark"""
    return mqtt_client.get_benchmark_state()

@router.get("/live")
def get_live_tracking(db: Session = Depends(get_db)):
    """Returns the current location status of all students with trackers"""
    # Search for cards that have a tracker_mac
    cards = db.query(Card).filter(Card.tracker_mac != None).all()
    
    results = []
    now_utc = datetime.now(timezone.utc)
    
    for card in cards:
        # Get the absolute last log for this MAC
        last_log = db.query(TrackingLog)\
            .filter(TrackingLog.mac == card.tracker_mac)\
            .order_by(TrackingLog.timestamp.desc())\
            .first()
        
        status = "offline"
        if last_log:
            log_time = last_log.timestamp
            # If naive, assume the DB saved in UTC (SQLite default for func.now())
            if log_time.tzinfo is None:
                log_time = log_time.replace(tzinfo=timezone.utc)
                
            # Set to offline if the last signal was more than 2 minutes ago
            if now_utc - log_time <= timedelta(minutes=2):
                status = "online"
        
        results.append({
            "student_name": card.name,
            "mac": card.tracker_mac,
            "status": status,
            "rssi": last_log.rssi if last_log else None,
            "zone": last_log.zone if last_log else "Unknown",
            "last_seen": last_log.timestamp if last_log else None
        })
    
    return results

@router.get("/heatmap/{mac}")
def get_heatmap_data(mac: str, db: Session = Depends(get_db)):
    """Returns structured data for stacked bar charts per scanner"""
    # Logs from the last 24 hours
    now_utc = datetime.now(timezone.utc)
    start_time = now_utc - timedelta(days=1)
    
    logs = db.query(TrackingLog).filter(
        TrackingLog.mac == mac,
        TrackingLog.timestamp >= start_time
    ).all()

    # Map scanner identifiers to friendly names
    scanners = {s.identifier: s.name for s in db.query(Scanner).all()}
    
    # Data structure: { scanner_id: { hour: { zone: count } } }
    agg = collections.defaultdict(lambda: collections.defaultdict(lambda: collections.defaultdict(int)))
    # For boxplot: { scanner_id: { hour: [rssi_values] } }
    agg_rssi = collections.defaultdict(lambda: collections.defaultdict(list))
    
    for log in logs:
        # Group by the start of the hour
        dt = log.timestamp
        if dt.tzinfo is None:
            dt = dt.replace(tzinfo=timezone.utc)
        
        hour_iso = dt.replace(minute=0, second=0, microsecond=0).isoformat()
        if not hour_iso.endswith('Z') and '+00:00' not in hour_iso:
            hour_iso += 'Z'
        
        agg[log.scanner][hour_iso][log.zone] += 1
        distance = round(rssi_to_meters(log.rssi), 2)
        agg_rssi[log.scanner][hour_iso].append(distance)

    # Generate the last 24 hours as ISO strings
    hours_iso = []
    curr = now_utc.replace(minute=0, second=0, microsecond=0) - timedelta(hours=23)
    for _ in range(24):
        h_str = curr.isoformat()
        if not h_str.endswith('Z') and '+00:00' not in h_str:
            h_str += 'Z'
        hours_iso.append(h_str)
        curr += timedelta(hours=1)

    zones = ["Very Near", "Near", "Far"]
    payload = []
    
    # Sort scanner IDs for consistent display
    for s_id in sorted(agg.keys()):
        s_name = scanners.get(s_id, f"Scanner {s_id}")
        scanner_series = []
        
        # 1. Stacked Bar Series
        for zone in zones:
            series_data = []
            for h in hours_iso:
                series_data.append({"x": h, "y": agg[s_id][h][zone]})
            scanner_series.append({"name": zone, "data": series_data})
            
        # 2. BoxPlot Series (RSSI Distribution)
        boxplot_data = []
        for h in hours_iso:
            rssi_list = agg_rssi[s_id][h]
            if rssi_list:
                if len(rssi_list) >= 2:
                    q1, median, q3 = statistics.quantiles(rssi_list, n=4)
                    y_val = [min(rssi_list), q1, median, q3, max(rssi_list)]
                else:
                    val = rssi_list[0]
                    y_val = [val, val, val, val, val]
                boxplot_data.append({"x": h, "y": y_val})
        
        scanner_boxplot = [{"type": "boxPlot", "name": "Distance (m)", "data": boxplot_data}]

        payload.append({
            "scanner_id": s_id,
            "scanner_name": s_name,
            "series": scanner_series,
            "boxplot_series": scanner_boxplot
        })
    
    return payload
