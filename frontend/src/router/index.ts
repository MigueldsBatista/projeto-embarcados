import { createRouter, createWebHistory } from 'vue-router'
import DashboardView from '../views/DashboardView.vue'

const router = createRouter({
  history: createWebHistory(import.meta.env.BASE_URL),
  routes: [
    {
      path: '/',
      name: 'dashboard',
      component: DashboardView,
    },
    {
      path: '/cards',
      name: 'cards',
      component: () => import('../views/CardsView.vue'),
    },
    {
      path: '/tracking',
      name: 'tracking',
      component: () => import('../views/TrackingView.vue'),
    },
    {
      path: '/scanners',
      name: 'scanners',
      component: () => import('../views/ScannersView.vue'),
    },
  ],
})

export default router
