import { useQuery } from '@tanstack/vue-query';
import client from '@/api/client';
import type { DiscoveredTag } from '@/types';

export function useDiscovery() {
  return useQuery({
    queryKey: ['tracking', 'discovery'],
    queryFn: async () => {
      const response = await client.get<Record<string, DiscoveredTag>>('/tracking/discovery');
      return response.data;
    },
    refetchInterval: 2000,
  });
}
