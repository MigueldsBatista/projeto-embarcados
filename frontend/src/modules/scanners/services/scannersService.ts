import client from '@/infrastructure/api/client';
import type { Scanner } from '@/shared';

export const fetchScanners = async (): Promise<Scanner[]> => {
  const response = await client.get('/scanners/');
  return response.data;
};

export const updateScanner = async (id: string, name: string): Promise<Scanner> => {
  const response = await client.put(`/scanners/${id}`, { name });
  return response.data;
};
