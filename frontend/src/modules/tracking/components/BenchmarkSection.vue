<script setup lang="ts">
import { computed } from 'vue';

import { Activity } from 'lucide-vue-next';

import { useBenchmarkQuery } from '../composables/useBenchmarkQuery';
import BenchmarkDataChart from './BenchmarkDataChart.vue';
import BenchmarkPerfChart from './BenchmarkPerfChart.vue';
import BenchmarkSummaryChart from './BenchmarkSummaryChart.vue';

const { data: benchmark } = useBenchmarkQuery();

const benchmarkSummaries = computed(() => benchmark.value?.summaries ?? []);
const benchmarkPerfPoints = computed(() => benchmark.value?.perf_points ?? []);
const recentBenchmarkBatches = computed(() => benchmark.value?.recent_batches ?? []);
</script>

<template>
  <section class="space-y-4">
    <div class="flex items-center gap-2">
      <Activity class="h-5 w-5 text-indigo-600" />
      <h2 class="text-lg font-bold text-slate-800">
        Buffer Performance Benchmark
      </h2>
    </div>

    <div class="bg-white p-5 rounded-3xl border border-slate-200 shadow-sm">
      <div
        v-if="benchmarkSummaries.length === 0"
        class="text-sm text-slate-500 text-center py-8"
      >
        <Activity class="h-8 w-8 mx-auto mb-2 opacity-20" />
        Waiting for benchmark telemetry on <span class="font-mono text-xs bg-slate-100 px-1 rounded">tracking/benchmark/#</span>. 
        <br>Enable <span class="font-mono text-xs bg-slate-100 px-1 rounded">TRACKING_BENCHMARK_ENABLED</span> in ESP32.
      </div>

      <div
        v-else
        class="space-y-8"
      >
        <!-- Numerical Summaries -->
        <div class="grid grid-cols-1 md:grid-cols-2 gap-4">
          <div
            v-for="summary in benchmarkSummaries"
            :key="summary.summary_key" 
            class="rounded-2xl border border-slate-100 p-5 bg-slate-50/50"
          >
            <div class="flex items-center justify-between mb-4">
              <span class="text-xs font-black uppercase tracking-widest text-slate-400">{{ summary.approach }}</span>
              <span class="text-[10px] font-black text-indigo-600 bg-white px-2 py-1 rounded-lg border border-indigo-100 shadow-sm">N = {{ summary.n_target.toLocaleString() }}</span>
            </div>
            <div class="grid grid-cols-2 gap-4">
              <div class="space-y-1">
                <p class="text-[10px] text-slate-400 font-black uppercase tracking-tighter">
                  Avg Latency
                </p>
                <p class="text-xl font-bold text-slate-800">
                  {{ summary.latency_avg_us }} <span class="text-xs font-normal text-slate-400">μs</span>
                </p>
              </div>
              <div class="space-y-1">
                <p class="text-[10px] text-slate-400 font-black uppercase tracking-tighter">
                  Max Latency
                </p>
                <p class="text-xl font-bold text-slate-800">
                  {{ summary.latency_max_us }} <span class="text-xs font-normal text-slate-400">μs</span>
                </p>
              </div>
              <div class="space-y-1">
                <p class="text-[10px] text-slate-400 font-black uppercase tracking-tighter">
                  Min Heap
                </p>
                <p class="text-sm font-bold text-slate-700">
                  {{ summary.heap_min.toLocaleString() }} <span class="text-[10px] font-normal text-slate-400">B</span>
                </p>
              </div>
              <div class="space-y-1">
                <p class="text-[10px] text-slate-400 font-black uppercase tracking-tighter">
                  Dropped
                </p>
                <p class="text-sm font-bold text-slate-700">
                  {{ summary.dropped }}
                </p>
              </div>
            </div>
          </div>
        </div>

        <!-- ApexCharts Components -->
        <div class="grid grid-cols-1 xl:grid-cols-2 gap-6">
          <BenchmarkSummaryChart :summaries="benchmarkSummaries" />
          <BenchmarkPerfChart :perf-points="benchmarkPerfPoints" />
        </div>

        <div class="pt-6 border-t border-slate-100">
          <BenchmarkDataChart :batches="recentBenchmarkBatches" />
        </div>
      </div>
    </div>
  </section>
</template>
