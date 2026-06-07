import json
from datetime import datetime
import paho.mqtt.client as mqtt
from .database import SessionLocal
from .models import Card, AccessLog
import logging

# Configure logging
logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

MQTT_BROKER = "localhost"
MQTT_PORT = 1883
MQTT_TOPIC_SCANS = "rfid/scans"
MQTT_TOPIC_RESPONSES = "rfid/responses/"
MQTT_TOPIC_DISCOVERY = "tracking/discovery"
MQTT_TOPIC_HEARTBEAT = "tracking/heartbeat"

class RFIDMQTTClient:
    def __init__(self):
        self.client = mqtt.Client(callback_api_version=mqtt.CallbackAPIVersion.VERSION2)
        self.client.on_connect = self.on_connect
        self.client.on_message = self.on_message
        self.discovered_tags = {} # MAC -> {rssi, timestamp}

    def on_connect(self, client, userdata, flags, rc, properties):
        logger.info(f"Connected to MQTT Broker with result code {rc}")
        client.subscribe(MQTT_TOPIC_SCANS)
        client.subscribe(MQTT_TOPIC_DISCOVERY)
        client.subscribe(MQTT_TOPIC_HEARTBEAT)

    def on_message(self, client, userdata, msg):
        try:
            payload = json.loads(msg.payload.decode())
            topic = msg.topic

            if topic == MQTT_TOPIC_SCANS:
                uid = payload.get("uid")
                if uid:
                    self.process_scan(uid)
            elif topic == MQTT_TOPIC_DISCOVERY:
                mac = payload.get("mac")
                rssi = payload.get("rssi")
                scanner = payload.get("scanner", "unknown")
                if mac:
                    # Cache para o frontend consultar o que está perto
                    self.discovered_tags[mac] = {
                        "rssi": rssi,
                        "timestamp": datetime.now().isoformat()
                    }
                    self.process_tracking(mac, rssi, scanner)
                    # Limpa cache antigo (> 1 min)
                    self._cleanup_discovery()
        except Exception as e:
            logger.error(f"Error processing MQTT message: {e}")

    def _cleanup_discovery(self):
        from datetime import datetime as dt, timedelta
        now = dt.now()
        to_delete = [mac for mac, data in self.discovered_tags.items() 
                     if dt.fromisoformat(data["timestamp"]) < now - timedelta(minutes=1)]
        for mac in to_delete:
            del self.discovered_tags[mac]

    def process_tracking(self, mac, rssi, scanner):
        db = SessionLocal()
        try:
            # Verifica se este MAC pertence a algum aluno
            card = db.query(Card).filter(Card.tracker_mac == mac).first()
            if not card:
                return # Ignora dispositivos não registrados (lixo WiFi)

            # Classifica a zona baseada no RSSI
            if rssi >= -55:
                zone = "Very Near"
            elif rssi >= -75:
                zone = "Near"
            else:
                zone = "Far"

            # Salva o log de rastreamento
            from .models import TrackingLog
            log = TrackingLog(mac=mac, rssi=rssi, zone=zone, scanner=scanner)
            db.add(log)
            db.commit()
            logger.info(f"Tracking logged for {card.name}: {zone} ({rssi}dBm)")

        except Exception as e:
            logger.error(f"Error logging tracking: {e}")
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
                if card.is_active:
                    authorized = True
                    status = "authorized"
                else:
                    status = "inactive"
            
            # Log the access
            access_log = AccessLog(uid=uid, status=status)
            db.add(access_log)
            db.commit()

            # Publish response
            response_topic = f"{MQTT_TOPIC_RESPONSES}{uid}"
            response_payload = {
                "authorized": authorized,
                "name": name
            }
            self.client.publish(response_topic, json.dumps(response_payload))
            logger.info(f"Published response for {uid}: {status}")

        except Exception as e:
            logger.error(f"Database error: {e}")
            db.rollback()
        finally:
            db.close()

    def start(self):
        try:
            self.client.connect(MQTT_BROKER, MQTT_PORT, 60)
            self.client.loop_start()
            logger.info("MQTT Client loop started")
        except Exception as e:
            logger.error(f"Could not connect to MQTT Broker: {e}")

    def stop(self):
        self.client.loop_stop()
        self.client.disconnect()
        logger.info("MQTT Client disconnected")

mqtt_client = RFIDMQTTClient()
