# Plano de Integração: Otimização de Telemetria com Buffer Circular

## 1. Visão Geral
Este plano descreve as etapas para extrair e adaptar os requisitos de telemetria e o comparativo de `Buffer Circular` (Vertente 2) vs `Buffer Ineficiente` (Vertente 1) da branch `marlon_requisitos_cumpridos` para a branch `main`.

## 2. Escopo da Integração

### 2.1. Firmware (ESP32)
**Arquivo:** `src/main.cpp`
*   **Adicionar Macros e Tópicos MQTT:** `TRACKING_BENCHMARK_ENABLED`, e os tópicos para dados, performance e resumo.
*   **Implementar Estruturas de Dados:** `TrackingSample`, `TrackingPerfStats`.
*   **Implementar Vertente 1 (Anti-Padrão):** Classe `TrackingInefficientWindow` utilizando `malloc/free` a cada novo registro.
*   **Implementar Vertente 2 (Circular):** Classe `TrackingRingBuffer` utilizando um array fixo de O(1).
*   **Implementar Instrumentação e Publicação:** Funções de benchmark (`runTrackingBenchmark`, `publishTrackingBenchmarkSummary`, etc.) e instrumentação com `micros()` e `ESP.getFreeHeap()`.
*   **Integrar na Varredura WiFi:** Disparar o benchmark durante o escaneamento na função `scan_tracking_tags()`.

### 2.2. Backend (FastAPI)
**Arquivos:** `backend/mqtt_client.py` e `backend/routers/tracking.py`
*   **Assinatura de Tópicos:** Adicionar os novos tópicos `tracking/benchmark/*` no cliente MQTT.
*   **Armazenamento em Memória:** Criar lógicas para guardar (`store_benchmark_summary`, `store_benchmark_perf`, etc.) as estatísticas em cache para disponibilizar para o frontend.
*   **Rotas da API:** Adaptar o router `tracking.py` para disponibilizar a rota `GET /api/tracking/benchmark`.

### 2.3. Documentação
**Arquivos:** `docs/TRABALHO_MONITORAMENTO_MQTT_BUFFER.md`
*   **Trazer o Relatório Técnico:** Copiar o documento explicativo adicionado pelo Marlon na branch original para comprovar a conclusão dos requisitos acadêmicos (Complexidade e Memória).

### 2.4. Frontend (Vue.js - Opcional/Necessário)
**Arquivos:** `frontend/src/composables/useTracking.ts` e views associadas.
*   *Nota:* Como o objetivo é trazer "só o conteúdo necessário", adaptaremos o mínimo para permitir que os gráficos sejam exibidos no painel já existente, ou traremos as views exclusivas de rastreamento (`TrackingView.vue`) conforme implementadas.

## 3. Passo a Passo da Execução

1.  **Checkout de Arquivos Específicos:** Em vez de um merge completo (que poderia trazer lixo de outras funcionalidades não desejadas), usaremos `git checkout origin/marlon_requisitos_cumpridos -- <caminho>` ou aplicaremos patches manuais cirúrgicos nos arquivos `src/main.cpp` e backend.
2.  **Revisão do Código (Lint/Types):** Garantir que as lógicas de C++ e Python inseridas estão em conformidade com as padronizações da `main`.
3.  **Teste de Compilação:** Compilar o firmware ESP32 para verificar se o `Ring Buffer` e o `Inefficient Window` não causam quebra no PlatformIO.
4.  **Teste de Execução:** Iniciar o backend localmente e verificar se os payloads de benchmark MQTT estão sendo consumidos corretamente.

## 4. Conclusão
Ao final deste plano, a `main` conterá empiricamente a comprovação exigida pelo projeto acadêmico de Sistemas Embarcados sem sujar o histórico principal com modificações desnecessárias da outra branch.