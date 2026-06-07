import { useQuery } from '@tanstack/vue-query';
import client from '@/api/client';
import type { AccessLog } from '@/types';

export function useLogs() {
  return useQuery<AccessLog[]>({
    queryKey: ['logs'],
    queryFn: async () => {
      const response = await client.get('/metrics/logs');
      return response.data;
    },
    refetchInterval: 5000,
  });
}
