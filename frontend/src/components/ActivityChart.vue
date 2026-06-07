<script setup lang="ts">
import { computed } from 'vue';
import VueApexCharts from "vue3-apexcharts";

const props = defineProps<{
  series: any[];
  title?: string;
  type?: 'bar' | 'area' | 'heatmap' | 'boxPlot'; 
}>();

const chartOptions = computed(() => {
  const isHeatmap = props.type === 'heatmap';
  const isBoxPlot = props.type === 'boxPlot';
  const isBar = props.type === 'bar' || !props.type;
  
  return {
    chart: {
      height: 350,
      type: (props.type === 'boxPlot' ? 'boxPlot' : props.type) || 'bar',
      stacked: (isBar || props.type === 'area') && !isBoxPlot, 
      stackType: isBar ? '100%' : undefined,
      toolbar: {
        show: false
      },
      animations: {
        enabled: true,
        easing: 'easeinout' as const,
        speed: 800,
        animateGradually: {
          enabled: true,
          delay: 150
        },
        dynamicAnimation: {
          enabled: true,
          speed: 350
        }
      }
    },
    dataLabels: {
      enabled: false
    },
    // Zone colors for bar/area, distinct for boxplot
    colors: isBoxPlot ? ["#8b5cf6"] : ["#22c55e", "#3b82f6", "#eab308"],
    title: {
      text: props.title,
      align: 'left' as const,
      style: {
        fontSize: '14px',
        fontWeight: 'bold',
        fontFamily: 'Inter, sans-serif',
        color: '#1e293b'
      }
    },
    xaxis: {
      type: 'datetime' as const,
      labels: {
        datetimeUTC: false,
        style: {
          fontSize: '10px'
        }
      }
    },
    yaxis: {
      title: {
        text: isBoxPlot ? 'Distance (meters)' : (isBar ? 'Activity (%)' : undefined),
        style: {
          fontSize: '10px',
          fontWeight: 'bold',
          color: '#64748b'
        }
      },
      labels: {
        formatter: (val: number) => {
          if (isBoxPlot) return `${val.toFixed(1)}m`;
          if (isBar) return `${Math.round(val)}%`;
          return val.toString();
        },
        style: {
          fontSize: '10px'
        }
      }
    },
    stroke: {
      curve: 'smooth' as const,
      width: isHeatmap ? 0 : 2
    },
    plotOptions: {
      heatmap: isHeatmap ? {
        shadeIntensity: 0.5,
        radius: 2,
        useFillColorAsStroke: true,
        colorScale: {
          ranges: [
            { from: 1, to: 10, name: 'Low Activity', color: '#e2e8f0' },
            { from: 11, to: 30, name: 'Moderate Activity', color: '#94a3b8' },
            { from: 31, to: 70, name: 'High Activity', color: '#6366f1' },
            { from: 71, to: 1000, name: 'Intense Activity', color: '#4338ca' }
          ]
        }
      } : undefined,
      bar: {
        horizontal: false,
        borderRadius: isBoxPlot ? 0 : 4,
        columnWidth: '60%',
      },
      boxPlot: isBoxPlot ? {
        colors: {
          upper: '#8b5cf6',
          lower: '#c4b5fd'
        }
      } : undefined
    },
    tooltip: {
      shared: true,
      intersect: false,
      x: {
        format: 'dd MMM HH:mm'
      },
      y: {
        formatter: (val: number, opts: any) => {
          if (isBoxPlot) {
            // ApexCharts calls this formatter for each point in the boxplot summary
            return `${val.toFixed(2)}m`;
          }
          
          if (isBar) {
            const seriesIndex = opts.seriesIndex;
            const dataPointIndex = opts.dataPointIndex;
            try {
              const percent = opts.w.globals.seriesPercent[seriesIndex][dataPointIndex];
              return `${val} pings (${percent.toFixed(1)}%)`;
            } catch (e) {
              return `${val} pings`;
            }
          }
          return val.toString();
        }
      }
    },
    legend: {
      position: 'top' as const,
      horizontalAlign: 'right' as const,
      fontSize: '12px',
      fontFamily: 'Inter, sans-serif',
      show: true
    }
  };
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