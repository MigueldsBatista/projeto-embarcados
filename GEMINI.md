# Projeto Embarcados - Architectural Guidelines

## Frontend Modular Architecture

The frontend follows an **Idiomatic Vue Feature Modules** architecture to ensure strict separation of concerns, easier maintenance, and better scalability. For a deep dive, see `frontend/ARCHITECTURE.md`.

### Directory Structure
```text
src/
├── infrastructure/    # Foundational, low-level configuration (API client, error handlers)
├── modules/           # Feature-based domains (cards, dashboard, scanners, tracking)
└── shared/            # Cross-domain resources (Navbar, global types)
```

### Module Boundaries
Each feature module (`src/modules/*`) is self-contained and consists of:
- **`views/`**: Route components.
- **`components/`**: Feature-specific components.
- **`services/`**: Pure async functions for HTTP requests. **The only place for Axios.**
- **`composables/`**: Vue Query hooks. **Must not contain raw Axios calls.**

### Data Fetching (Vue Query + Services)
1. **Service**: Handles the API request (e.g., `cardsService.ts`).
2. **Composable**: Wraps the service in TanStack Query for caching.
3. **Component**: Consumes the composable.

**Composable Rules:**
1. **One Operation Per Composable**: One `useQuery`/`useMutation` per file.
2. **Naming Convention**: `use[Resource].ts` or `use[Action][Resource].ts`.
3. **Reactive Parameters**: Use `MaybeRefOrGetter` for dynamic IDs/MACs.

### Strict Typing & Linting
- **No `any` allowed** (`@typescript-eslint/no-explicit-any` is an error).
- **Import Sorting**: Enforced via `eslint-plugin-simple-import-sort`. Order: Vue ecosystem -> Third-party -> `@/` alias -> Relative paths.
- Explicitly stringify numbers in template literals where needed.
- Await `invalidateQueries` to avoid floating promises.
- Use `fallow health` to keep cyclomatic complexity low.

