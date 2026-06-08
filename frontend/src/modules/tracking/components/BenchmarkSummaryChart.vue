<script setup lang="ts">
import { computed } from 'vue';
import VueApexCharts from "vue3-apexcharts";

import type { TrackingBenchmarkSummary } from '@/shared/types';

const props = defineProps<{
  summaries: TrackingBenchmarkSummary[];
}>();

const chartData = computed(() => {
  const nValues = [...new Set(props.summaries.map((s) => s.n_target))].sort((a, b) => a - b);
  const getAvg = (approach: string, n: number) =>
    props.summaries.find((s) => s.approach === approach && s.n_target === n)?.latency_avg_us ?? 0;

  return [
    {
      name: 'Ineficiente (μs)',
      data: nValues.map((n) => getAvg('inefficient', n)),
    },
    {
      name: 'Circular (μs)',
      data: nValues.map((n) => getAvg('circular', n)),
    },
  ];
});

const chartOptions = computed(() => {
  const nValues = [...new Set(props.summaries.map((s) => s.n_target))].sort((a, b) => a - b);
  
  return {
    chart: {
      type: 'bar' as const,
      toolbar: { show: false },
      animations: { enabled: true }
    },
    plotOptions: {
      bar: {
        horizontal: false,
        columnWidth: '55%',
        borderRadius: 4,
      },
    },
    dataLabels: { enabled: false },
    stroke: {
      show: true,
      width: 2,
      colors: ['transparent']
    },
    colors: ['#f97316', '#6366f1'],
    xaxis: {
      categories: nValues.map((n) => `N = ${n.toLocaleString('pt-BR')}`),
      title: { text: 'Escala (N amostras)' }
    },
    yaxis: {
      title: { text: 'Latência Média (μs)' },
      labels: {
        formatter: (val: number) => val.toFixed(0)
      }
    },
    fill: { opacity: 1 },
    tooltip: {
      y: {
        formatter: (val: number) => `${val.toFixed(2)} μs`
      }
    },
    legend: { position: 'top' as const, horizontalAlign: 'right' as const },
    title: {
      text: 'Comparativo de Latência por Volume (μs)',
      align: 'left' as const,
      style: { fontSize: '14px', fontWeight: 'bold' }
    }
  };
});
</script>

<template>
  <div class="bg-white p-4 rounded-2xl border border-slate-100 shadow-sm">
    <div
      v-if="summaries.length > 0"
      class="h-80"
    >
      <VueApexCharts 
        type="bar" 
        height="100%" 
        :options="chartOptions" 
        :series="chartData" 
      />
    </div>
    <p
      v-else
      class="text-sm text-slate-400 py-12 text-center italic"
    >
      Aguardando sumários do benchmark...
    </p>
  </div>
</template>
