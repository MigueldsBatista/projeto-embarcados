# Fluxo de Operação: Wemos D1 Mini (Tag de Rastreamento)

Este documento descreve o comportamento lógico e o ciclo de vida da placa Wemos D1 Mini atuando como tag de rastreamento no sistema de crianças.

## 1. Ciclo de Inicialização (Boot)
1. **Power On:** A placa liga via bateria/powerbank.
2. **Setup Serial:** Inicia comunicação serial a 115200 baud para debug.
3. **WiFi Connection:** 
   - Entra em modo `WIFI_STA` (Station).
   - Tenta conectar no SSID configurado.
   - Aguarda conexão (indicado por pontos no Serial).
   - Ao conectar, exibe o **IP** e o **MAC Address** (ID único da tag).

## 2. Loop de Comunicação (Atividade de Rádio)
Para que o ESP32 (Scanner) consiga medir a distância, o rádio WiFi do Wemos precisa estar emitindo pacotes. O loop executa:

1. **Manutenção MQTT:** 
   - Verifica se o cliente MQTT ainda está conectado ao Broker.
   - Caso caia, tenta reconectar automaticamente com um `clientId` aleatório (`WemosTag-XXXX`).
2. **Heartbeat (Emissão de Sinal):**
   - A cada **3 segundos**, a placa gera um JSON:
     ```json
     {
       "mac": "AA:BB:CC:DD:EE:FF",
       "status": "online",
       "rssi": -60
     }
     ```
   - Publica este JSON no tópico `tracking/heartbeat`.
   - **Importante:** Esta publicação força o envio de pacotes WiFi, permitindo que qualquer scanner próximo capture o sinal e meça o RSSI.

## 3. Requisitos de Hardware
- **Alimentação:** Micro-USB (5V) ou pinos 5V/GND.
- **Consumo:** Aproximadamente 80-170mA durante a transmissão WiFi. Recomenda-se bateria de pelo menos 500mAh para algumas horas de autonomia ou Powerbank para uso prolongado.

## 4. Troubleshooting
- **Sem conexão WiFi:** Verifique se as credenciais no `main.cpp` estão corretas.
- **MQTT Falhando (rc=-2):** Verifique se o IP do Broker (seu PC) está acessível na rede e se o Mosquitto está rodando.
- **Não aparece no Scanner:** Verifique se o intervalo de heartbeat não está longo demais (padrão 3s).
