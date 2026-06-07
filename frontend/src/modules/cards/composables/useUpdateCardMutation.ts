import { useMutation, useQueryClient } from '@tanstack/vue-query';

import type { Card } from '@/shared';

import { CARD_QUERY_KEYS } from '../constants';
import { updateCard } from '../services/cardsService';

export function useUpdateCardMutation() {
  const queryClient = useQueryClient();

  return useMutation({
    mutationFn: ({ uid, data }: { uid: string; data: Partial<Card> }) => updateCard(uid, data),
    onSuccess: async () => {
      await queryClient.invalidateQueries({ queryKey: CARD_QUERY_KEYS.ALL });
    },
  });
}
