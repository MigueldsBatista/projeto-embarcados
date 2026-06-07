<script setup lang="ts">
import { computed } from 'vue';
import VueApexCharts from "vue3-apexcharts";

import { getChartOptions } from '../utils/chartOptions';

const props = defineProps<{
  series: unknown[];
  title?: string;
  type?: 'bar' | 'area' | 'heatmap' | 'boxPlot'; 
}>();

const chartOptions = computed(() => {
  const isHeatmap = props.type === 'heatmap';
  const isBoxPlot = props.type === 'boxPlot';
  const isBar = props.type === 'bar' || !props.type;
  
  return getChartOptions(isHeatmap, isBoxPlot, isBar, props.title);
});
</script>

<template>
  <div class="bg-white p-4 rounded-2xl border border-slate-100 shadow-sm">
    <VueApexCharts 
      :key="props.type"
      :type="(props.type === 'boxPlot' ? 'boxPlot' : props.type) || 'bar'" 
      height="350" 
      :options="chartOptions" 
      :series="series" 
    />
  </div>
</template>