# Projeto: ESP32 + RC522 (RFID)

Leitor RFID RC522 (MFRC522) via SPI no ESP32, lendo UID do cracha e mapeando para informacoes.

Docs:
- `docs/RC522_ESP32.md` (conexoes e troubleshooting)
- `docs/USO_E_CADASTRO.md` (como ler UID e cadastrar informacoes)
- `docs/TRABALHO_MONITORAMENTO_MQTT_BUFFER.md` (documentacao completa do trabalho de benchmark de buffers + MQTT)

Modo de benchmark do rastreamento:
- `esp32dev`: scanner normal com RFID + descoberta de tags WiFi.
- `esp32trackingbench`: usa a coleta real de RSSI do rastreamento para comparar vertente ineficiente vs buffer circular e publica telemetria em `tracking/benchmark/#`.
