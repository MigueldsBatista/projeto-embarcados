<script setup lang="ts">
import { useTracking } from '@/composables/useTracking';
import { MapPin, Signal, Clock, User } from 'lucide-vue-next';

const { liveTracking, isLoadingLive } = useTracking();

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
  <div class="space-y-6">
    <header>
      <h1 class="text-2xl font-bold text-slate-800">Rastreamento em Tempo Real</h1>
      <p class="text-slate-500">Localização aproximada das crianças via proximidade WiFi.</p>
    </header>

    <div v-if="isLoadingLive" class="flex justify-center py-12">
      <div class="animate-spin rounded-full h-8 w-8 border-b-2 border-indigo-600"></div>
    </div>

    <div v-else-if="liveTracking?.length === 0" class="bg-white p-8 rounded-xl shadow-sm border border-slate-200 text-center">
      <MapPin class="h-12 w-12 text-slate-300 mx-auto mb-3" />
      <p class="text-slate-500">Nenhuma criança com rastreador vinculada.</p>
    </div>

    <div v-else class="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-4">
      <div v-for="item in liveTracking" :key="item.mac" 
           class="bg-white p-5 rounded-xl shadow-sm border border-slate-200 flex flex-col gap-4">
        
        <div class="flex items-start justify-between">
          <div class="flex items-center gap-3">
            <div class="h-10 w-10 bg-indigo-100 rounded-full flex items-center justify-center text-indigo-600">
              <User class="h-5 w-5" />
            </div>
            <div>
              <h3 class="font-semibold text-slate-800">{{ item.student_name }}</h3>
              <p class="text-xs text-slate-400 font-mono">{{ item.mac }}</p>
            </div>
          </div>
          <span :class="['px-2 py-1 rounded text-xs font-medium border', item.status === 'online' ? 'bg-emerald-50 text-emerald-600 border-emerald-200' : 'bg-slate-50 text-slate-400 border-slate-200']">
            {{ item.status.toUpperCase() }}
          </span>
        </div>

        <div class="grid grid-cols-2 gap-2">
          <div :class="['p-3 rounded-lg border text-center', getZoneColor(item.zone)]">
            <p class="text-[10px] uppercase font-bold opacity-70 mb-1">Zona Atual</p>
            <p class="font-bold">{{ item.zone }}</p>
          </div>
          <div class="p-3 bg-slate-50 rounded-lg border border-slate-200 text-center">
            <p class="text-[10px] uppercase font-bold text-slate-400 mb-1">Sinal (RSSI)</p>
            <p class="font-bold text-slate-700">{{ item.rssi || '--' }} dBm</p>
          </div>
        </div>

        <div class="flex items-center gap-2 text-xs text-slate-400 mt-auto pt-2 border-t border-slate-50">
          <Clock class="h-3 w-3" />
          <span>Visto por último: {{ item.last_seen ? new Date(item.last_seen).toLocaleTimeString() : 'Nunca' }}</span>
        </div>
      </div>
    </div>

    <!-- Seção de Explicação -->
    <div class="bg-indigo-50 border border-indigo-100 rounded-xl p-5">
      <h4 class="text-indigo-800 font-semibold mb-2 flex items-center gap-2">
        <Signal class="h-4 w-4" /> Como funciona?
      </h4>
      <p class="text-indigo-700 text-sm leading-relaxed">
        O sistema mede a força do sinal WiFi (RSSI) emitido pelo Wemos da criança. 
        <strong>Very Near</strong> significa que a criança está na mesma sala que o scanner. 
        <strong>Near</strong> indica que ela está nas proximidades (corredor/pátio). 
        <strong>Far</strong> indica que o sinal está fraco e ela pode estar se afastando.
      </p>
    </div>
  </div>
</template>
