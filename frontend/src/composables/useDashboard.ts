import { useQuery } from '@tanstack/vue-query';
import client from '../api/client';
import type { AccessLog, Summary, Card } from '../types';

export function useDashboard() {
  const { data: logs, isLoading: isLoadingLogs } = useQuery<AccessLog[]>({
    queryKey: ['logs'],
    queryFn: async () => {
      const response = await client.get('/metrics/logs');
      return response.data;
    },
    refetchInterval: 5000,
  });

  const { data: summary, isLoading: isLoadingSummary } = useQuery<Summary>({
    queryKey: ['summary'],
    queryFn: async () => {
      const response = await client.get('/metrics/summary');
      return response.data;
    },
    refetchInterval: 5000,
  });

  const { data: cards, isLoading: isLoadingCards } = useQuery<Card[]>({
    queryKey: ['cards'],
    queryFn: async () => {
      const response = await client.get('/cards/');
      return response.data;
    },
  });

  return {
    logs,
    summary,
    cards,
    isLoading: isLoadingLogs || isLoadingSummary || isLoadingCards,
  };
}
