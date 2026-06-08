<script setup lang="ts">
import { ref } from 'vue';

import { BarChart3, Bug, Clock, Copy, MapPin, Radio, Signal, User, X } from 'lucide-vue-next';

import { AppNavbar } from '@/shared';

import ActivityChart from '../components/ActivityChart.vue';
import BenchmarkSection from '../components/BenchmarkSection.vue';
import { useDiscoveryQuery } from '../composables/useDiscoveryQuery';
import { useHeatmapQuery } from '../composables/useHeatmapQuery';
import { useLiveTrackingQuery } from '../composables/useLiveTrackingQuery';

const { data: liveTracking, isLoading: isLoadingLive } = useLiveTrackingQuery();
const { sortedDiscovery, isLoading: isLoadingDiscovery } = useDiscoveryQuery();

const showDebug = ref(false);
const selectedStudentMac = ref<string | null>(null);
const chartType = ref<'bar' | 'boxPlot' | 'heatmap'>('bar');

const { data: heatmapData, isLoading: isLoadingHeatmap } = useHeatmapQuery(selectedStudentMac);

const copyToClipboard = (text: string) => {
  void navigator.clipboard.writeText(text);
  alert('MAC copied!');
};

const getZoneColor = (zone: string) => {
  switch (zone) {
    case 'Very Near': return 'text-green-500 bg-green-50 border-green-200';
    case 'Near': return 'text-blue-500 bg-blue-50 border-blue-200';
    case 'Far': return 'text-yellow-500 bg-yellow-50 border-yellow-200';
    default: return 'text-gray-400 bg-gray-50 border-gray-200';
  }
};

const selectStudent = (mac: string) => {
  console.log('Selected MAC:', mac);
  selectedStudentMac.value = mac;
};
</script>

