# SafeKid Frontend Architecture

## Overview
SafeKid follows an **Idiomatic Vue Feature Modules** architecture. This design groups files by **feature domain** rather than by file type, ensuring strict separation of concerns, easier maintenance, and high scalability.

## Directory Structure

```text
src/
├── infrastructure/    # Foundational, low-level configuration (e.g., API client)
├── modules/           # Feature-based domains (Cards, Tracking, etc.)
│   └── [module]/      # Each module is a self-contained domain
│       ├── index.ts   # Public Interface (Facade)
│       ├── views/     # Route components
│       ├── components/# Internal UI components
│       ├── services/  # Raw API calls (Axios)
│       └── composables/# Business logic & state (Vue Query)
├── router/            # Centralized routing configuration
└── shared/            # Reusable UI components and global types
```

---

## The Public Interface Pattern (Facade)
To prevent tight coupling and maintainable boundaries, every module MUST expose an `index.ts` at its root.

### 1. Encapsulation
The `index.ts` file is the **only** entry point for other modules. It should only export:
- Module routes (to be consumed by the central router).
- Composables and types intended for cross-module use.
- Publicly available components.

### 2. Automated Enforcement (ESLint)
We use `eslint-plugin-boundaries` to strictly forbid "deep imports". 
- **Valid**: `import { useCards } from '@/modules/cards';`
- **Invalid**: `import { useCards } from '@/modules/cards/composables/useCards';` (Raises ESLint Error)

This ensures you can refactor the internal folder structure of a module without breaking the rest of the application, as long as the `index.ts` remains stable.

---

## Architectural Guardrails

We use a "layered defense" strategy to keep the codebase clean:

| Tool | Responsibility | Enforcement |
| :--- | :--- | :--- |
| **ESLint** | Code Style & Boundaries | Real-time editor feedback; prevents deep cross-module imports. |
| **TypeScript** | Type Safety | Prevents runtime errors; strictly forbids the use of `any`. |
| **Fallow** | Structural Health | Detects **Circular Dependencies** and **Dead Code** via `bunx fallow dead-code`. |
| **Oxlint** | Performance | Ultra-fast linting for common pitfalls. |

---

## Data Fetching Pattern (Vue Query + Services)
We enforce a strict separation of fetch logic from state management:

1.  **Service (`*Service.ts`)**: Pure async functions that handle Axios requests. No Vue state allowed.
2.  **Composable (`use*.ts`)**: TanStack Query hooks that handle reactivity and caching.
3.  **Component (`*.vue`)**: Consumes the composable.

### Composable Rules:
- **One Operation Per Composable**: Every `useQuery` or `useMutation` hook must reside in its own dedicated file.
- **Naming**: Queries use `use[Resource].ts` (e.g., `useCards.ts`). Mutations use `use[Action][Resource].ts` (e.g., `useCreateCard.ts`).
- **Reactivity**: Composables must accept `MaybeRefOrGetter` for parameters to ensure automatic re-fetching when values change.

---

## Circular Dependency Management
Circular dependencies (A -> B -> A) are the primary cause of architectural decay and undefined runtime behavior. 

- **Detection**: Run `bunx fallow dead-code --circular-deps` to identify cycles.
- **Prevention**: If two modules need to share logic, **hoist it** to `src/shared/`. Never import from `module A` to `module B` if `module B` already imports from `module A`.

---

## Maintenance Guidelines

1.  **Strict Typing**: Always define interfaces in `src/shared/types/index.ts` or module-specific type files. The use of `any` is prohibited.
2.  **Pre-Commit Check**: Before pushing code, ensure both linting and type-checking pass:
    ```bash
    bun run lint:eslint && bun run type-check
    ```
3.  **Fallow Health**: Periodically run `npx fallow health` to monitor complexity and maintain a Grade A health score.
