# Fluxo de Operação: Wemos D1 Mini (Tag de Rastreamento)

Este documento descreve o comportamento lógico e o ciclo de vida da placa Wemos D1 Mini atuando como tag de rastreamento no sistema de crianças.

## 1. Ciclo de Inicialização (Boot)
1. **Power On:** A placa liga via bateria/powerbank.
2. **Setup Serial:** Inicia comunicação serial a 115200 baud para debug.
3. **WiFi Connection:** 
   - Entra em modo `WIFI_STA` (Station).
   - Tenta conectar no SSID configurado.
   - Caso não conecte ou seja a primeira vez, a placa entra em modo **Access Point (AP)**.
   - **Nome do AP:** `WemosTag-XXXX` (onde `XXXX` são os últimos dígitos do MAC da placa).
   - **Endereço de Configuração:** Se o seu celular não abrir a página automaticamente, abra o navegador e acesse: **`http://192.168.4.1`**
   - No portal, selecione a sua rede WiFi e defina o IP do Broker MQTT.

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

## 3. Compilação e Upload (PlatformIO CLI)

Para compilar e carregar o firmware na placa Wemos D1 Mini, utilize os seguintes comandos no terminal:

```bash
# Navegar para a pasta do firmware (opcional se usar o flag -d)
# Compilar e fazer o upload
platformio run -d firmware/wemos_tag --target upload

# Monitorar o Serial e salvar em log simultaneamente
platformio device monitor -d firmware/wemos_tag -b 115200 | tee wemos_monitor.log
```

## 4. Requisitos de Hardware
- **Alimentação:** Micro-USB (5V) ou pinos 5V/GND.
- **Consumo:** Aproximadamente 80-170mA durante a transmissão WiFi. Recomenda-se bateria de pelo menos 500mAh para algumas horas de autonomia ou Powerbank para uso prolongado.

## 5. Entendendo os Endereços MAC

Cada placa Wemos possui **dois endereços MAC fixos** que nunca mudam, mas são usados em situações diferentes:

1.  **STA MAC (Station):** Usado quando a placa se conecta ao seu roteador. É o que aparece no log de Heartbeat e no Broker MQTT.
2.  **AP MAC (BSSID):** Usado quando a placa emite o sinal de Access Point. **Este é o MAC que o ESP32 Scanner enxerga.**

**Diferença Visual:** Geralmente, se o STA MAC começa com `8C`, o AP MAC começará com `8E`. O restante do código costuma ser idêntico. No sistema de rastreamento, **sempre utilize o MAC que aparecer na lista de Descoberta (Discovery)**.

## 6. Feedback Visual (LED)
A placa piscará o LED azul interno rapidamente a cada 3 segundos para confirmar que o sinal WiFi foi emitido com sucesso.
