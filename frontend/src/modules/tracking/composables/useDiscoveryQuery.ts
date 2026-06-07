import { computed } from 'vue';

import { useQuery } from '@tanstack/vue-query';

import { TRACKING_QUERY_KEYS } from '../constants';
import { fetchDiscovery } from '../services/trackingService';

export function useDiscoveryQuery() {
  const query = useQuery({
    queryKey: TRACKING_QUERY_KEYS.DISCOVERY,
    queryFn: fetchDiscovery,
    refetchInterval: 2000,
  });

  const sortedDiscovery = computed(() => {
    if (!query.data.value) return [];
    return Object.entries(query.data.value).map(([mac, data]) => ({
      mac,
      rssi: data.rssi,
      timestamp: data.timestamp
    })).sort((a, b) => b.rssi - a.rssi);
  });

  return { ...query, sortedDiscovery };
}
