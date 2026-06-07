import { useQuery } from '@tanstack/vue-query';
import client from '@/api/client';
import type { Card } from '@/types';

export function useCards() {
  return useQuery<Card[]>({
    queryKey: ['cards'],
    queryFn: async () => {
      const response = await client.get('/cards/');
      return response.data;
    },
  });
}
