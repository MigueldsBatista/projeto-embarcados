import json
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

class RFIDMQTTClient:
    def __init__(self):
        self.client = mqtt.Client(callback_api_version=mqtt.CallbackAPIVersion.VERSION2)
        self.client.on_connect = self.on_connect
        self.client.on_message = self.on_message

    def on_connect(self, client, userdata, flags, rc, properties):
        logger.info(f"Connected to MQTT Broker with result code {rc}")
        client.subscribe(MQTT_TOPIC_SCANS)

    def on_message(self, client, userdata, msg):
        try:
            payload = json.loads(msg.payload.decode())
            uid = payload.get("uid")
            if not uid:
                logger.warning("Received message without UID")
                return

            logger.info(f"Processing scan for UID: {uid}")
            self.process_scan(uid)
        except Exception as e:
            logger.error(f"Error processing MQTT message: {e}")

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
