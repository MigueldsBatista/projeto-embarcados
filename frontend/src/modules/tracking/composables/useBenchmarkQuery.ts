import { useQuery } from '@tanstack/vue-query';

import { fetchBenchmark } from '../services/trackingService';

export function useBenchmarkQuery() {
  return useQuery({
    queryKey: ['tracking', 'benchmark'],
    queryFn: fetchBenchmark,
    refetchInterval: 3000,
  });
}
