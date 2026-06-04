<script setup lang="ts">
import { Cpu } from 'lucide-vue-next';
</script>

<template>
  <nav class="bg-white border-b border-gray-100 py-4 mb-8">
    <div class="max-w-7xl mx-auto px-4 flex items-center justify-between">
      <div class="flex items-center space-x-2">
        <div class="bg-blue-600 p-1.5 rounded-lg text-white">
          <Cpu class="w-5 h-5" />
        </div>
        <span class="text-xl font-bold tracking-tight">RFID Dashboard</span>
      </div>
      <div class="flex items-center space-x-6">
        <a href="#" class="text-sm font-medium text-gray-600 hover:text-blue-600 transition-colors">Dashboard</a>
        <a href="#" class="text-sm font-medium text-gray-600 hover:text-blue-600 transition-colors">Cards</a>
        <a href="#" class="text-sm font-medium text-gray-600 hover:text-blue-600 transition-colors">Settings</a>
      </div>
    </div>
  </nav>
</template>
