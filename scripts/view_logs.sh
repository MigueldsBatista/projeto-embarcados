#!/bin/bash
# View logs for both devices or specific one
DEVICE=$1

if [ "$DEVICE" == "esp32" ]; then
    echo "📋 Mostrando logs do ESP32 (Scanner)..."
    tail -f esp32_monitor.log
elif [ "$DEVICE" == "wemos" ]; then
    echo "📋 Mostrando logs do Wemos (Tag)..."
    tail -f wemos_monitor.log
else
    echo "Uso: ./scripts/view_logs.sh [esp32|wemos]"
    echo "Exemplo: ./scripts/view_logs.sh esp32"
fi
