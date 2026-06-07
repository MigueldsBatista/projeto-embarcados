import { useMutation, useQueryClient } from '@tanstack/vue-query';
import client from '@/api/client';

export function useUpdateScanner() {
  const queryClient = useQueryClient();

  return useMutation({
    mutationFn: async ({ id, name }: { id: string, name: string }) => {
      const response = await client.put(`/scanners/${id}`, { name });
      return response.data;
    },
    onSuccess: () => {
      queryClient.invalidateQueries({ queryKey: ['scanners'] });
    }
  });
}
