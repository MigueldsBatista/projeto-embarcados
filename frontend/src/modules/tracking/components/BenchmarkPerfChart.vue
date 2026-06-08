<script setup lang="ts">
import { computed } from 'vue';
import VueApexCharts from "vue3-apexcharts";

import type { TrackingBenchmarkPerfPoint } from '@/shared/types';

const props = defineProps<{
  perfPoints: TrackingBenchmarkPerfPoint[];
}>();

const chartData = computed(() => {
  const inefficient = props.perfPoints.filter((p) => p.approach === 'inefficient');
  const circular = props.perfPoints.filter((p) => p.approach === 'circular');

  return [
    {
      name: 'Ineficiente (μs)',
      data: inefficient.map((p) => p.latency_us),
    },
    {
      name: 'Circular (μs)',
      data: circular.map((p) => p.latency_us),
    },
  ];
});

const chartOptions = computed(() => {
  return {
    chart: {
      type: 'line' as const,
      toolbar: { show: false },
      animations: { enabled: true, easing: 'linear' as const, dynamicAnimation: { speed: 500 } },
    },
    colors: ['#f97316', '#6366f1'],
    dataLabels: { enabled: false },
    stroke: { curve: 'smooth' as const, width: 3 },
    xaxis: {
      title: { text: 'Ponto de Medição (ordem de chegada)' },
      labels: { show: false }
    },
    yaxis: {
      title: { text: 'Latência (μs)' },
      labels: {
        formatter: (val: number) => val.toFixed(0)
      }
    },
    tooltip: {
      shared: true,
      intersect: false,
      y: {
        formatter: (val: number) => `${val.toFixed(0)} μs`
      }
    },
    legend: { position: 'top' as const, horizontalAlign: 'right' as const },
    title: {
      text: 'Evolução da Latência por Amostra',
      align: 'left' as const,
      style: { fontSize: '14px', fontWeight: 'bold' }
    }
  };
});
</script>

<template>
  <div class="bg-white p-4 rounded-2xl border border-slate-100 shadow-sm">
    <div
      v-if="perfPoints.length > 0"
      class="h-80"
    >
      <VueApexCharts 
        type="line" 
        height="100%" 
        :options="chartOptions" 
        :series="chartData" 
      />
    </div>
    <p
      v-else
      class="text-sm text-slate-400 py-12 text-center italic"
    >
      Aguardando pontos de performance...
    </p>
  </div>
</template>
