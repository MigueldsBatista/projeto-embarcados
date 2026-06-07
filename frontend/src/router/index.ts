import { createRouter, createWebHistory } from 'vue-router'

import { cardsRoutes } from '@/modules/cards'
import { dashboardRoutes } from '@/modules/dashboard'
import { scannersRoutes } from '@/modules/scanners'
import { trackingRoutes } from '@/modules/tracking'

const router = createRouter({
  history: createWebHistory(import.meta.env.BASE_URL),
  routes: [
    ...dashboardRoutes,
    ...cardsRoutes,
    ...trackingRoutes,
    ...scannersRoutes,
  ],
})

export default router
