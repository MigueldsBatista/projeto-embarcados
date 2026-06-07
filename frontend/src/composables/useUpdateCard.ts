import { useMutation, useQueryClient } from '@tanstack/vue-query';
import client from '@/api/client';
import type { Card } from '@/types';

export function useUpdateCard() {
  const queryClient = useQueryClient();

  return useMutation({
    mutationFn: async ({ uid, data }: { uid: string; data: Partial<Card> }) => {
      const response = await client.put(`/cards/${uid}`, data);
      return response.data;
    },
    onSuccess: () => {
      queryClient.invalidateQueries({ queryKey: ['cards'] });
    },
  });
}
