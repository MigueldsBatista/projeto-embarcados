import { useQuery } from '@tanstack/vue-query';
import client from '@/api/client';
import { toValue } from 'vue';
import type { MaybeRefOrGetter } from 'vue';

export function useHeatmap(mac: MaybeRefOrGetter<string | null>) {
  return useQuery({
    queryKey: ['tracking', 'heatmap', mac],
    queryFn: async () => {
      const macValue = toValue(mac);
      if (!macValue) return [];
      console.log('Fetching heatmap for student:', macValue);
      const response = await client.get(`/tracking/heatmap/${macValue}`);
      return response.data;
    },
    enabled: () => !!toValue(mac),
  });
}
