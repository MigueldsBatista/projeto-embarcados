<script setup lang="ts">
import { ref, watch, computed } from 'vue';
import { X, Wifi, Search } from 'lucide-vue-next';
import type { Card } from '../types';
import { useTracking } from '../composables/useTracking';

const props = defineProps<{
  show: boolean;
  editingCard?: Card | null;
  loading?: boolean;
}>();

const emit = defineEmits<{
  (e: 'close'): void;
  (e: 'submit', data: Partial<Card>): void;
}>();

const { discovery } = useTracking();

const form = ref({
  uid: '',
  name: '',
  role: 'user',
  is_active: true,
  tracker_mac: '',
});

// Transforma o objeto de descoberta em uma lista para o select
const discoveredOptions = computed(() => {
  if (!discovery.value) return [];
  return Object.entries(discovery.value).map(([mac, data]) => ({
    mac,
    rssi: data.rssi,
    timestamp: data.timestamp
  })).sort((a, b) => b.rssi - a.rssi); // Mostra os mais perto primeiro
});

watch(() => props.editingCard, (card) => {
  if (card) {
    form.value = {
      uid: card.uid,
      name: card.name,
      role: card.role,
      is_active: card.is_active,
      tracker_mac: card.tracker_mac || '',
    };
  } else {
    form.value = {
      uid: '',
      name: '',
      role: 'user',
      is_active: true,
      tracker_mac: '',
    };
  }
}, { immediate: true });

const handleSubmit = () => {
  emit('submit', { ...form.value });
};
</script>

<template>
  <div v-if="show" class="fixed inset-0 z-50 flex items-center justify-center p-4 bg-black/50 backdrop-blur-sm overflow-y-auto">
    <div class="bg-white rounded-2xl shadow-xl w-full max-w-md overflow-hidden animate-in fade-in zoom-in duration-200 my-auto">
      <div class="p-6 border-b border-gray-100 flex justify-between items-center">
        <h2 class="text-xl font-bold text-gray-900">
          {{ editingCard ? 'Editar Aluno' : 'Novo Cadastro' }}
        </h2>
        <button @click="emit('close')" class="text-gray-400 hover:text-gray-600">
          <X class="w-6 h-6" />
        </button>
      </div>

      <form @submit.prevent="handleSubmit" class="p-6 space-y-4">
        <div>
          <label class="block text-sm font-medium text-gray-700 mb-1">UID do Cartão RFID</label>
          <input v-model="form.uid" type="text" required 
                 :disabled="!!editingCard"
                 class="w-full px-4 py-2 border border-gray-200 rounded-lg focus:ring-2 focus:ring-blue-500 focus:border-transparent outline-none disabled:bg-gray-50 disabled:text-gray-500 font-mono"
                 placeholder="e.g. 1A 2B 3C 4D" />
        </div>

        <div>
          <label class="block text-sm font-medium text-gray-700 mb-1">Nome da Criança</label>
          <input v-model="form.name" type="text" required 
                 class="w-full px-4 py-2 border border-gray-200 rounded-lg focus:ring-2 focus:ring-blue-500 focus:border-transparent outline-none"
                 placeholder="e.g. Enzo Silva" />
        </div>

        <div class="p-4 bg-slate-50 border border-slate-100 rounded-xl space-y-3">
          <div class="flex items-center gap-2 text-sm font-bold text-slate-700">
            <Wifi class="h-4 w-4 text-blue-500" />
            Vincular Tag WiFi (Rastreador)
          </div>
          
          <div>
            <label class="block text-xs font-medium text-slate-500 mb-1 uppercase tracking-wider">MAC do Wemos Próximo</label>
            <div class="relative">
              <select v-model="form.tracker_mac" 
                      class="w-full px-4 py-2 pr-10 border border-gray-200 rounded-lg focus:ring-2 focus:ring-blue-500 outline-none bg-white appearance-none">
                <option value="">Nenhum rastreador vinculado</option>
                <option v-for="tag in discoveredOptions" :key="tag.mac" :value="tag.mac">
                  {{ tag.mac }} (Sinal: {{ tag.rssi }} dBm)
                </option>
              </select>
              <div class="absolute right-3 top-2.5 pointer-events-none">
                <Search v-if="discoveredOptions.length === 0" class="h-4 w-4 text-slate-300 animate-pulse" />
              </div>
            </div>
            <p v-if="discoveredOptions.length === 0" class="text-[10px] text-slate-400 mt-1 italic">
              Ligue o Wemos da criança agora para ele aparecer na lista...
            </p>
          </div>
        </div>

        <div class="grid grid-cols-2 gap-4">
          <div>
            <label class="block text-sm font-medium text-gray-700 mb-1">Role</label>
            <select v-model="form.role" 
                    class="w-full px-4 py-2 border border-gray-200 rounded-lg focus:ring-2 focus:ring-blue-500 outline-none bg-white">
              <option value="user">User</option>
              <option value="admin">Admin</option>
            </select>
          </div>

          <div class="flex items-center space-x-2 pt-6">
            <input v-model="form.is_active" type="checkbox" id="is_active" 
                   class="w-4 h-4 text-blue-600 border-gray-300 rounded focus:ring-blue-500" />
            <label for="is_active" class="text-sm font-medium text-gray-700">Ativo</label>
          </div>
        </div>

        <div class="pt-4 flex space-x-3">
          <button type="button" @click="emit('close')" 
                  class="flex-1 px-4 py-2 border border-gray-200 text-gray-600 rounded-lg hover:bg-gray-50 transition-colors">
            Cancelar
          </button>
          <button type="submit" :disabled="loading"
                  class="flex-1 px-4 py-2 bg-blue-600 text-white rounded-lg hover:bg-blue-700 transition-colors disabled:opacity-50">
            {{ loading ? 'Salvando...' : (editingCard ? 'Atualizar' : 'Cadastrar') }}
          </button>
        </div>
      </form>
    </div>
  </div>
</template>
