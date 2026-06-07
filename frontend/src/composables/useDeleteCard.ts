import { useMutation, useQueryClient } from '@tanstack/vue-query';
import client from '@/api/client';

export function useDeleteCard() {
  const queryClient = useQueryClient();

  return useMutation({
    mutationFn: async (uid: string) => {
      const response = await client.delete(`/cards/${uid}`);
      return response.data;
    },
    onSuccess: () => {
      queryClient.invalidateQueries({ queryKey: ['cards'] });
    },
  });
}
