<script setup lang="ts">
import { computed, ref } from 'vue';
import { useTracking } from '@/composables/useTracking';
import AppNavbar from '@/components/AppNavbar.vue';
import { MapPin, Signal, Clock, User, Radio, Copy, Bug } from 'lucide-vue-next';

const { liveTracking, isLoadingLive, discovery, isLoadingDiscovery } = useTracking();
const showDebug = ref(false);

const sortedDiscovery = computed(() => {
  if (!discovery.value) return [];
  return Object.entries(discovery.value).map(([mac, data]) => ({
    mac,
    rssi: data.rssi,
    timestamp: data.timestamp
  })).sort((a, b) => b.rssi - a.rssi);
});

const copyToClipboard = (text: string) => {
  navigator.clipboard.writeText(text);
  alert('MAC copiado!');
};

const getZoneColor = (zone: string) => {
  switch (zone) {
    case 'Very Near': return 'text-green-500 bg-green-50 border-green-200';
    case 'Near': return 'text-blue-500 bg-blue-50 border-blue-200';
    case 'Far': return 'text-yellow-500 bg-yellow-50 border-yellow-200';
    default: return 'text-gray-400 bg-gray-50 border-gray-200';
  }
};
</script>

<template>
  <div class="min-h-screen pb-12 bg-slate-50/50">
    <AppNavbar />

    <main class="max-w-7xl mx-auto px-4 space-y-8">
      <header class="flex justify-between items-start">
        <div>
          <h1 class="text-2xl font-bold text-slate-800">Rastreamento em Tempo Real</h1>
          <p class="text-slate-500 text-sm">Localização aproximada das crianças via proximidade WiFi.</p>
        </div>
        <button @click="showDebug = !showDebug" 
                class="p-2 text-slate-400 hover:text-indigo-600 transition-colors">
          <Bug class="h-5 w-5" />
        </button>
      </header>

      <!-- Debug View -->
      <div v-if="showDebug" class="bg-slate-900 text-emerald-400 p-4 rounded-xl font-mono text-xs overflow-auto max-h-64 shadow-inner">
        <p class="mb-2 border-b border-emerald-900 pb-1 font-bold">// RAW DATA DEBUG</p>
        <p class="font-bold">Discovery ({{ sortedDiscovery.length }} items):</p>
        <pre>{{ JSON.stringify(discovery, null, 2) }}</pre>
        <p class="mt-4 font-bold">Live Tracking:</p>
        <pre>{{ JSON.stringify(liveTracking, null, 2) }}</pre>
      </div>

      <!-- Seção 1: Alunos Vinculados -->
      <section class="space-y-4">
        <div class="flex items-center gap-2">
          <User class="h-5 w-5 text-emerald-600" />
          <h2 class="text-lg font-bold text-slate-800">Alunos Monitorados</h2>
        </div>

        <div v-if="isLoadingLive" class="flex justify-center py-12">
          <div class="animate-spin rounded-full h-8 w-8 border-b-2 border-emerald-600"></div>
        </div>

        <div v-else-if="!liveTracking || liveTracking.length === 0" 
             class="bg-white p-12 rounded-2xl shadow-sm border border-slate-200 text-center">
          <MapPin class="h-12 w-12 text-slate-300 mx-auto mb-3" />
          <p class="text-slate-500 font-medium">Nenhum aluno com rastreador vinculado.</p>
          <p class="text-xs text-slate-400 mt-1">Vincule um MAC address no cadastro do aluno para começar a monitorar.</p>
        </div>

        <div v-else class="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-6">
          <div v-for="item in liveTracking" :key="item.mac" 
               class="bg-white p-5 rounded-2xl shadow-sm border border-slate-200 flex flex-col gap-4 hover:shadow-md transition-shadow">
            
            <div class="flex items-start justify-between">
              <div class="flex items-center gap-3">
                <div class="h-12 w-12 bg-emerald-100 rounded-full flex items-center justify-center text-emerald-600">
                  <User class="h-6 w-6" />
                </div>
                <div>
                  <h3 class="font-bold text-slate-800 leading-tight">{{ item.student_name }}</h3>
                  <p class="text-xs text-slate-400 font-mono tracking-tighter">{{ item.mac }}</p>
                </div>
              </div>
              <span :class="['px-2 py-0.5 rounded-full text-[10px] font-black uppercase tracking-wider border', item.status === 'online' ? 'bg-emerald-50 text-emerald-600 border-emerald-200' : 'bg-slate-50 text-slate-400 border-slate-200']">
                {{ item.status }}
              </span>
            </div>

            <div class="grid grid-cols-2 gap-3">
              <div :class="['p-3 rounded-xl border text-center transition-colors', getZoneColor(item.zone)]">
                <p class="text-[9px] uppercase font-black opacity-60 mb-0.5">Zona</p>
                <p class="font-bold text-sm">{{ item.zone }}</p>
              </div>
              <div class="p-3 bg-slate-50 rounded-xl border border-slate-100 text-center">
                <p class="text-[9px] uppercase font-black text-slate-400 mb-0.5">Sinal</p>
                <p class="font-bold text-sm text-slate-700">{{ item.rssi || '--' }} <span class="text-[10px] font-normal opacity-50">dBm</span></p>
              </div>
            </div>

            <div class="flex items-center gap-2 text-[10px] text-slate-400 mt-auto pt-3 border-t border-slate-50">
              <Clock class="h-3 w-3" />
              <span>Última atualização: {{ item.last_seen ? new Date(item.last_seen).toLocaleTimeString() : 'Nunca' }}</span>
            </div>
          </div>
        </div>
      </section>

      <!-- Seção 2: Descoberta -->
      <section class="space-y-4">
        <div class="flex items-center gap-2">
          <Radio class="h-5 w-5 text-indigo-600" />
          <h2 class="text-lg font-bold text-slate-800">Dispositivos Próximos (Novos)</h2>
        </div>

        <div v-if="isLoadingDiscovery" class="grid grid-cols-1 md:grid-cols-2 gap-3">
          <div v-for="i in 2" :key="i" class="h-16 bg-white animate-pulse rounded-xl border border-slate-100"></div>
        </div>
        
        <div v-else-if="sortedDiscovery.length === 0" class="p-8 bg-white/50 border-2 border-dashed border-slate-200 rounded-2xl text-center">
          <Signal class="h-8 w-8 text-slate-300 mx-auto mb-2 animate-bounce" />
          <p class="text-sm text-slate-500 italic">Buscando tags próximas...</p>
          <p class="text-[10px] text-slate-400 mt-1">Ligue um Wemos para vê-lo aparecer aqui.</p>
        </div>

        <div v-else class="grid grid-cols-1 md:grid-cols-2 gap-4">
          <div v-for="tag in sortedDiscovery" :key="tag.mac" 
               class="bg-white p-4 rounded-2xl shadow-sm border border-slate-200 flex items-center justify-between group hover:border-indigo-300 transition-all">
            <div class="flex items-center gap-4">
              <div class="p-2 bg-indigo-50 rounded-lg group-hover:bg-indigo-100 transition-colors text-indigo-600">
                <Signal class="h-5 w-5" />
              </div>
              <div class="flex flex-col">
                <span class="text-sm font-mono font-black text-slate-700">{{ tag.mac }}</span>
                <div class="flex items-center gap-2">
                  <span class="text-[10px] font-bold text-indigo-500 uppercase leading-none">{{ tag.rssi }} dBm</span>
                  <span class="text-[10px] text-slate-300">•</span>
                  <span class="text-[10px] text-slate-400 font-semibold uppercase leading-none">{{ new Date(tag.timestamp).toLocaleTimeString() }}</span>
                </div>
              </div>
            </div>
            <button @click="copyToClipboard(tag.mac)" 
                    class="flex items-center gap-2 px-4 py-2 bg-indigo-50 text-indigo-600 rounded-xl text-xs font-bold hover:bg-indigo-600 hover:text-white transition-all">
              <Copy class="h-3.5 w-3.5" />
              Copiar
            </button>
          </div>
        </div>
      </section>

      <!-- Seção 3: Como Funciona -->
      <div class="bg-indigo-600 rounded-3xl p-8 text-white relative overflow-hidden shadow-xl shadow-indigo-200">
        <Signal class="absolute -right-8 -bottom-8 h-48 w-48 opacity-10 rotate-12" />
        <h4 class="text-xl font-bold mb-3 flex items-center gap-2">
          <Signal class="h-5 w-5" /> Entendendo as Zonas
        </h4>
        <div class="grid grid-cols-1 sm:grid-cols-3 gap-6 relative z-10">
          <div>
            <p class="font-black text-emerald-300 uppercase text-xs mb-1">Very Near</p>
            <p class="text-sm opacity-90 leading-relaxed text-indigo-50">Sinal forte (>-55dBm). A criança está muito próxima do scanner (mesmo ambiente).</p>
          </div>
          <div>
            <p class="font-black text-blue-300 uppercase text-xs mb-1">Near</p>
            <p class="text-sm opacity-90 leading-relaxed text-indigo-50">Sinal médio (>-75dBm). A criança está nos arredores ou em uma sala vizinha.</p>
          </div>
          <div>
            <p class="font-black text-yellow-300 uppercase text-xs mb-1">Far</p>
            <p class="text-sm opacity-90 leading-relaxed text-indigo-50">Sinal fraco. A criança está se afastando ou há muitos obstáculos no caminho.</p>
          </div>
        </div>
      </div>
    </main>
  </div>
</template>
