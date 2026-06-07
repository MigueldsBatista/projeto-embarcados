import { createApp } from 'vue'

import { QueryClient, VueQueryPlugin } from '@tanstack/vue-query'

import App from './App.vue'
import router from './router'

import './assets/main.css'

const queryClient = new QueryClient()
const app = createApp(App)

app.use(VueQueryPlugin, { queryClient })
app.use(router)

app.mount('#app')
