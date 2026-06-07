import { useQuery } from '@tanstack/vue-query';
import client from '@/api/client';
import type { Summary } from '@/types';

export function useSummary() {
  return useQuery<Summary>({
    queryKey: ['summary'],
    queryFn: async () => {
      const response = await client.get('/metrics/summary');
      return response.data;
    },
    refetchInterval: 5000,
  });
}
