export * from './constants'
export const trackingRoutes = [
  {
    path: '/tracking',
    name: 'tracking',
    component: () => import('./views/TrackingView.vue'),
  },
]

export { useDiscoveryQuery } from './composables/useDiscoveryQuery'