<template>
  <div class="min-h-screen pb-12 bg-slate-50/50">
    <AppNavbar />

    <main class="max-w-7xl mx-auto px-4 space-y-8">
      <header class="flex justify-between items-start">
        <div>
          <h1 class="text-2xl font-bold text-slate-800">
            Real-Time Tracking
          </h1>
          <p class="text-slate-500 text-sm">
            Approximate child location via WiFi proximity.
          </p>
        </div>
        <button
          class="p-2 text-slate-400 hover:text-indigo-600 transition-colors"
          @click="showDebug = !showDebug"
        >
          <Bug class="h-5 w-5" />
        </button>
      </header>

      <!-- Activity Section -->
      <section
        v-if="selectedStudentMac"
        class="animate-in fade-in slide-in-from-top-4 duration-300"
      >
        <div class="bg-white rounded-3xl border border-slate-200 shadow-lg overflow-hidden">
          <div class="p-6 border-b border-slate-100 flex justify-between items-center bg-slate-50/50">
            <div class="flex items-center gap-3">
              <div class="p-2 bg-indigo-100 rounded-xl text-indigo-600">
                <BarChart3 class="h-5 w-5" />
              </div>
              <div>
                <h2 class="font-bold text-slate-800">
                  Scanner Distribution: {{ liveTracking?.find(s => s.mac === selectedStudentMac)?.student_name }}
                </h2>
                <p class="text-xs text-slate-500">
                  Activity distribution by zone and scanner in the last 24h
                </p>
              </div>
            </div>

            <div class="flex items-center gap-2">
              <div class="flex bg-slate-200 p-1 rounded-xl">
                <button
                  :class="['px-3 py-1.5 text-[10px] font-black uppercase tracking-wider rounded-lg transition-all', chartType === 'bar' ? 'bg-white text-indigo-600 shadow-sm' : 'text-slate-500 hover:text-slate-700']" 
                  @click="chartType = 'bar'"
                >
                  Dist.
                </button>
                <button
                  :class="['px-3 py-1.5 text-[10px] font-black uppercase tracking-wider rounded-lg transition-all', chartType === 'boxPlot' ? 'bg-white text-indigo-600 shadow-sm' : 'text-slate-500 hover:text-slate-700']" 
                  @click="chartType = 'boxPlot'"
                >
                  Signal
                </button>
              </div>
              
              <button
                class="text-slate-400 hover:text-slate-600 p-2 hover:bg-slate-100 rounded-full transition-all"
                @click="selectedStudentMac = null"
              >
                <X class="h-5 w-5" />
              </button>
            </div>
          </div>
          <div class="p-6 space-y-8">
            <div
              v-if="isLoadingHeatmap"
              class="h-64 flex items-center justify-center"
            >
              <div class="animate-spin rounded-full h-8 w-8 border-b-2 border-indigo-600" />
            </div>
            <template v-else-if="heatmapData && heatmapData.length > 0">
              <div
                v-for="scanner in heatmapData"
                :key="scanner.scanner_id"
                class="space-y-2"
              >
                <div class="flex items-center gap-2 px-1">
                  <Signal class="h-4 w-4 text-slate-400" />
                  <h3 class="text-sm font-bold text-slate-600 uppercase tracking-tight">
                    {{ scanner.scanner_name }}
                  </h3>
                </div>
                <ActivityChart
                  :series="chartType === 'boxPlot' ? scanner.boxplot_series : scanner.series"
                  :type="chartType"
                />
              </div>
            </template>
            <p
              v-else
              class="text-center py-12 text-slate-400 italic"
            >
              No movement data for this student in the last 24h.
            </p>
          </div>
        </div>
      </section>

      <!-- Benchmark Section -->
      <BenchmarkSection />

      <!-- Section 1: Monitored Students -->
      <section class="space-y-4">
        <div class="flex items-center gap-2">
          <User class="h-5 w-5 text-emerald-600" />
          <h2 class="text-lg font-bold text-slate-800">
            Monitored Students
          </h2>
        </div>

        <div
          v-if="isLoadingLive"
          class="flex justify-center py-12"
        >
          <div class="animate-spin rounded-full h-8 w-8 border-b-2 border-emerald-600" />
        </div>

        <div
          v-else-if="!liveTracking || liveTracking.length === 0"
          class="bg-white p-12 rounded-2xl shadow-sm border border-slate-200 text-center"
        >
          <MapPin class="h-12 w-12 text-slate-300 mx-auto mb-3" />
          <p class="text-slate-500 font-medium">
            No students with linked trackers.
          </p>
          <p class="text-xs text-slate-400 mt-1">
            Link a MAC address in the student registry to start monitoring.
          </p>
        </div>

        <div
          v-else
          class="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-6"
        >
          <div
            v-for="item in liveTracking"
            :key="item.mac"
            :class="['bg-white p-5 rounded-2xl shadow-sm border flex flex-col gap-4 hover:shadow-md transition-all cursor-pointer', selectedStudentMac === item.mac ? 'border-indigo-500 ring-4 ring-indigo-50' : 'border-slate-200']"
            @click="selectStudent(item.mac)"
          >
            <div class="flex items-start justify-between">
              <div class="flex items-center gap-3">
                <div class="h-12 w-12 bg-emerald-100 rounded-full flex items-center justify-center text-emerald-600">
                  <User class="h-6 w-6" />
                </div>
                <div>
                  <h3 class="font-bold text-slate-800 leading-tight">
                    {{ item.student_name }}
                  </h3>
                  <p class="text-xs text-slate-400 font-mono tracking-tighter">
                    {{ item.mac }}
                  </p>
                </div>
              </div>
              <span :class="['px-2 py-0.5 rounded-full text-[10px] font-black uppercase tracking-wider border', item.status === 'online' ? 'bg-emerald-50 text-emerald-600 border-emerald-200' : 'bg-slate-50 text-slate-400 border-slate-200']">
                {{ item.status }}
              </span>
            </div>

            <div class="grid grid-cols-2 gap-3">
              <div :class="['p-3 rounded-xl border text-center transition-colors', getZoneColor(item.zone)]">
                <p class="text-[9px] uppercase font-black opacity-60 mb-0.5">
                  Zone
                </p>
                <p class="font-bold text-sm">
                  {{ item.zone }}
                </p>
              </div>
              <div class="p-3 bg-slate-50 rounded-xl border border-slate-100 text-center">
                <p class="text-[9px] uppercase font-black text-slate-400 mb-0.5">
                  Signal
                </p>
                <p class="font-bold text-sm text-slate-700">
                  {{ (item.rssi ?? '--').toString() }} <span class="text-[10px] font-normal opacity-50">dBm</span>
                </p>
              </div>
            </div>

            <div class="flex items-center justify-between mt-auto pt-3 border-t border-slate-50">
              <div class="flex items-center gap-2 text-[10px] text-slate-400">
                <Clock class="h-3 w-3" />
                <span>Last updated: {{ item.last_seen ? new Date(item.last_seen).toLocaleTimeString() : 'Never' }}</span>
              </div>
              <div class="text-indigo-500">
                <BarChart3 class="h-4 w-4" />
              </div>
            </div>
          </div>
        </div>
      </section>

      <!-- Section 2: Discovery -->
      <section class="space-y-4">
        <div class="flex items-center gap-2">
          <Radio class="h-5 w-5 text-indigo-600" />
          <h2 class="text-lg font-bold text-slate-800">
            Nearby Devices (New)
          </h2>
        </div>

        <div
          v-if="isLoadingDiscovery"
          class="grid grid-cols-1 md:grid-cols-2 gap-3"
        >
          <div
            v-for="i in 2"
            :key="i"
            class="h-16 bg-white animate-pulse rounded-xl border border-slate-100"
          />
        </div>

        <div
          v-else-if="sortedDiscovery.length === 0"
          class="p-8 bg-white/50 border-2 border-dashed border-slate-200 rounded-2xl text-center"
        >
          <Signal class="h-8 w-8 text-slate-300 mx-auto mb-2 animate-bounce" />
          <p class="text-sm text-slate-500 italic">
            Searching for nearby tags...
          </p>
          <p class="text-[10px] text-slate-400 mt-1">
            Turn on a Wemos to see it appear here.
          </p>
        </div>

        <div
          v-else
          class="grid grid-cols-1 md:grid-cols-2 gap-4"
        >
          <div
            v-for="tag in sortedDiscovery"
            :key="tag.mac"
            class="bg-white p-4 rounded-2xl shadow-sm border border-slate-200 flex items-center justify-between group hover:border-indigo-300 transition-all"
          >
            <div class="flex items-center gap-4">
              <div class="p-2 bg-indigo-50 rounded-lg group-hover:bg-indigo-100 transition-colors text-indigo-600">
                <Signal class="h-5 w-5" />
              </div>
              <div class="flex flex-col">
                <span class="text-sm font-mono font-black text-slate-700">{{ tag.mac }}</span>
                <div class="flex items-center gap-2">
                  <span class="text-[10px] font-bold text-indigo-500 uppercase leading-none">{{ tag.rssi.toString() }} dBm</span>
                  <span class="text-[10px] text-slate-300">•</span>
                  <span class="text-[10px] text-slate-400 font-semibold uppercase leading-none">{{ new Date(tag.timestamp).toLocaleTimeString() }}</span>
                </div>
              </div>
            </div>
            <button
              class="flex items-center gap-2 px-4 py-2 bg-indigo-50 text-indigo-600 rounded-xl text-xs font-bold hover:bg-indigo-600 hover:text-white transition-all"
              @click="copyToClipboard(tag.mac)"
            >
              <Copy class="h-3.5 w-3.5" />
              Copy
            </button>
          </div>
        </div>
      </section>

      <!-- Section 3: How it Works -->
      <div class="bg-indigo-600 rounded-3xl p-8 text-white relative overflow-hidden shadow-xl shadow-indigo-200">
        <Signal class="absolute -right-8 -bottom-8 h-48 w-48 opacity-10 rotate-12" />
        <h4 class="text-xl font-bold mb-3 flex items-center gap-2">
          <Signal class="h-5 w-5" /> Understanding Zones
        </h4>
        <div class="grid grid-cols-1 sm:grid-cols-3 gap-6 relative z-10">
          <div>
            <p class="font-black text-emerald-300 uppercase text-xs mb-1">
              Very Near
            </p>
            <p class="text-sm opacity-90 leading-relaxed text-indigo-50">
              Strong signal (>-55dBm). The child is very close to the scanner (same room).
            </p>
          </div>
          <div>
            <p class="font-black text-blue-300 uppercase text-xs mb-1">
              Near
            </p>
            <p class="text-sm opacity-90 leading-relaxed text-indigo-50">
              Medium signal (>-75dBm). The child is nearby or in an adjacent room.
            </p>
          </div>
          <div>
            <p class="font-black text-yellow-300 uppercase text-xs mb-1">
              Far
            </p>
            <p class="text-sm opacity-90 leading-relaxed text-indigo-50">
              Weak signal. The child is moving away or there are many obstacles in the path.
            </p>
          </div>
        </div>
      </div>
    </main>
  </div>
</template>
