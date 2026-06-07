<script setup lang="ts">
import { computed } from 'vue';
import { Bar } from 'vue-chartjs';
import {
  Chart as ChartJS,
  CategoryScale,
  LinearScale,
  BarElement,
  Title,
  Tooltip,
  Legend,
} from 'chart.js';
import type { TrackingBenchmarkBatch } from '@/types';

ChartJS.register(CategoryScale, LinearScale, BarElement, Title, Tooltip, Legend);

const props = defineProps<{
  batches: TrackingBenchmarkBatch[];
}>();

const lastBatch = computed(() => props.batches.at(-1));

const chartData = computed(() => {
  const batch = lastBatch.value;
  if (!batch) return { labels: [], datasets: [] };

  const isInefficient = batch.approach === 'inefficient';
  const color = isInefficient ? 'rgba(249,115,22,0.7)' : 'rgba(99,102,241,0.7)';
  const border = isInefficient ? '#f97316' : '#6366f1';
  const approachLabel = isInefficient ? 'Ineficiente' : 'Circular';

  return {
    labels: batch.samples.map((_, i) => `#${i}`),
    datasets: [
      {
        label: `RSSI (dBm) — ${approachLabel} · lote ${batch.batch} · N=${batch.n_target.toLocaleString('pt-BR')}`,
        data: batch.samples.map((s) => s.rssi),
        backgroundColor: color,
        borderColor: border,
        borderWidth: 1,
        borderRadius: 2,
      },
    ],
  };
});

const chartOptions = {
  responsive: true,
  maintainAspectRatio: false,
  plugins: {
    legend: { position: 'top' as const },
    title: {
      display: true,
      text: 'RSSI das Amostras — Último Lote Recebido via MQTT',
      font: { size: 13 },
    },
  },
  scales: {
    y: {
      title: { display: true, text: 'RSSI (dBm)' },
    },
    x: {
      title: { display: true, text: 'Amostra no lote' },
      ticks: { maxTicksLimit: 20 },
    },
  },
};
</script>

<template>
  <div v-if="lastBatch" class="h-72">
    <Bar :data="chartData" :options="chartOptions" />
  </div>
  <p v-else class="text-sm text-slate-400 py-6 text-center">
    Aguardando lotes de amostras via MQTT...
  </p>
</template>
