import { useQuery, useMutation, useQueryClient } from '@tanstack/vue-query';
import client from '../api/client';
import type { Card } from '../types';

export function useCards() {
  const queryClient = useQueryClient();

  const { data: cards, isLoading } = useQuery<Card[]>({
    queryKey: ['cards'],
    queryFn: async () => {
      const response = await client.get('/cards/');
      return response.data;
    },
  });

  const createMutation = useMutation({
    mutationFn: async (newCard: Partial<Card>) => {
      const response = await client.post('/cards/', newCard);
      return response.data;
    },
    onSuccess: () => {
      queryClient.invalidateQueries({ queryKey: ['cards'] });
    },
  });

  const updateMutation = useMutation({
    mutationFn: async ({ uid, data }: { uid: string; data: Partial<Card> }) => {
      const response = await client.put(`/cards/${uid}`, data);
      return response.data;
    },
    onSuccess: () => {
      queryClient.invalidateQueries({ queryKey: ['cards'] });
    },
  });

  const deleteMutation = useMutation({
    mutationFn: async (uid: string) => {
      const response = await client.delete(`/cards/${uid}`);
      return response.data;
    },
    onSuccess: () => {
      queryClient.invalidateQueries({ queryKey: ['cards'] });
    },
  });

  return {
    cards,
    isLoading,
    isCreating: createMutation.isPending,
    isUpdating: updateMutation.isPending,
    isDeleting: deleteMutation.isPending,
    createCard: createMutation.mutateAsync,
    updateCard: updateMutation.mutateAsync,
    deleteCard: deleteMutation.mutateAsync,
  };
}
