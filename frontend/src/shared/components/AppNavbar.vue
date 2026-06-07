<script setup lang="ts">
import { RouterLink } from 'vue-router';

import { Cpu } from 'lucide-vue-next';
</script>

<template>
  <nav class="bg-white border-b border-gray-100 py-4 mb-8">
    <div class="max-w-7xl mx-auto px-4 flex items-center justify-between">
      <RouterLink
        to="/"
        class="flex items-center space-x-2"
      >
        <div class="bg-blue-600 p-1.5 rounded-lg text-white">
          <Cpu class="w-5 h-5" />
        </div>
        <span class="text-xl font-bold tracking-tight">SafeKid</span>
      </RouterLink>
      <div class="flex items-center space-x-6">
        <RouterLink
          to="/"
          class="text-sm font-medium text-gray-600 hover:text-blue-600 transition-colors"
          active-class="text-blue-600"
        >
          Dashboard
        </RouterLink>
        <RouterLink
          to="/cards"
          class="text-sm font-medium text-gray-600 hover:text-blue-600 transition-colors"
          active-class="text-blue-600"
        >
          Cards
        </RouterLink>
        <RouterLink
          to="/tracking"
          class="text-sm font-medium text-gray-600 hover:text-blue-600 transition-colors"
          active-class="text-blue-600"
        >
          Tracking
        </RouterLink>
        <RouterLink
          to="/scanners"
          class="text-sm font-medium text-gray-600 hover:text-blue-600 transition-colors"
          active-class="text-blue-600"
        >
          Scanners
        </RouterLink>
        <a
          href="#"
          class="text-sm font-medium text-gray-600 hover:text-blue-600 transition-colors"
        >Settings</a>
      </div>
    </div>
  </nav>
</template>
