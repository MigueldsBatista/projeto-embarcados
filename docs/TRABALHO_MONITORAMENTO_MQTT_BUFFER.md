# Trabalho: Monitoramento de Amostras + MQTT + Comparativo de Buffer

Este documento explica, de forma completa, onde esta toda a implementacao do trabalho no codigo e como ela funciona.

## 1. Objetivo do trabalho

Implementar um sistema que:

1. captura amostras de sinal (RSSI) de dispositivos detectados via WiFi;
2. transmite os dados via MQTT;
3. compara duas abordagens de buffer para medir eficiencia:
- abordagem ineficiente (realocacao + deslocamento de memoria, custo O(n));
- abordagem eficiente (buffer circular fixo, custo O(1)).

## 2. Escopo escolhido para este projeto

Foi adotado o cenario de **um roteador/scanner principal** com classificacao de proximidade em:
- `Perto`
- `Medio`
- `Longe`

A ideia nao e triangulacao 2D (que exigiria multiplos scanners), e sim uma base robusta de distancia relativa por RSSI.

## 3. Mapa de onde esta no codigo

### 3.1 Firmware ESP32 (captura + benchmark + MQTT)
Arquivo principal:
- `src/main.cpp`

Partes principais:

1. Configuracao MQTT e topicos do benchmark:
- `tracking_topic_discovery`
- `tracking_topic_benchmark_data`
- `tracking_topic_benchmark_perf`
- `tracking_topic_benchmark_summary`

2. Configuracao do benchmark (macros):
- `TRACKING_BENCHMARK_ENABLED`
- `TRACKING_BENCHMARK_WINDOW_SIZE`
- `TRACKING_BENCHMARK_RING_CAPACITY`
- `TRACKING_BENCHMARK_BATCH_SIZE`
- `TRACKING_BENCHMARK_NETWORK_DELAY_MS`
- `TRACKING_BENCHMARK_SNAPSHOT_LIMIT`

3. Estruturas e classes de comparacao:
- `struct TrackingSample`
- `struct TrackingPerfStats`
- `class TrackingInefficientWindow` (vertente ineficiente)
- `class TrackingRingBuffer` (vertente eficiente)

4. Funcoes de publicacao MQTT da telemetria do benchmark:
- `publishTrackingBenchmarkPerf(...)`
- `publishTrackingBenchmarkData(...)`
- `publishTrackingBenchmarkSummary(...)`

5. Funcoes de execucao comparativa:
- `runTrackingInefficientBenchmark(...)`
- `runTrackingCircularBenchmark(...)`
- `runTrackingBenchmark(...)`

6. Captura real de amostras do rastreamento:
- `scan_tracking_tags()`

Nesta funcao, cada rede detectada gera amostra `mac + rssi + scanner` enviada em:
- `tracking/discovery`

e, quando habilitado, dispara o benchmark comparativo com o snapshot coletado.

### 3.2 Backend FastAPI (consumo MQTT + API)
Arquivos:
- `backend/mqtt_client.py`
- `backend/routers/tracking.py`

#### `backend/mqtt_client.py`

Responsabilidades:

1. Assinar topicos MQTT:
- `rfid/scans`
- `tracking/discovery`
- `tracking/heartbeat`
- `tracking/benchmark/data`
- `tracking/benchmark/perf`
- `tracking/benchmark/summary`

2. Processar descoberta de RSSI:
- `process_discovery(...)`

3. Classificar zona por RSSI (base atual):
- `Perto` quando RSSI >= -55
- `Medio` quando RSSI >= -75
- `Longe` caso contrario

4. Salvar rastreamento no banco:
- `process_tracking(...)` grava em `TrackingLog`

5. Armazenar telemetria do benchmark em memoria (cache para frontend):
- `store_benchmark_summary(...)`
- `store_benchmark_perf(...)`
- `store_benchmark_batch(...)`
- `get_benchmark_state()`

#### `backend/routers/tracking.py`

Endpoints disponiveis:

1. `GET /api/tracking/discovery`
- retorna tags recentemente descobertas

2. `GET /api/tracking/live`
- retorna estado ao vivo por aluno (status, RSSI, zona, ultimo visto)

3. `GET /api/tracking/benchmark`
- retorna telemetria do benchmark (resumos + pontos + lotes)

### 3.3 Frontend Vue (visualizacao)
Arquivos:
- `frontend/src/composables/useTracking.ts`
- `frontend/src/types/index.ts`
- `frontend/src/views/TrackingView.vue`

#### `useTracking.ts`
Queries para:
- `/tracking/live`
- `/tracking/discovery`
- `/tracking/benchmark`

#### `types/index.ts`
Tipos principais usados na tela:
- `TrackingStatus` (com zona `Perto | Medio | Longe | Desconhecida`)
- `TrackingBenchmarkState`
- `TrackingBenchmarkSummary`
- `TrackingBenchmarkPerfPoint`
- `TrackingBenchmarkBatch`

#### `TrackingView.vue`
A tela mostra:

1. **Benchmark do Rastreamento**
- resumo por abordagem (`inefficient`, `circular`)
- latencia media/maxima, heap minimo, perdas
- ultimo ponto por abordagem

2. **Rastreamento em Tempo Real**
- aluno
- MAC
- zona (`Perto`, `Medio`, `Longe`)
- RSSI
- ultimo timestamp

## 4. O que foi removido por nao ser mais necessario

Para simplificar ao cenario de um roteador/scanner:

1. visualizacoes de mapa por multiplas salas/scanners;
2. endpoint de heatmap por scanner (`/api/tracking/heatmap`);
3. query de heatmap no composable frontend;
4. tipos de heatmap no frontend.

Com isso, o sistema fica focado no que interessa ao trabalho: **coleta + MQTT + comparacao de eficiencia de buffer**.

## 5. Como rodar para o trabalho

## 5.1 Broker MQTT

Na raiz do projeto:

```bash
docker-compose up -d
```

(Se seu ambiente usa plugin novo do Docker, o comando equivalente e `docker compose up -d`.)

## 5.2 Backend

```bash
python3 -m venv .venv
source .venv/bin/activate
pip install -r requirements.txt
uvicorn backend.main:app --reload --host 0.0.0.0 --port 8000
```

## 5.3 Frontend

```bash
cd frontend
npm install
npm run dev
```

## 5.4 Firmware

Modo normal (rastreamento + RFID):

```bash
pio run -e esp32dev -t upload
```

Modo benchmark habilitado:

```bash
pio run -e esp32trackingbench -t upload
```

## 6. Como interpretar os resultados do benchmark

1. `inefficient` deve piorar com N maior (mais jitter/latencia), pois cada insercao pode copiar/deslocar janela.
2. `circular` deve se manter estavel com crescimento de N, pois insercao/remocao sao O(1).
3. Compare nos resumos:
- `latency_avg_us`
- `latency_max_us`
- `heap_min`
- `dropped`

Esses indicadores sao a evidencia empirica pedida no trabalho.

## 7. Observacoes finais para o relatorio

1. Base teorica:
- abordagem ineficiente: custo O(n)
- ring buffer: custo O(1)

2. Evidencia pratica:
- usar telemetria publicada em `tracking/benchmark/#`
- incluir capturas de serial e dashboard

3. Discussao de rede:
- variacoes de latencia de transmissao sao absorvidas melhor pelo buffer circular.

Com isso, o projeto fica alinhado com a proposta academica sem depender de multiplos roteadores.
