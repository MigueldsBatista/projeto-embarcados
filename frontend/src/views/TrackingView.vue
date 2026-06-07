<script setup lang="ts">
import { computed } from 'vue';
import { useTracking } from '@/composables/useTracking';
import { Activity, MapPin, Signal, Clock, User } from 'lucide-vue-next';
import BenchmarkSummaryChart from '@/components/BenchmarkSummaryChart.vue';
import BenchmarkPerfChart from '@/components/BenchmarkPerfChart.vue';
import BenchmarkDataChart from '@/components/BenchmarkDataChart.vue';

const { liveTracking, isLoadingLive, benchmark } = useTracking();

const benchmarkSummaries = computed(() => benchmark.value?.summaries ?? []);
const benchmarkPerfPoints = computed(() => benchmark.value?.perf_points ?? []);
const recentBenchmarkBatches = computed(() => benchmark.value?.recent_batches ?? []);
const latestPerfByApproach = computed(() => {
  const latest = new Map<string, { latency_us: number; heap_free: number; n_target: number }>();
  for (const point of benchmarkPerfPoints.value) {
    latest.set(point.approach, {
      latency_us: point.latency_us,
      heap_free: point.heap_free,
      n_target: point.n_target,
    });
  }
  return latest;
});

const getZoneColor = (zone: string) => {
  switch (zone) {
    case 'Perto': return 'text-green-500 bg-green-50 border-green-200';
    case 'Medio': return 'text-blue-500 bg-blue-50 border-blue-200';
    case 'Longe': return 'text-yellow-500 bg-yellow-50 border-yellow-200';
    default: return 'text-gray-400 bg-gray-50 border-gray-200';
  }
};
</script>

<template>
  <div class="space-y-6">
    <header>
      <h1 class="text-2xl font-bold text-slate-800">Rastreamento em Tempo Real</h1>
      <p class="text-slate-500">Classificação de distância por RSSI com um roteador: Perto, Medio e Longe.</p>
    </header>

    <div class="bg-white p-5 rounded-xl shadow-sm border border-slate-200">
      <div class="flex items-center gap-2 mb-4 text-slate-800">
        <Activity class="h-4 w-4 text-indigo-600" />
        <h2 class="font-semibold">Benchmark do Rastreamento</h2>
      </div>

      <div v-if="benchmarkSummaries.length === 0" class="text-sm text-slate-500">
        Nenhuma telemetria de benchmark recebida ainda em <span class="font-mono">tracking/benchmark/#</span>.
      </div>

      <div v-else class="space-y-6">
        <!-- Sumários numéricos -->
        <div class="grid grid-cols-1 md:grid-cols-2 gap-3">
          <div v-for="summary in benchmarkSummaries" :key="summary.summary_key" class="rounded-lg border border-slate-200 p-4 bg-slate-50">
            <div class="flex items-center justify-between mb-2">
              <span class="text-xs font-bold uppercase tracking-wide text-slate-500">{{ summary.approach }}</span>
              <span class="text-xs text-slate-400">N={{ summary.n_target }}</span>
            </div>
            <div class="grid grid-cols-2 gap-2 text-sm">
              <div>
                <p class="text-slate-400">Latência média</p>
                <p class="font-semibold text-slate-800">{{ summary.latency_avg_us }} us</p>
              </div>
              <div>
                <p class="text-slate-400">Máxima</p>
                <p class="font-semibold text-slate-800">{{ summary.latency_max_us }} us</p>
              </div>
              <div>
                <p class="text-slate-400">Heap mínimo</p>
                <p class="font-semibold text-slate-800">{{ summary.heap_min }}</p>
              </div>
              <div>
                <p class="text-slate-400">Perdas</p>
                <p class="font-semibold text-slate-800">{{ summary.dropped }}</p>
              </div>
            </div>
          </div>
        </div>

        <!-- Gráfico de Performance: comparativo por escala -->
        <div class="pt-4 border-t border-slate-100">
          <h3 class="text-sm font-semibold text-slate-600 mb-3">Gráfico de Performance — Latência Média por Volume</h3>
          <BenchmarkSummaryChart :summaries="benchmarkSummaries" />
        </div>

        <!-- Gráfico de Performance: evolução temporal da latência -->
        <div class="pt-4 border-t border-slate-100">
          <h3 class="text-sm font-semibold text-slate-600 mb-3">Evolução da Latência por Ponto de Medição</h3>
          <BenchmarkPerfChart :perf-points="benchmarkPerfPoints" />
        </div>

        <!-- Gráfico de Dados: amostras RSSI do último lote -->
        <div class="pt-4 border-t border-slate-100">
          <h3 class="text-sm font-semibold text-slate-600 mb-3">Gráfico de Dados — Amostras RSSI do Último Lote MQTT</h3>
          <BenchmarkDataChart :batches="recentBenchmarkBatches" />
        </div>
      </div>
    </div>

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
        O sistema mede a força do sinal WiFi (RSSI) emitido pela tag da criança e classifica em três níveis.
        <strong>Perto</strong> indica sinal forte, <strong>Medio</strong> indica distância intermediária,
        e <strong>Longe</strong> indica sinal fraco.
      </p>
    </div>
  </div>
</template>
