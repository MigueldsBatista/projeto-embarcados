import { useMutation, useQueryClient } from '@tanstack/vue-query';
import client from '@/api/client';
import type { Card } from '@/types';

export function useCreateCard() {
  const queryClient = useQueryClient();

  return useMutation({
    mutationFn: async (newCard: Partial<Card>) => {
      const response = await client.post('/cards/', newCard);
      return response.data;
    },
    onSuccess: () => {
      queryClient.invalidateQueries({ queryKey: ['cards'] });
    },
  });
}
