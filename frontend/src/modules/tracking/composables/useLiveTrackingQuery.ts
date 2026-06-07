import { useQuery } from '@tanstack/vue-query';

import type { TrackingStatus } from '@/shared';

import { TRACKING_QUERY_KEYS } from '../constants';
import { fetchLiveTracking } from '../services/trackingService';

export function useLiveTrackingQuery() {
  return useQuery({
    queryKey: TRACKING_QUERY_KEYS.LIVE,
    queryFn: fetchLiveTracking,
    refetchInterval: 3000,
  });
}
