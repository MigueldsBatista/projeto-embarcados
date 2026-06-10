#!/bin/bash
# Upload firmware to Wemos D1 Mini (Tag)
PORT=$1
if [ -z "$PORT" ]; then
    PORT="/dev/ttyUSB1"
fi

echo "🚀 Iniciando upload para Wemos D1 Mini (Tag) em $PORT..."
platformio run -d firmware/wemos --target upload --upload-port $PORT
