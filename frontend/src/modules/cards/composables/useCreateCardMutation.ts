import { useMutation, useQueryClient } from '@tanstack/vue-query';

import type { Card } from '@/shared';

import { CARD_QUERY_KEYS } from '../constants';
import { createCard } from '../services/cardsService';

export function useCreateCardMutation() {
  const queryClient = useQueryClient();

  return useMutation({
    mutationFn: createCard,
    onSuccess: async () => {
      await queryClient.invalidateQueries({ queryKey: CARD_QUERY_KEYS.ALL });
    },
  });
}
