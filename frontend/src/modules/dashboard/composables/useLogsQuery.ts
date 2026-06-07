import { useQuery } from '@tanstack/vue-query';

import type { AccessLog } from '@/shared';

import { DASHBOARD_QUERY_KEYS } from '../constants';
import { fetchLogs } from '../services/dashboardService';

export function useLogsQuery() {
  return useQuery<AccessLog[]>({
    queryKey: DASHBOARD_QUERY_KEYS.LOGS,
    queryFn: fetchLogs,
    refetchInterval: 5000,
  });
}
