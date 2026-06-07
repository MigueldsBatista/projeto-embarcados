import { useMutation, useQueryClient } from '@tanstack/vue-query';

import { SCANNER_QUERY_KEYS } from '../constants';
import { updateScanner } from '../services/scannersService';

export function useUpdateScannerMutation() {
  const queryClient = useQueryClient();

  return useMutation({
    mutationFn: ({ id, name }: { id: string, name: string }) => updateScanner(id, name),
    onSuccess: async () => {
      await queryClient.invalidateQueries({ queryKey: SCANNER_QUERY_KEYS.ALL });
    }
  });
}
