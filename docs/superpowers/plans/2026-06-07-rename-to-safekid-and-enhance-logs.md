# SafeKid Renaming and Log Enhancement Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Rename the application to SafeKid across all components and enhance the dashboard to show the person's name in access logs.

**Architecture:** 
- Systematic string replacement for "RFID Dashboard" to "SafeKid".
- Backend: Update `AccessLog` schemas and use a SQL `JOIN` in the metrics router to fetch the person's name from the `Card` table.
- Frontend: Update TypeScript types and the `LogsTable` component to display the new `person_name` field.

**Tech Stack:** FastAPI, Vue 3, SQLAlchemy, Tailwind CSS.

---

### Task 1: Global Renaming to SafeKid (Documentation & Configuration)

**Files:**
- Modify: `README.md`
- Modify: `DESIGN.md`
- Modify: `docs/DASHBOARD_PROGRESS.md`
- Modify: `docs/superpowers/plans/MASTER_PLAN.md`
- Modify: `frontend/package.json`

- [ ] **Step 1: Update README.md**
  Replace `# Projeto: ESP32 + RC522 (RFID)` with `# SafeKid - ESP32 + RC522 (RFID)`.

- [ ] **Step 2: Update DESIGN.md**
  Replace `# RFID Dashboard - Design Document` with `# SafeKid - Design Document`.

- [ ] **Step 3: Update docs/DASHBOARD_PROGRESS.md**
  Replace `# RFID Dashboard Project: Plan & Progress` with `# SafeKid Project: Plan & Progress`.

- [ ] **Step 4: Update docs/superpowers/plans/MASTER_PLAN.md**
  Replace `# RFID Dashboard - Implementation Master Plan` with `# SafeKid - Implementation Master Plan`.

- [ ] **Step 5: Update frontend/package.json**
  Change `"name": "frontend"` to `"name": "safekid-frontend"`.

- [ ] **Step 6: Commit renaming changes**
```bash
git add README.md DESIGN.md docs/ frontend/package.json
git commit -m "docs: rename project to SafeKid in documentation and config"
```

---

### Task 2: Rename Application in Backend and Frontend Code

**Files:**
- Modify: `backend/main.py`
- Modify: `frontend/index.html`
- Modify: `frontend/src/components/AppNavbar.vue`

- [ ] **Step 1: Update Backend FastAPI Title**
  In `backend/main.py`, change `app = FastAPI(title="RFID Dashboard API", ...)` to `app = FastAPI(title="SafeKid API", ...)`.

- [ ] **Step 2: Update Frontend index.html Title**
  In `frontend/index.html`, change `<title>Vite App</title>` to `<title>SafeKid Dashboard</title>`.

- [ ] **Step 3: Update Frontend Navbar Title**
  In `frontend/src/components/AppNavbar.vue`, change `RFID Dashboard` to `SafeKid`.

- [ ] **Step 4: Verify naming changes**
  Run backend health check (if possible) or just check files.
  Run: `grep -r "RFID Dashboard" .` (should return minimal/no matches in code).

- [ ] **Step 5: Commit code renaming**
```bash
git add backend/main.py frontend/index.html frontend/src/components/AppNavbar.vue
git commit -m "feat: rename application to SafeKid in code"
```

---

### Task 3: Backend - Enhance Access Log Schema and Router

**Files:**
- Modify: `backend/schemas.py`
- Modify: `backend/routers/metrics.py`

- [ ] **Step 1: Add AccessLogResponse schema**
  Add the following to `backend/schemas.py`:
```python
class AccessLogResponse(BaseModel):
    id: int
    uid: str
    status: str
    timestamp: datetime
    person_name: Optional[str] = None

    class Config:
        from_attributes = True
```

- [ ] **Step 2: Update metrics router to return person_name via JOIN**
  Modify `backend/routers/metrics.py` to join with `Card` table.
```python
from fastapi import APIRouter, Depends
from sqlalchemy.orm import Session
from ..database import get_db
from ..models import AccessLog, Card
from ..schemas import AccessLogResponse
from typing import List

router = APIRouter(prefix="/api/metrics", tags=["metrics"])

@router.get("/logs", response_model=List[AccessLogResponse])
def get_logs(limit: int = 50, db: Session = Depends(get_db)):
    # Perform an outer join to include logs without a matching card (unknown UIDs)
    results = db.query(
        AccessLog.id,
        AccessLog.uid,
        AccessLog.status,
        AccessLog.timestamp,
        Card.name.label("person_name")
    ).outerjoin(
        Card, AccessLog.uid == Card.uid
    ).order_by(AccessLog.timestamp.desc()).limit(limit).all()
    
    return results
```

- [ ] **Step 3: Verify backend changes**
  If there is a test suite, run it. Otherwise, manually verify the API if possible.
  Run: `pytest tests/backend/test_api.py` (check if it needs updates).

- [ ] **Step 4: Commit backend log enhancement**
```bash
git add backend/schemas.py backend/routers/metrics.py
git commit -m "feat: enhance access logs with person_name via backend JOIN"
```

---

### Task 4: Frontend - Update Types and Display Log Name

**Files:**
- Modify: `frontend/src/types/index.ts`
- Modify: `frontend/src/components/LogsTable.vue`

- [ ] **Step 1: Update AccessLog interface**
  In `frontend/src/types/index.ts`, add `person_name?: string;` to `AccessLog`.

- [ ] **Step 2: Add Person column to LogsTable.vue**
  Modify `frontend/src/components/LogsTable.vue` to add the new column.
  - Add `th`: `<th class="p-4 font-semibold text-sm text-gray-600">Person</th>`
  - Add `td`: `<td class="p-4 text-sm">{{ log.person_name || 'Desconhecido' }}</td>`

- [ ] **Step 3: Verify frontend changes**
  Check `DashboardView.vue` to ensure it still renders correctly.
  Run: `cd frontend && bun type-check`

- [ ] **Step 4: Commit frontend changes**
```bash
git add frontend/src/types/index.ts frontend/src/components/LogsTable.vue
git commit -m "feat: display person name in logs table"
```

---

### Task 5: Final Validation

- [ ] **Step 1: Run full build and tests**
  - Backend: `pytest`
  - Frontend: `cd frontend && bun build`

- [ ] **Step 2: Final check of naming**
  `grep -ri "RFID Dashboard" .`
  `grep -ri "SafeKid" .`
