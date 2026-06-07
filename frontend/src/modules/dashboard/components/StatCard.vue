<script setup lang="ts">
import type { Component } from 'vue';

defineProps<{
  title: string;
  value: number | string;
  icon?: Component;
  colorClass?: string;
}>();
</script>

<template>
  <div class="bg-white p-6 rounded-xl shadow-sm border border-gray-100 flex items-center space-x-4">
    <div
      v-if="icon"
      :class="['p-3 rounded-lg', colorClass || 'bg-blue-50 text-blue-600']"
    >
      <component
        :is="icon"
        class="w-6 h-6"
      />
    </div>
    <div>
      <p class="text-sm font-medium text-gray-500">
        {{ title }}
      </p>
      <p class="text-2xl font-bold">
        {{ value }}
      </p>
    </div>
  </div>
</template>
