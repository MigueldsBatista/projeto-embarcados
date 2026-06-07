import { useMutation, useQueryClient } from '@tanstack/vue-query';

import { CARD_QUERY_KEYS } from '../constants';
import { deleteCard } from '../services/cardsService';

export function useDeleteCardMutation() {
  const queryClient = useQueryClient();

  return useMutation({
    mutationFn: deleteCard,
    onSuccess: async () => {
      await queryClient.invalidateQueries({ queryKey: CARD_QUERY_KEYS.ALL });
    },
  });
}
