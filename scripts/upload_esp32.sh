#!/bin/bash
# Upload firmware to ESP32 (Scanner)
echo "🚀 Iniciando upload para ESP32 (Scanner) em /dev/ttyUSB0..."
platformio run --target upload --upload-port /dev/ttyUSB0
