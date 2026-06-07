import { useQuery } from '@tanstack/vue-query';

import type { Card } from '@/shared';

import { CARD_QUERY_KEYS } from '../constants';
import { fetchCards } from '../services/cardsService';

export function useCardsQuery() {
  return useQuery<Card[]>({
    queryKey: CARD_QUERY_KEYS.ALL,
    queryFn: fetchCards,
  });
}
