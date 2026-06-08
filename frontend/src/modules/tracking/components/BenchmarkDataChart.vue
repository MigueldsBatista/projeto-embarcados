<script setup lang="ts">
import { computed } from 'vue';
import VueApexCharts from "vue3-apexcharts";

import type { TrackingBenchmarkBatch } from '@/shared/types';

const props = defineProps<{
  batches: TrackingBenchmarkBatch[];
}>();

const lastBatch = computed(() => props.batches[props.batches.length - 1]);

const chartData = computed(() => {
  if (!lastBatch.value) return [];
  
  return [
    {
      name: 'RSSI (dBm)',
      data: lastBatch.value.samples.map((s) => s.rssi),
    },
  ];
});

const chartOptions = computed(() => {
  const batch = lastBatch.value;
  if (!batch) return undefined;
  
  const approachLabel = batch.approach === 'inefficient' ? 'Ineficiente' : 'Circular';
  const color = batch.approach === 'inefficient' ? '#f97316' : '#6366f1';

  return {
    chart: {
      type: 'bar' as const,
      toolbar: { show: false },
    },
    colors: [color],
    plotOptions: {
      bar: {
        borderRadius: 4,
        columnWidth: '70%',
      },
    },
    dataLabels: { enabled: false },
    xaxis: {
      categories: batch.samples.map((_, i) => `#${String(i)}`),
      title: { text: 'Amostra no lote' }
    },
    yaxis: {
      title: { text: 'RSSI (dBm)' },
    },
    title: {
      text: `RSSI das Amostras — ${approachLabel} (Lote ${String(batch.batch)})`,
      align: 'left' as const,
      style: { fontSize: '14px', fontWeight: 'bold' }
    },
    tooltip: {
      y: {
        formatter: (val: number) => `${String(val)} dBm`
      }
    }
  };
});
</script>

<template>
  <div class="bg-white p-4 rounded-2xl border border-slate-100 shadow-sm">
    <div
      v-if="lastBatch"
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
      Aguardando lotes de amostras RSSI...
    </p>
  </div>
</template>
