<script setup lang="ts">
import { Edit2, Power, Shield, Trash2, User, Wifi } from 'lucide-vue-next';

import type { Card } from '@/shared';

defineProps<{
  cards: Card[];
}>();

const emit = defineEmits<{
  (e: 'edit' | 'toggle', card: Card): void;
  (e: 'delete', uid: string): void;
}>();

const getRoleClass = (role: string) => {
  return role === 'admin' ? 'bg-purple-100 text-purple-700' : 'bg-blue-100 text-blue-700';
};
</script>

<template>
  <div class="overflow-x-auto bg-white rounded-xl shadow-sm border border-gray-100">
    <table class="w-full text-left border-collapse">
      <thead>
        <tr class="bg-gray-50 border-b border-gray-100">
          <th class="p-4 font-semibold text-sm text-gray-600">
            Card Info
          </th>
          <th class="p-4 font-semibold text-sm text-gray-600 text-center">
            Tracker
          </th>
          <th class="p-4 font-semibold text-sm text-gray-600">
            Role
          </th>
          <th class="p-4 font-semibold text-sm text-gray-600">
            Status
          </th>
          <th class="p-4 font-semibold text-sm text-gray-600 text-right">
            Actions
          </th>
        </tr>
      </thead>
      <tbody>
        <tr
          v-for="card in cards"
          :key="card.id"
          class="border-b border-gray-50 last:border-0 hover:bg-gray-50 transition-colors"
        >
          <td class="p-4">
            <div class="flex flex-col">
              <span class="text-sm font-medium text-gray-900">{{ card.name }}</span>
              <span class="text-xs text-gray-500 font-mono">{{ card.uid }}</span>
            </div>
          </td>
          <td class="p-4 text-center">
            <div
              v-if="card.tracker_mac"
              class="flex flex-col items-center justify-center text-blue-600"
              :title="`MAC: ${card.tracker_mac}`"
            >
              <Wifi class="w-4 h-4" />
              <span class="text-[9px] font-mono mt-0.5">{{ card.tracker_mac.split(':').slice(-2).join(':') }}</span>
            </div>
            <span
              v-else
              class="text-gray-300 text-xs"
            >—</span>
          </td>
          <td class="p-4">
            <span :class="['inline-flex items-center px-2.5 py-1 rounded-full text-xs font-medium capitalize', getRoleClass(card.role)]">
              <component
                :is="card.role === 'admin' ? Shield : User"
                class="w-3 h-3 mr-1"
              />
              {{ card.role }}
            </span>
          </td>
          <td class="p-4 text-sm">
            <span :class="['px-2.5 py-1 rounded-full text-xs font-medium', card.is_active ? 'bg-green-100 text-green-700' : 'bg-red-100 text-red-700']">
              {{ card.is_active ? 'Active' : 'Inactive' }}
            </span>
          </td>
          <td class="p-4 text-right">
            <div class="flex justify-end space-x-2">
              <button
                class="p-1.5 text-gray-400 hover:text-blue-600 hover:bg-blue-50 rounded-lg transition-colors" 
                :title="card.is_active ? 'Deactivate' : 'Activate'"
                @click="emit('toggle', card)"
              >
                <Power class="w-4 h-4" />
              </button>
              <button
                class="p-1.5 text-gray-400 hover:text-amber-600 hover:bg-amber-50 rounded-lg transition-colors" 
                title="Edit"
                @click="emit('edit', card)"
              >
                <Edit2 class="w-4 h-4" />
              </button>
              <button
                class="p-1.5 text-gray-400 hover:text-red-600 hover:bg-red-50 rounded-lg transition-colors" 
                title="Delete"
                @click="emit('delete', card.uid)"
              >
                <Trash2 class="w-4 h-4" />
              </button>
            </div>
          </td>
        </tr>
      </tbody>
    </table>
  </div>
</template>
