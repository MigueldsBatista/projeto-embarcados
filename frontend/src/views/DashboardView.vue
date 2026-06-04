<script setup lang="ts">
import { useDashboard } from '../composables/useDashboard';
import Navbar from '../components/Navbar.vue';
import StatCard from '../components/StatCard.vue';
import LogsTable from '../components/LogsTable.vue';
import { Activity, ShieldAlert, CreditCard, Loader2 } from 'lucide-vue-next';

const { logs, summary, cards, isLoading } = useDashboard();
</script>

<template>
  <div class="min-h-screen pb-12">
    <Navbar />

    <main class="max-w-7xl mx-auto px-4">
      <div v-if="isLoading && !summary" class="flex flex-col items-center justify-center py-24 text-gray-400">
        <Loader2 class="w-12 h-12 animate-spin mb-4" />
        <p>Loading dashboard data...</p>
      </div>

      <template v-else>
        <!-- Stats Grid -->
        <div class="grid grid-cols-1 md:grid-cols-3 gap-6 mb-12">
          <StatCard 
            title="Total Scans" 
            :value="summary?.total_scans || 0" 
            :icon="Activity" 
            colorClass="bg-blue-50 text-blue-600"
          />
          <StatCard 
            title="Unknown Attempts" 
            :value="summary?.unknown_scans || 0" 
            :icon="ShieldAlert" 
            colorClass="bg-red-50 text-red-600"
          />
          <StatCard 
            title="Registered Cards" 
            :value="cards?.length || 0" 
            :icon="CreditCard" 
            colorClass="bg-purple-50 text-purple-600"
          />
        </div>

        <!-- Main Content Section -->
        <div class="grid grid-cols-1 gap-8">
          <section>
            <div class="flex items-center justify-between mb-4">
              <h2 class="text-xl font-bold">Recent Access Logs</h2>
              <button class="text-sm font-medium text-blue-600 hover:underline">View all</button>
            </div>
            <LogsTable :logs="logs || []" />
          </section>
        </div>
      </template>
    </main>
  </div>
</template>
