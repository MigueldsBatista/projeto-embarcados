export function getChartOptions(isHeatmap: boolean, isBoxPlot: boolean, isBar: boolean, title?: string) {
  return {
    chart: {
      height: 350,
      type: (isBoxPlot ? 'boxPlot' : (isHeatmap ? 'heatmap' : (isBar ? 'bar' : 'area'))) as 'bar',
      stacked: (isBar || (!isBoxPlot && !isHeatmap)) && !isBoxPlot,
      stackType: (isBar ? '100%' : undefined) as '100%' | 'normal' | undefined,
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
    colors: isBoxPlot ? ["#8b5cf6"] : ["#22c55e", "#3b82f6", "#eab308"],
    title: {
      text: title,
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
          if (isBar) return `${Math.round(val).toString()}%`;
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
        formatter: (val: number, opts?: { 
          seriesIndex: number, 
          dataPointIndex: number, 
          w: { globals: { seriesPercent: number[][] } } 
        }) => {
          if (isBoxPlot) {
            return `${val.toFixed(2)}m`;
          }
          
          if (isBar && opts) {
            const seriesIndex = opts.seriesIndex;
            const dataPointIndex = opts.dataPointIndex;
            try {
              const seriesPercent = opts.w.globals.seriesPercent[seriesIndex];
              const percent = seriesPercent ? seriesPercent[dataPointIndex] : undefined;
              return percent !== undefined 
                ? `${val.toString()} pings (${percent.toFixed(1)}%)`
                : `${val.toString()} pings`;
            } catch (e) {
              return `${val.toString()} pings`;
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
}
