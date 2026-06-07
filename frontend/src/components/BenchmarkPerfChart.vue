<script setup lang="ts">
import { computed } from 'vue';
import { Line } from 'vue-chartjs';
import {
  Chart as ChartJS,
  CategoryScale,
  LinearScale,
  PointElement,
  LineElement,
  Title,
  Tooltip,
  Legend,
} from 'chart.js';
import type { TrackingBenchmarkPerfPoint } from '@/types';

ChartJS.register(CategoryScale, LinearScale, PointElement, LineElement, Title, Tooltip, Legend);

const props = defineProps<{
  perfPoints: TrackingBenchmarkPerfPoint[];
}>();

const chartData = computed(() => {
  const inefficient = props.perfPoints.filter((p) => p.approach === 'inefficient');
  const circular = props.perfPoints.filter((p) => p.approach === 'circular');
  const maxLen = Math.max(inefficient.length, circular.length, 1);
  const labels = Array.from({ length: maxLen }, (_, i) => String(i + 1));

  return {
    labels,
    datasets: [
      {
        label: 'Ineficiente (μs)',
        data: inefficient.map((p) => p.latency_us),
        borderColor: '#f97316',
        backgroundColor: 'rgba(249,115,22,0.08)',
        tension: 0.3,
        pointRadius: 2,
        fill: true,
      },
      {
        label: 'Circular (μs)',
        data: circular.map((p) => p.latency_us),
        borderColor: '#6366f1',
        backgroundColor: 'rgba(99,102,241,0.08)',
        tension: 0.3,
        pointRadius: 2,
        fill: true,
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
      text: 'Evolução da Latência por Ponto de Medição (μs)',
      font: { size: 13 },
    },
    tooltip: {
      callbacks: {
        afterLabel: (ctx: { dataIndex: number; dataset: { label?: string } }) => {
          const approach = ctx.dataset.label?.startsWith('Ineficiente') ? 'inefficient' : 'circular';
          // not shown directly, but could be extended
          return `Abordagem: ${approach}`;
        },
      },
    },
  },
  scales: {
    y: {
      beginAtZero: true,
      title: { display: true, text: 'Latência (μs)' },
    },
    x: {
      title: { display: true, text: 'Ponto de Medição (ordem de chegada)' },
      ticks: { maxTicksLimit: 15 },
    },
  },
};
</script>

<template>
  <div v-if="perfPoints.length > 0" class="h-72">
    <Line :data="chartData" :options="chartOptions" />
  </div>
  <p v-else class="text-sm text-slate-400 py-6 text-center">
    Aguardando pontos de performance do benchmark...
  </p>
</template>
