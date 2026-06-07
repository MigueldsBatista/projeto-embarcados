#!/bin/bash
# Upload firmware to Wemos D1 Mini (Tag)
echo "🚀 Iniciando upload para Wemos D1 Mini (Tag) em /dev/ttyUSB1..."
platformio run -d firmware/wemos_tag --target upload --upload-port /dev/ttyUSB1
