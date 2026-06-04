<script setup lang="ts">
import { defineProps } from 'vue';
import type { AccessLog } from '../types';

defineProps<{
  logs: AccessLog[];
}>();

const formatDate = (dateStr: string) => {
  return new Date(dateStr).toLocaleString();
};

const getStatusClass = (status: string) => {
  switch (status) {
    case 'authorized': return 'bg-green-100 text-green-700';
    case 'inactive': return 'bg-yellow-100 text-yellow-700';
    case 'unknown': return 'bg-red-100 text-red-700';
    default: return 'bg-gray-100 text-gray-700';
  }
};
</script>

<template>
  <div class="overflow-x-auto bg-white rounded-xl shadow-sm border border-gray-100">
    <table class="w-full text-left border-collapse">
      <thead>
        <tr class="bg-gray-50 border-b border-gray-100">
          <th class="p-4 font-semibold text-sm text-gray-600">Timestamp</th>
          <th class="p-4 font-semibold text-sm text-gray-600">UID</th>
          <th class="p-4 font-semibold text-sm text-gray-600">Status</th>
        </tr>
      </thead>
      <tbody>
        <tr v-for="log in logs" :key="log.id" class="border-b border-gray-50 last:border-0 hover:bg-gray-50 transition-colors">
          <td class="p-4 text-sm">{{ formatDate(log.timestamp) }}</td>
          <td class="p-4 text-sm font-mono text-gray-500">{{ log.uid }}</td>
          <td class="p-4 text-sm">
            <span :class="['px-2.5 py-1 rounded-full text-xs font-medium capitalize', getStatusClass(log.status)]">
              {{ log.status }}
            </span>
          </td>
        </tr>
      </tbody>
    </table>
  </div>
</template>
