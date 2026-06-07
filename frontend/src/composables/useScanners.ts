import { useQuery } from '@tanstack/vue-query';
import client from '@/api/client';

export function useScanners() {
  return useQuery({
    queryKey: ['scanners'],
    queryFn: async () => {
      const response = await client.get('/scanners/');
      return response.data;
    }
  });
}
