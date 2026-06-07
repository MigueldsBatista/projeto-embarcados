export * from './constants'
export const cardsRoutes = [
  {
    path: '/cards',
    name: 'cards',
    component: () => import('./views/CardsView.vue'),
  },
]

export { useCardsQuery } from './composables/useCardsQuery'
