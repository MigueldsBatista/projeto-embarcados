# Projeto Embarcados - Architectural Guidelines

## Frontend Patterns

### Composables and Vue Query
To maintain a clean and maintainable codebase, we follow a strict single-responsibility pattern for our Vue Query composables:

1. **One Operation Per Composable**: Every `useQuery` and `useMutation` hook must reside in its own dedicated composable file.
2. **Naming Convention**: 
   - Queries: `use[Resource]Query.ts` or simply `use[Resource].ts` (e.g., `useCards.ts`, `useSummary.ts`).
   - Mutations: `use[Action][Resource].ts` (e.g., `useCreateCard.ts`, `useDeleteCard.ts`).
3. **Reactive Parameters**: Composables that depend on parameters (like IDs or MAC addresses) should accept `MaybeRefOrGetter` to ensure the query remains reactive and automatically re-fetches when dependencies change.

This pattern avoids "God" composables that combine multiple unrelated queries, simplifying testing and reusability.
