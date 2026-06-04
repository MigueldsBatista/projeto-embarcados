<script setup lang="ts">
import { ref, watch } from 'vue';
import { X } from 'lucide-vue-next';
import type { Card } from '../types';

const props = defineProps<{
  show: boolean;
  editingCard?: Card | null;
  loading?: boolean;
}>();

const emit = defineEmits<{
  (e: 'close'): void;
  (e: 'submit', data: Partial<Card>): void;
}>();

const form = ref({
  uid: '',
  name: '',
  role: 'user',
  is_active: true,
});

watch(() => props.editingCard, (card) => {
  if (card) {
    form.value = {
      uid: card.uid,
      name: card.name,
      role: card.role,
      is_active: card.is_active,
    };
  } else {
    form.value = {
      uid: '',
      name: '',
      role: 'user',
      is_active: true,
    };
  }
}, { immediate: true });

const handleSubmit = () => {
  emit('submit', { ...form.value });
};
</script>

<template>
  <div v-if="show" class="fixed inset-0 z-50 flex items-center justify-center p-4 bg-black/50 backdrop-blur-sm">
    <div class="bg-white rounded-2xl shadow-xl w-full max-w-md overflow-hidden animate-in fade-in zoom-in duration-200">
      <div class="p-6 border-b border-gray-100 flex justify-between items-center">
        <h2 class="text-xl font-bold text-gray-900">
          {{ editingCard ? 'Edit Card' : 'Add New Card' }}
        </h2>
        <button @click="emit('close')" class="text-gray-400 hover:text-gray-600">
          <X class="w-6 h-6" />
        </button>
      </div>

      <form @submit.prevent="handleSubmit" class="p-6 space-y-4">
        <div>
          <label class="block text-sm font-medium text-gray-700 mb-1">UID</label>
          <input v-model="form.uid" type="text" required 
                 :disabled="!!editingCard"
                 class="w-full px-4 py-2 border border-gray-200 rounded-lg focus:ring-2 focus:ring-blue-500 focus:border-transparent outline-none disabled:bg-gray-50 disabled:text-gray-500 font-mono"
                 placeholder="e.g. 1A 2B 3C 4D" />
        </div>

        <div>
          <label class="block text-sm font-medium text-gray-700 mb-1">Name</label>
          <input v-model="form.name" type="text" required 
                 class="w-full px-4 py-2 border border-gray-200 rounded-lg focus:ring-2 focus:ring-blue-500 focus:border-transparent outline-none"
                 placeholder="e.g. John Doe" />
        </div>

        <div>
          <label class="block text-sm font-medium text-gray-700 mb-1">Role</label>
          <select v-model="form.role" 
                  class="w-full px-4 py-2 border border-gray-200 rounded-lg focus:ring-2 focus:ring-blue-500 focus:border-transparent outline-none bg-white">
            <option value="user">User</option>
            <option value="admin">Admin</option>
          </select>
        </div>

        <div class="flex items-center space-x-2">
          <input v-model="form.is_active" type="checkbox" id="is_active" 
                 class="w-4 h-4 text-blue-600 border-gray-300 rounded focus:ring-blue-500" />
          <label for="is_active" class="text-sm font-medium text-gray-700">Active</label>
        </div>

        <div class="pt-4 flex space-x-3">
          <button type="button" @click="emit('close')" 
                  class="flex-1 px-4 py-2 border border-gray-200 text-gray-600 rounded-lg hover:bg-gray-50 transition-colors">
            Cancel
          </button>
          <button type="submit" :disabled="loading"
                  class="flex-1 px-4 py-2 bg-blue-600 text-white rounded-lg hover:bg-blue-700 transition-colors disabled:opacity-50">
            {{ loading ? 'Saving...' : (editingCard ? 'Update' : 'Register') }}
          </button>
        </div>
      </form>
    </div>
  </div>
</template>
