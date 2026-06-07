import { useQuery } from '@tanstack/vue-query';

import type { Summary } from '@/shared';

import { DASHBOARD_QUERY_KEYS } from '../constants';
import { fetchSummary } from '../services/dashboardService';

export function useSummaryQuery() {
  return useQuery<Summary>({
    queryKey: DASHBOARD_QUERY_KEYS.SUMMARY,
    queryFn: fetchSummary,
    refetchInterval: 5000,
  });
}
