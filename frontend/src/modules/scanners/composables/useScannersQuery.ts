import { useQuery } from '@tanstack/vue-query';

import { SCANNER_QUERY_KEYS } from '../constants';
import { fetchScanners } from '../services/scannersService';

export function useScannersQuery() {
  return useQuery({
    queryKey: SCANNER_QUERY_KEYS.ALL,
    queryFn: fetchScanners,
  });
}
