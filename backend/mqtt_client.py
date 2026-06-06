import json
import logging
from collections import deque
from datetime import datetime, timedelta

import paho.mqtt.client as mqtt

from .database import SessionLocal
from .models import AccessLog, Card, TrackingLog

# Configure logging
logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

MQTT_BROKER = "localhost"
MQTT_PORT = 1883
MQTT_TOPIC_SCANS = "rfid/scans"
MQTT_TOPIC_RESPONSES = "rfid/responses/"
MQTT_TOPIC_DISCOVERY = "tracking/discovery"
MQTT_TOPIC_HEARTBEAT = "tracking/heartbeat"
MQTT_TOPIC_BENCHMARK_DATA = "tracking/benchmark/data"
MQTT_TOPIC_BENCHMARK_PERF = "tracking/benchmark/perf"
MQTT_TOPIC_BENCHMARK_SUMMARY = "tracking/benchmark/summary"


class RFIDMQTTClient:
    def __init__(self):
        self.client = mqtt.Client()
        self.client.on_connect = self.on_connect
        self.client.on_message = self.on_message
        self.discovered_tags = {}
        self.benchmark_summaries = {}
        self.benchmark_perf_points = deque(maxlen=240)
        self.benchmark_batches = deque(maxlen=24)

    def on_connect(self, client, userdata, flags, rc, properties):
        logger.info(f"Connected to MQTT Broker with result code {rc}")
        client.subscribe(MQTT_TOPIC_SCANS)
        client.subscribe(MQTT_TOPIC_DISCOVERY)
        client.subscribe(MQTT_TOPIC_HEARTBEAT)
        client.subscribe(MQTT_TOPIC_BENCHMARK_DATA)
        client.subscribe(MQTT_TOPIC_BENCHMARK_PERF)
        client.subscribe(MQTT_TOPIC_BENCHMARK_SUMMARY)

    def on_message(self, client, userdata, msg):
        try:
            payload = json.loads(msg.payload.decode())
            topic = msg.topic

            if topic == MQTT_TOPIC_SCANS:
                uid = payload.get("uid")
                if uid:
                    self.process_scan(uid)
            elif topic == MQTT_TOPIC_DISCOVERY:
                self.process_discovery(payload)
            elif topic == MQTT_TOPIC_BENCHMARK_SUMMARY:
                self.store_benchmark_summary(payload)
            elif topic == MQTT_TOPIC_BENCHMARK_PERF:
                self.store_benchmark_perf(payload)
            elif topic == MQTT_TOPIC_BENCHMARK_DATA:
                self.store_benchmark_batch(payload)
        except json.JSONDecodeError:
            logger.exception("Error decoding MQTT message")
        except Exception:
            logger.exception("Error processing MQTT message")

    def process_discovery(self, payload):
        mac = payload.get("mac")
        rssi = payload.get("rssi")
        scanner = payload.get("scanner", "unknown")
        if not mac:
            return

        self.discovered_tags[mac] = {
            "rssi": rssi,
            "scanner": scanner,
            "timestamp": datetime.now().isoformat(),
        }
        self.process_tracking(mac, rssi, scanner)
        self.cleanup_discovery()

    def cleanup_discovery(self):
        cutoff = datetime.now() - timedelta(minutes=1)
        stale_keys = []
        for mac, info in self.discovered_tags.items():
            timestamp = info.get("timestamp")
            if not timestamp:
                stale_keys.append(mac)
                continue

            try:
                seen_at = datetime.fromisoformat(timestamp)
            except ValueError:
                stale_keys.append(mac)
                continue

            if seen_at < cutoff:
                stale_keys.append(mac)

        for mac in stale_keys:
            self.discovered_tags.pop(mac, None)

    def store_benchmark_summary(self, payload):
        approach = payload.get("approach", "unknown")
        target = payload.get("n_target", 0)
        self.benchmark_summaries[approach] = {
            **payload,
            "summary_key": f"{approach}-{target}",
            "updated_at": datetime.now().isoformat(),
        }

    def store_benchmark_perf(self, payload):
        self.benchmark_perf_points.append(
            {
                **payload,
                "received_at": datetime.now().isoformat(),
            }
        )

    def store_benchmark_batch(self, payload):
        self.benchmark_batches.append(
            {
                **payload,
                "received_at": datetime.now().isoformat(),
            }
        )

    def get_benchmark_state(self):
        return {
            "summaries": list(self.benchmark_summaries.values()),
            "perf_points": list(self.benchmark_perf_points),
            "recent_batches": list(self.benchmark_batches),
        }

    def process_tracking(self, mac, rssi, scanner):
        db = SessionLocal()
        try:
            card = db.query(Card).filter(Card.tracker_mac == mac).first()
            if not card:
                return

            if rssi >= -55:
                zone = "Perto"
            elif rssi >= -75:
                zone = "Medio"
            else:
                zone = "Longe"

            log = TrackingLog(mac=mac, rssi=rssi, zone=zone, scanner=scanner)
            db.add(log)
            db.commit()
            logger.info(f"Tracking logged for {card.name}: {zone} ({rssi}dBm)")
        except Exception:
            logger.exception("Error logging tracking")
            db.rollback()
        finally:
            db.close()

    def process_scan(self, uid):
        db = SessionLocal()
        try:
            card = db.query(Card).filter(Card.uid == uid).first()

            authorized = False
            name = "Unknown"
            status = "unknown"

            if card:
                name = card.name
                if bool(card.is_active):
                    authorized = True
                    status = "authorized"
                else:
                    status = "inactive"

            access_log = AccessLog(uid=uid, status=status)
            db.add(access_log)
            db.commit()

            response_topic = f"{MQTT_TOPIC_RESPONSES}{uid}"
            response_payload = {
                "authorized": authorized,
                "name": name,
            }
            self.client.publish(response_topic, json.dumps(response_payload))
            logger.info(f"Published response for {uid}: {status}")
        except Exception:
            logger.exception("Database error")
            db.rollback()
        finally:
            db.close()

    def start(self):
        try:
            self.client.connect(MQTT_BROKER, MQTT_PORT, 60)
            self.client.loop_start()
            logger.info("MQTT Client loop started")
        except Exception:
            logger.exception("Could not connect to MQTT Broker")

    def stop(self):
        self.client.loop_stop()
        self.client.disconnect()
        logger.info("MQTT Client disconnected")


mqtt_client = RFIDMQTTClient()
