export * from './constants'
export const scannersRoutes = [
  {
    path: '/scanners',
    name: 'scanners',
    component: () => import('./views/ScannersView.vue'),
  },
]
