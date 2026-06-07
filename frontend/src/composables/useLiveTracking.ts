import { useQuery } from '@tanstack/vue-query';
import client from '@/api/client';
import type { TrackingStatus } from '@/types';

export function useLiveTracking() {
  return useQuery({
    queryKey: ['tracking', 'live'],
    queryFn: async () => {
      const response = await client.get<TrackingStatus[]>('/tracking/live');
      return response.data;
    },
    refetchInterval: 3000,
  });
}
