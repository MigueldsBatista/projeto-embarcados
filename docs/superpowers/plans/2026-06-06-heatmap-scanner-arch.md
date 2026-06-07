# Mapa de Calor e Arquitetura de Scanners Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Implementar uma arquitetura para gerenciamento de múltiplos scanners (ESP32) e uma visualização de mapa de calor diário para acompanhar a movimentação dos alunos.

**Architecture:** 
- Backend: Nova tabela `scanners` para nomes amigáveis, auto-registro via MQTT, e endpoint de agregação para heatmap.
- Frontend: Integração com ApexCharts para exibir o mapa de calor e interface de gerenciamento de scanners.

**Tech Stack:** FastAPI, SQLAlchemy, Paho MQTT, Vue 3, ApexCharts, TailwindCSS.

---

### Task 1: Modelo de Dados de Scanners

**Files:**
- Modify: `backend/models.py`
- Modify: `backend/schemas.py`
- Test: `tests/backend/test_scanners.py`

- [ ] **Step 1: Adicionar modelo `Scanner` em `backend/models.py`**

```python
class Scanner(Base):
    __tablename__ = "scanners"
    id = Column(Integer, primary_key=True, index=True)
    identifier = Column(String, unique=True, index=True, nullable=False) # ID técnico do ESP32
    name = Column(String, nullable=False)
    last_seen = Column(DateTime(timezone=True), onupdate=func.now())
```

- [ ] **Step 2: Adicionar schemas em `backend/schemas.py`**

```python
class ScannerBase(BaseModel):
    identifier: str
    name: str

class ScannerCreate(ScannerBase):
    pass

class ScannerUpdate(BaseModel):
    name: Optional[str] = None

class ScannerResponse(ScannerBase):
    id: int
    last_seen: Optional[datetime] = None
    class Config:
        from_attributes = True
```

- [ ] **Step 3: Criar teste de criação de scanner**

```python
from backend.database import SessionLocal
from backend.models import Scanner

def test_create_scanner():
    db = SessionLocal()
    scanner = Scanner(identifier="esp32_01", name="Portão Principal")
    db.add(scanner)
    db.commit()
    db.refresh(scanner)
    assert scanner.id is not None
    assert scanner.identifier == "esp32_01"
    db.delete(scanner)
    db.commit()
    db.close()
```

- [ ] **Step 4: Executar teste**

Run: `pytest tests/backend/test_scanners.py` (Criar o arquivo primeiro)

- [ ] **Step 5: Commit**

```bash
git add backend/models.py backend/schemas.py tests/backend/test_scanners.py
git commit -m "feat: add Scanner model and schemas"
```

---

### Task 2: Auto-registro de Scanners via MQTT

**Files:**
- Modify: `backend/mqtt_client.py`

- [ ] **Step 1: Implementar lógica de auto-registro no `process_tracking`**

```python
def process_tracking(self, mac, rssi, scanner_id):
    db = SessionLocal()
    try:
        # Auto-registro do scanner
        from .models import Scanner
        scanner = db.query(Scanner).filter(Scanner.identifier == scanner_id).first()
        if not scanner:
            scanner = Scanner(identifier=scanner_id, name=f"Scanner {scanner_id}")
            db.add(scanner)
            db.commit()
            logger.info(f"New scanner registered: {scanner_id}")
        else:
            scanner.last_seen = datetime.now()
            db.commit()

        # ... (restante da lógica existente para Card e TrackingLog)
```

- [ ] **Step 2: Validar se o log está salvando o identifier do scanner corretamente**

---

### Task 3: API de Gerenciamento de Scanners

**Files:**
- Create: `backend/routers/scanners.py`
- Modify: `backend/main.py`

- [ ] **Step 1: Criar router de scanners**

```python
from fastapi import APIRouter, Depends, HTTPException
from sqlalchemy.orm import Session
from typing import List
from ..database import get_db
from ..models import Scanner
from .. import schemas

router = APIRouter(prefix="/api/scanners", tags=["scanners"])

@router.get("/", response_model=List[schemas.ScannerResponse])
def get_scanners(db: Session = Depends(get_db)):
    return db.query(Scanner).all()

@router.put("/{identifier}", response_model=schemas.ScannerResponse)
def update_scanner(identifier: str, scanner_in: schemas.ScannerUpdate, db: Session = Depends(get_db)):
    scanner = db.query(Scanner).filter(Scanner.identifier == identifier).first()
    if not scanner:
        raise HTTPException(status_code=404, detail="Scanner not found")
    scanner.name = scanner_in.name
    db.commit()
    db.refresh(scanner)
    return scanner
```

- [ ] **Step 2: Registrar no `backend/main.py`**

```python
from .routers import cards, metrics, tracking, scanners
# ...
app.include_router(scanners.router)
```

---

### Task 4: Endpoint de Agregação para Heatmap

**Files:**
- Modify: `backend/routers/tracking.py`

- [ ] **Step 1: Adicionar rota `/api/tracking/heatmap/{mac}`**

```python
@router.get("/heatmap/{mac}")
def get_heatmap_data(mac: str, db: Session = Depends(get_db)):
    # Pega logs das últimas 24h
    start_time = datetime.now() - timedelta(days=1)
    logs = db.query(TrackingLog).filter(
        TrackingLog.mac == mac,
        TrackingLog.timestamp >= start_time
    ).all()
    
    # Agrega por scanner e por hora
    # Retorno esperado pelo ApexCharts: List[{ name: "Scanner Name", data: List[{ x: "00h", y: pontuacao }] }]
    # ... lógica de agregação ...
    return heatmap_payload
```

---

### Task 5: Integração Frontend com ApexCharts

**Files:**
- Modify: `frontend/package.json`
- Create: `frontend/src/components/HeatmapChart.vue`

- [ ] **Step 1: Instalar dependências**

Run: `cd frontend && bun add apexcharts vue3-apexcharts`

- [ ] **Step 2: Criar componente de gráfico**

```vue
<script setup lang="ts">
import VueApexCharts from "vue3-apexcharts";
defineProps<{ series: any[] }>();
const chartOptions = {
  chart: { type: 'heatmap' },
  dataLabels: { enabled: false },
  colors: ["#008FFB"],
  xaxis: { type: 'category' }
};
</script>
<template>
  <VueApexCharts type="heatmap" height="350" :options="chartOptions" :series="series" />
</template>
```

---

### Task 6: UI de Gerenciamento e Visualização

**Files:**
- Create: `frontend/src/views/ScannersView.vue`
- Modify: `frontend/src/views/TrackingView.vue`

- [ ] **Step 1: Adicionar visualização do Heatmap no TrackingView**
- [ ] **Step 2: Criar página de Scanners para renomear os pontos**
