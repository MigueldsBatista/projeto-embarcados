<script setup lang="ts">
import { ref } from 'vue';

import { Check, Clock, Edit2, Loader2,Radio, X } from 'lucide-vue-next';

import { AppNavbar, type Scanner } from '@/shared';

import { useScannersQuery } from '../composables/useScannersQuery';
import { useUpdateScannerMutation } from '../composables/useUpdateScannerMutation';

const { data: scanners, isLoading } = useScannersQuery();
const updateMutation = useUpdateScannerMutation();

const editingId = ref<string | null>(null);
const editingName = ref('');

const startEdit = (scanner: Scanner) => {
  editingId.value = scanner.identifier;
  editingName.value = scanner.name;
};

const cancelEdit = () => {
  editingId.value = null;
};

const saveEdit = (id: string) => {
  updateMutation.mutate({ id, name: editingName.value }, {
    onSuccess: () => {
      editingId.value = null;
    }
  });
};
</script>

<template>
  <div class="min-h-screen pb-12 bg-slate-50/50">
    <AppNavbar />

    <main class="max-w-7xl mx-auto px-4 space-y-8">
      <div>
        <h1 class="text-2xl font-bold text-slate-800">
          Manage Scanners
        </h1>
        <p class="text-slate-500 text-sm">
          Configure monitoring points (ESP32) spread throughout the school.
        </p>
      </div>

      <div
        v-if="isLoading"
        class="flex justify-center py-24"
      >
        <Loader2 class="h-12 w-12 text-indigo-600 animate-spin" />
      </div>

      <div
        v-else
        class="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-6"
      >
        <div
          v-for="scanner in scanners"
          :key="scanner.id" 
          class="bg-white p-6 rounded-3xl border border-slate-200 shadow-sm hover:shadow-md transition-all"
        >
          <div class="flex items-start justify-between mb-4">
            <div class="p-3 bg-indigo-50 rounded-2xl text-indigo-600">
              <Radio class="h-6 w-6" />
            </div>
            <div class="text-right">
              <span class="text-[10px] font-black uppercase text-slate-400 block mb-1">Technical ID</span>
              <code class="text-xs font-mono bg-slate-100 px-2 py-1 rounded-lg text-slate-600">{{ scanner.identifier }}</code>
            </div>
          </div>

          <div
            v-if="editingId === scanner.identifier"
            class="space-y-3"
          >
            <input
              v-model="editingName" 
              class="w-full px-4 py-2 border-2 border-indigo-500 rounded-xl outline-none"
              @keyup.enter="saveEdit(scanner.identifier)"
            >
            <div class="flex gap-2">
              <button
                class="flex-1 bg-indigo-600 text-white py-2 rounded-xl text-sm font-bold flex items-center justify-center gap-2" 
                @click="saveEdit(scanner.identifier)"
              >
                <Check class="h-4 w-4" /> Save
              </button>
              <button
                class="px-4 py-2 bg-slate-100 text-slate-600 rounded-xl text-sm font-bold" 
                @click="cancelEdit"
              >
                <X class="h-4 w-4" />
              </button>
            </div>
          </div>
          
          <div v-else>
            <h3 class="text-lg font-bold text-slate-800 mb-1">
              {{ scanner.name }}
            </h3>
            <button
              class="text-xs text-indigo-600 font-bold flex items-center gap-1 hover:underline"
              @click="startEdit(scanner)"
            >
              <Edit2 class="h-3 w-3" /> Change name
            </button>
          </div>

          <div class="mt-6 pt-4 border-t border-slate-50 flex items-center gap-2 text-[10px] text-slate-400">
            <Clock class="h-3 w-3" />
            <span>Last seen: {{ scanner.last_seen ? new Date(scanner.last_seen).toLocaleString() : 'Never' }}</span>
          </div>
        </div>
      </div>

      <div
        v-if="!isLoading && scanners?.length === 0"
        class="text-center py-24 bg-white rounded-3xl border-2 border-dashed border-slate-200"
      >
        <Radio class="h-12 w-12 text-slate-300 mx-auto mb-4" />
        <p class="text-slate-500 font-bold">
          No scanners detected.
        </p>
        <p class="text-sm text-slate-400">
          Scanners will appear here automatically as soon as they detect a tag for the first time.
        </p>
      </div>
    </main>
  </div>
</template>
