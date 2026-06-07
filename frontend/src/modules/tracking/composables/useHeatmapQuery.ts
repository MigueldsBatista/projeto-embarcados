import type { MaybeRefOrGetter } from 'vue';
import { toValue } from 'vue';

import { useQuery } from '@tanstack/vue-query';

import { TRACKING_QUERY_KEYS } from '../constants';
import { fetchHeatmap } from '../services/trackingService';

export function useHeatmapQuery(mac: MaybeRefOrGetter<string | null>) {
  return useQuery({
    queryKey: TRACKING_QUERY_KEYS.HEATMAP(mac),
    queryFn: async () => {
      const macValue = toValue(mac);
      if (!macValue) return [];
      return fetchHeatmap(macValue);
    },
    enabled: () => !!toValue(mac),
  });
}
