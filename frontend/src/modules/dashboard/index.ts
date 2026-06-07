export * from './constants'
import DashboardView from './views/DashboardView.vue'

export const dashboardRoutes = [
  {
    path: '/',
    name: 'dashboard',
    component: DashboardView,
  },
]
