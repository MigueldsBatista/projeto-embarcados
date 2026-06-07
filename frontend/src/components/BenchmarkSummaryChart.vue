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
import type { TrackingBenchmarkSummary } from '@/types';

ChartJS.register(CategoryScale, LinearScale, BarElement, Title, Tooltip, Legend);

const props = defineProps<{
  summaries: TrackingBenchmarkSummary[];
}>();

const chartData = computed(() => {
  const nValues = [...new Set(props.summaries.map((s) => s.n_target))].sort((a, b) => a - b);
  const getAvg = (approach: string, n: number) =>
    props.summaries.find((s) => s.approach === approach && s.n_target === n)?.latency_avg_us ?? 0;

  return {
    labels: nValues.map((n) => `N = ${n.toLocaleString('pt-BR')}`),
    datasets: [
      {
        label: 'Ineficiente — latência média (μs)',
        data: nValues.map((n) => getAvg('inefficient', n)),
        backgroundColor: 'rgba(249,115,22,0.75)',
        borderColor: '#f97316',
        borderWidth: 1,
        borderRadius: 4,
      },
      {
        label: 'Circular — latência média (μs)',
        data: nValues.map((n) => getAvg('circular', n)),
        backgroundColor: 'rgba(99,102,241,0.75)',
        borderColor: '#6366f1',
        borderWidth: 1,
        borderRadius: 4,
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
      text: 'Comparativo de Latência por Volume (μs) — Ineficiente vs. Circular',
      font: { size: 13 },
    },
  },
  scales: {
    y: {
      beginAtZero: true,
      title: { display: true, text: 'Latência Média (μs)' },
    },
    x: {
      title: { display: true, text: 'Escala (N amostras)' },
    },
  },
};
</script>

<template>
  <div v-if="summaries.length > 0" class="h-72">
    <Bar :data="chartData" :options="chartOptions" />
  </div>
  <p v-else class="text-sm text-slate-400 py-6 text-center">
    Aguardando sumários do benchmark...
  </p>
</template>
