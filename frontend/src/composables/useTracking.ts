import { useQuery } from '@tanstack/vue-query';
import client from '@/api/client';
import type { TrackingStatus, DiscoveredTag } from '@/types';

export function useTracking() {
  const { data: liveTracking, isLoading: isLoadingLive } = useQuery({
    queryKey: ['tracking', 'live'],
    queryFn: async () => {
      const response = await client.get<TrackingStatus[]>('/tracking/live');
      return response.data;
    },
    refetchInterval: 3000, // Atualiza a cada 3 segundos
  });

  const { data: discovery, isLoading: isLoadingDiscovery } = useQuery({
    queryKey: ['tracking', 'discovery'],
    queryFn: async () => {
      const response = await client.get<Record<string, DiscoveredTag>>('/tracking/discovery');
      return response.data;
    },
    refetchInterval: 2000, // Atualiza descoberta a cada 2 segundos
  });

  return {
    liveTracking,
    isLoadingLive,
    discovery,
    isLoadingDiscovery
  };
}
